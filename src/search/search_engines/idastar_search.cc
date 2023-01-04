#include "idastar_search.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "../task_utils/task_properties.h"
#include "../task_utils/successor_generator.h"

#include "../rule_database/my_best_first_open_list.h"



#include "../utils/logging.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace my_best_first_open_list;
using namespace rule_tree_ex;
namespace idastar_search {
IdastarSearch::IdastarSearch(const Options &opts)
    : SearchEngine(opts),
      evaluator(opts.get<shared_ptr<Evaluator>>("h")),
      step_bound(0),
      update(opts.get<bool>("u")),
      RuleDatabase(),
      debug(opts.get<bool>("d")),
      timing_of_update(opts.get<bool>("t")),
      do_shrink(opts.get<bool>("s")),
      do_init_rules(opts.get<bool>("i")),
      output_ruledatabase(opts.get<bool>("o")),
      use_exist_ruledatabase(opts.get<bool>("r")),
      shrink_count(0),
      iterated_found_solution(false) {
    shrink_timer.stop();
    rule_compute_timer.stop();
    utils::g_log << "Launching IDA* Search..." << endl;  
}

void dumpQ(map<int, set<int>> Q, int heuristic){
    cout<< "Q:[";
        for (map<int,set<int>>::iterator iter=Q.begin();iter!=Q.end();iter++){
            cout << "var " << iter->first << ": vals <";
            for(int val: iter->second){
                cout<< val << " ";
            }
            cout << ">";
        }
        cout << "], h=" << heuristic << endl;
}

void IdastarSearch::initialize(){
    assert(evaluator);
    if (use_exist_ruledatabase)
        RuleDatabase.read_file();
    GoalsProxy goals = task_proxy.get_goals();
    if (do_init_rules){
        for (FactProxy goal: goals){
            Q goalQ;
            goalQ[goal.get_variable().get_id()].insert(goal.get_value());
            RuleDatabase.update(1,goalQ, true);
            Q emptyQ;
            set<pair<int,int>> candidate;
            set<pair<int,int>> prevs;
            candidate.insert(make_pair(goal.get_variable().get_id(),goal.get_value()));
            init_rules(candidate, 1);
        }
    }
    // initialize the step_bound value = h of initial state
    State init = state_registry.get_initial_state();
    
    EvaluationContext eval_context(init, 0, false, &statistics);
    if (update){
        // step_bound = max(eval_context.get_evaluator_value(evaluator.get()), computeRuleDatabaseHeuristic(init, -1, true).first);
        step_bound = computeRuleDatabaseHeuristic(init, -1, true).first;
    }else {
        step_bound= eval_context.get_evaluator_value(evaluator.get());
    }

    cout << RuleDatabase.get_count() << endl;
    utils::g_log << "initialize: step_bound =" <<step_bound << endl;
    
}

SearchStatus IdastarSearch::step() {
    State init = state_registry.get_initial_state();
    vector<pair<StateID,OperatorID>> path;
    
    int sub_search_result = sub_search(init.get_id(), OperatorID::no_operator, StateID::no_state, 0);

    if (iterated_found_solution) {
        
        utils::g_log << "shrink count:" << shrink_count << endl;  
        utils::g_log << "total database size: " << RuleDatabase.get_count() << endl;  
        utils::g_log << "update time: " << RuleDatabase.update_timer << endl;
        utils::g_log << "calculate time: " << RuleDatabase.calculate_timer << endl;
        utils::g_log << "read database time: " << RuleDatabase.read_timer << endl;
        utils::g_log << "shrink time: " << shrink_timer << endl;
        utils::g_log << "tie-breaking time: " << rule_compute_timer << endl;
        fstream fout;
        fout.open("results.csv", ios::out|ios::app);
        // expanded nodes, number of rules, update time, calculate time
        fout << statistics.get_expanded()<< ","<< RuleDatabase.get_count() << "," << RuleDatabase.update_timer << "," << RuleDatabase.calculate_timer << "," << RuleDatabase.read_timer << ",";
        fout.close();
        if (output_ruledatabase)
            RuleDatabase.output_statiticals();
        return SOLVED;
    }

    if (sub_search_result == INT16_MAX ){
        return FAILED;
    }
    step_bound = sub_search_result;
    utils::g_log << "new step_bound:" << step_bound << endl; 
    utils::g_log << "current database size:" << RuleDatabase.get_count() << endl; 

    
    return IN_PROGRESS;
}


// no longer useful
void IdastarSearch::dump(vector<pair<StateID,OperatorID>> &path) {
    utils::g_log << "path:" << endl;
    for (pair<StateID, OperatorID> node : path) {
        StateID state_id = node.first;
        OperatorID operator_id = node.second;
        State state = state_registry.lookup_state(state_id);
        state.unpack();
        if (operator_id != OperatorID::no_operator){
            OperatorProxy op = task_proxy.get_operators()[operator_id];
            utils::g_log << op.get_name() << endl;
        }
        utils::g_log << state.get_unpacked_values() << endl;
        
    }
}

int IdastarSearch::sub_search(StateID cur_s, OperatorID cur_o, StateID prev_s, int g) {
    
    
    State current_state = state_registry.lookup_state(cur_s);
    SearchNode current_node = search_space.get_node(current_state);
    if (cur_o == OperatorID::no_operator){
        if (current_node.is_new()){
            current_node.open_initial();
            
            statistics.inc_expanded();   
        }else {
            current_node.reopen_initial();
            statistics.inc_reopened();
        }
    } else {
        
        State predecessor = state_registry.lookup_state(prev_s);
        SearchNode parent_node = search_space.get_node(predecessor);
        OperatorProxy current_operator = task_proxy.get_operators()[cur_o];
        if (current_node.is_new()){
            current_node.open(parent_node, current_operator, get_adjusted_cost(current_operator));
            statistics.inc_expanded();
            
        }else {
            current_node.reopen(parent_node, current_operator, get_adjusted_cost(current_operator));
            statistics.inc_reopened();
        }
    }
    
    if (debug){
        current_state.unpack();
        
        if (cur_o != OperatorID::no_operator){
            OperatorProxy op = task_proxy.get_operators()[cur_o];
        utils::g_log << "current state:" << current_state.get_unpacked_values() << ", current operator: " << op.get_name() << endl;
        }
    }
    
    EvaluationContext eval_context(current_state, g, false, &statistics);
    int h;
    if (update){
        h = computeRuleDatabaseHeuristic(current_state, -1, true).first;
    }else {
        h = eval_context.get_evaluator_value(evaluator.get());
    }
    statistics.inc_evaluated_states();
    int f = g + h;
    if ( f > step_bound ){
        current_node.close();
        return f;
    }
    if (check_goal_and_set_plan(current_state)) {
        iterated_found_solution = true;
        return -1;
    }
        
    int threshold = INT16_MAX;

    MyBestFirstOpenList openlist;
    vector<OperatorID> applicable_operators;
    successor_generator.generate_applicable_ops(current_state, applicable_operators);
    pair<int, MyBestFirstOpenList> info= get_lookahead(current_state,applicable_operators, g);
    if (update && timing_of_update) {
        // check lookahed
        int lookahead = info.first;
        if (lookahead>h){
            // do update
            updateRule(current_state, applicable_operators, lookahead);
        }
    }
    openlist = info.second;


    while (!openlist.empty()) {

        pair<EvaluationContext, OperatorID> info2 = openlist.remove_min();
        EvaluationContext eval_context = info2.first;
        State successor = eval_context.get_state();
        
        SearchNode node = search_space.get_node(successor);
        if (node.is_open()){
            continue;
        }
        int new_g = eval_context.get_g_value();
        OperatorProxy op = task_proxy.get_operators()[info2.second];

        // StateID tmp_state = prev_s;
        // OperatorID tmp_operator = cur_o;
        

        // pre_state=cur_state;
        // cur_state=successor.get_id();
        // cur_operator = info.second;

        int t = sub_search(successor.get_id(), info2.second, cur_s, new_g);
        if (iterated_found_solution){
            return -1;
        }  
        if (t<threshold)
            threshold = t;
        
        
    }
    
    if (update && !timing_of_update) {
        h = max(computeRuleDatabaseHeuristic(current_state, -1, true).first, h);
        pair<int, MyBestFirstOpenList> info2= get_lookahead(current_state,applicable_operators, g);
        // check lookahed
        int lookahead = info2.first;
        if (lookahead>h){
            // do update
            updateRule(current_state, applicable_operators, lookahead);
        }
    }
    current_node.close();
    return threshold;

}

// Generate initial rules
void IdastarSearch::init_rules(set<pair<int,int>> candidate, int current_h) {
    set<pair<int,int>> q;
    set<pair<int,int>> res;
    for (auto p:candidate){
        set<pair<int,int>> q1;
        for (OperatorProxy op : task_proxy.get_operators()){
            set<pair<int,int>> q2;
            bool is_prev_op = false;

            for (EffectProxy eff : op.get_effects()){
                if (eff.get_fact().get_variable().get_id() == p.first && eff.get_fact().get_value() == p.second){
                    is_prev_op = true;
                    break;
                }
            }
            if (is_prev_op){
                for (FactProxy pre : op.get_preconditions()){
                    
                    q2.insert(make_pair(pre.get_variable().get_id(), pre.get_value()));
                    // utils::g_log << op.get_name() << pre.get_variable().get_id() << "-" << pre.get_value() << endl; 
                }
            }
            
            if (q2.size()!=0){
                if (q1.size()==0){
                    q1 = q2;
                }else {
                    set<pair<int,int>> tmp;
                    set_intersection(q1.begin(),q1.end(), q2.begin(),q2.end(), inserter(tmp,tmp.end()));
                    q1 = tmp;
                }
            }
        }
        if (q1.size()!=0){
            if (q.size()==0){
                q = q1;
            }else {
                set_union(q.begin(),q.end(),q1.begin(),q1.end(), inserter(q,q.end()));
            }
        }
        
        set_union(q.begin(),q.end(),candidate.begin(),candidate.end(), inserter(res,res.end()));
    }
    if (res.size()!= candidate.size()){
        // turn set into Q
        Q emptyQ;
        for (auto p : res){
            emptyQ[p.first].insert(p.second);
        }
        RuleDatabase.update(current_h+1, emptyQ, true);
        
        init_rules(res, current_h+1);
    }
}
 
vector<OperatorID> IdastarSearch::get_successor_operators(State &state) const {
    vector<OperatorID> applicable_operators;
    
    successor_generator.generate_applicable_ops(
        state, applicable_operators);
    return applicable_operators;
}

// compute the lookahed of a state
pair<int, MyBestFirstOpenList> IdastarSearch::get_lookahead(State &state,vector<OperatorID> applicable_operators, int g){
    int lookahead = INT16_MAX;

    MyBestFirstOpenList openlist;
    for (OperatorID op_id: applicable_operators) {
        OperatorProxy op = task_proxy.get_operators()[op_id];
        State successor = state_registry.get_successor_state(state, op);
        EvaluationContext eval_context(successor, g+get_adjusted_cost(op), false, &statistics);
        successor.unpack();
        int h;
        
        if (update){
            pair<int, map<int, set<int>>> rule = computeRuleDatabaseHeuristic(successor,-1, true);
            h = rule.first;
        }else {
            h = eval_context.get_evaluator_value(evaluator.get());
        }
        int l = h + get_adjusted_cost(op);
        if (l < lookahead)
            lookahead = l;
        openlist.do_insertion(make_pair(eval_context, op_id), h);   
    }
    return make_pair(lookahead, openlist);
}



pair<int, map<int, set<int>>> IdastarSearch::computeRuleDatabaseHeuristic(State &state, int search_bound, bool is_statistical){
    // impose goal-aware here
    if(task_properties::is_goal_state(task_proxy,state)){
        Q emptyQ;
        return make_pair(0,emptyQ);
    }else {
        pair<int, map<int, set<int>>> rule = RuleDatabase.calculate(state, search_bound, is_statistical);
        return rule;
    }
}

Q IdastarSearch::shrink(Q q){
    Q output = Q(q);
    for (auto iter=q.begin();iter!=q.end(); iter++){
        VariableProxy var = task_proxy.get_variables()[iter->first];
        if (iter->second.size() == var.get_domain_size()-1) {
            // check fact
            int val;
            for (int candidate=0; candidate<var.get_domain_size(); candidate++){
                if (iter->second.find(candidate) == iter->second.end()){
                    val = candidate;
                    break;
                }
            }
            FactProxy fact = var.get_fact(val);
            for (auto iter2=q.begin();iter2!=q.end(); iter2++){
                VariableProxy var2 = task_proxy.get_variables()[iter2->first];
                if (iter2->second.size() != var2.get_domain_size()-1) {
                    
                    for (int val2:iter2->second){
                        FactProxy fact2 = var2.get_fact(val2); 
                        if (fact.is_mutex(fact2)){
                            ++shrink_count;
                            output[iter2->first].erase(val2);
                            if (output[iter2->first].size()==0){
                                output.erase(iter2->first);
                            }
                        }
                    }
                } 
            }
        }
    }

    return output;

}

void IdastarSearch::updateRule(State &state,vector<OperatorID> applicable_operators, int lookahead){
    map<int, set<int>> Q;
    state.unpack();
    vector<int> state_values = state.get_unpacked_values();
    for (OperatorProxy op:task_proxy.get_operators()){
        // applicable
        if(find(applicable_operators.begin(), applicable_operators.end(),op.get_id())!=applicable_operators.end()){
            State successor = state_registry.get_successor_state(state, op);
            pair<int, map<int, set<int>>> rule = computeRuleDatabaseHeuristic(successor,lookahead-get_adjusted_cost(op), false);
            vector<int> defined_vars;
            for (EffectProxy eff: op.get_effects()){
                int var = eff.get_fact().get_variable().get_id();
                if (eff.get_fact().get_value()!=-1) 
                    defined_vars.push_back(var);
            }
            map<int, set<int>> Q_eff = rule.second;
            for (map<int, set<int>>::iterator iter=Q_eff.begin(); iter!=Q_eff.end(); iter++){
                if (find(defined_vars.begin(),defined_vars.end(), iter->first)==defined_vars.end()){
                    Q[iter->first].insert(iter->second.begin(), iter->second.end());
                }
            }
        // inapplicable
        }else {
            for(FactProxy pre: op.get_preconditions()) {
                int var = pre.get_variable().get_id();
                int val = pre.get_value();
                if (val != state_values.at(var) && val!=-1){
                    Q[var].insert(val);
                    break;
                }
                
            }
            
        }
    }
    if (do_shrink){
        shrink_timer.resume();
        Q = shrink(Q);
        shrink_timer.stop();
    }
    RuleDatabase.update(lookahead, Q, false);
}


void IdastarSearch::print_statistics() const {
    utils::g_log << "Cumulative statistics:" << endl;
    statistics.print_detailed_statistics();
    search_space.print_statistics();
}

void add_options_to_parser(OptionParser &parser) {
    SearchEngine::add_options_to_parser(parser);
}

}
