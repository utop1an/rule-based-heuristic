#include "idastar_search.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "../task_utils/task_properties.h"
#include "../task_utils/successor_generator.h"

#include "../rule_database/my_best_first_open_list.h"



#include "../utils/logging.h"

#include <iostream>

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
      shrink_count(0),
      iterated_found_solution(false) {
    utils::g_log << "Launching IDA* Search..." << endl;  
}

void IdastarSearch::initialize(){
    assert(evaluator);
    GoalsProxy goals = task_proxy.get_goals();
    for (FactProxy goal: goals){
        map<int, set<int>> Q;
        Q[goal.get_variable().get_id()].insert(goal.get_value());
        RuleDatabase.update(1,Q);
    }
    // initialize the step_bound value = h of initial state
    State init = state_registry.get_initial_state();
    EvaluationContext eval_context(init, 0, false, &statistics);
    step_bound = max(eval_context.get_evaluator_value(evaluator.get()), computeRuleDatabaseHeuristic(init, -1).first);
    utils::g_log << "initialize: step_bound =" <<step_bound << endl;
    // initialize the rule-database & initialize the initial-goal rules
    
    
}

vector<OperatorID> IdastarSearch::get_successor_operators(State &state) const {
    vector<OperatorID> applicable_operators;
    successor_generator.generate_applicable_ops(
        state, applicable_operators);
    return applicable_operators;
}


pair<int, MyBestFirstOpenList> IdastarSearch::get_lookahead(State &state,vector<OperatorID> applicable_operators, int g){
    int lookahead = INT16_MAX;

    MyBestFirstOpenList openlist;
    for (OperatorID op_id: applicable_operators) {
        OperatorProxy op = task_proxy.get_operators()[op_id];
        State successor = state_registry.get_successor_state(state, op);
        EvaluationContext eval_context(successor, g+get_adjusted_cost(op), false, &statistics);
        int h1 = eval_context.get_evaluator_value(evaluator.get());
        successor.unpack();
        pair<int, map<int, set<int>>> rule = computeRuleDatabaseHeuristic(successor,-1);
        int h2 = rule.first;
        int h = max(h1,h2);
        int l = h + get_adjusted_cost(op);
        if (l < lookahead)
            lookahead = l;
        openlist.do_insertion(make_pair(eval_context, op_id), h);   
    }
    return make_pair(lookahead, openlist);
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

pair<int, map<int, set<int>>> IdastarSearch::computeRuleDatabaseHeuristic(State &state, int search_bound){
    
    if(task_properties::is_goal_state(task_proxy,state)){
        
        Q emptyQ;
        return make_pair(0,emptyQ);
    }else {
        pair<int, map<int, set<int>>> rule = RuleDatabase.calculate(state, search_bound);
        return rule;
    }
}

Q IdastarSearch::shrink(Q q){
    // for var var1 in q, if var can only take 1 val val1 (inclusive), check other var var2 (with multi vals)(exclusive),
    // val2,val is mutex to var1,val1, delete val2,val
    // note that, for var2 with only one val, we cannot delete it?
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
                    // emmmmm....
                    for (int val2:iter2->second){
                        FactProxy fact2 = var2.get_fact(val2); 
                        if (fact.is_mutex(fact2)){
                            ++shrink_count;
                            output[iter2->first].erase(val2);
                        }
                    }
                } else {
                    // check real mutex?
                    // useless.....
                    int val2;
                    for (int candidate=0; candidate<var2.get_domain_size(); candidate++){
                        if (iter2->second.find(candidate) == iter2->second.end()){
                            val2 = candidate;
                            break;
                        }
                    }
                    FactProxy fact2 = var2.get_fact(val2);
                    if (fact.is_mutex(fact2)){
                        cout<< "got this" << endl;
                        output.erase(iter2->first);
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
        if(find(applicable_operators.begin(), applicable_operators.end(),op.get_id())!=applicable_operators.end()){
            // applicable
            State successor = state_registry.get_successor_state(state, op);
            pair<int, map<int, set<int>>> rule = computeRuleDatabaseHeuristic(successor,lookahead-get_adjusted_cost(op));
            // pair<int, map<int, set<int>>> rule = computeRuleDatabaseHeuristic(successor,-1);
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
        }else {
            // inapplicable
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
    if (do_shrink)
        Q = shrink(Q);
    RuleDatabase.update(lookahead, Q);
}


SearchStatus IdastarSearch::step() {
    State init = state_registry.get_initial_state();

    vector<pair<StateID,OperatorID>> path;
    path.push_back(make_pair(init.get_id(),OperatorID::no_operator));
    int sub_search_result = sub_search(path, 0);
     // exit
    if (iterated_found_solution) {
        if (debug)
            RuleDatabase.dump();
        utils::g_log << "shrink count:" << shrink_count << endl;  
        utils::g_log << "total database size:" << RuleDatabase.get_count() << endl;  
        return SOLVED;
    }

    if (sub_search_result == INT16_MAX ){
        
        return FAILED;
    }
    step_bound = sub_search_result;
    utils::g_log << "new step_bound:" << step_bound << endl; 
    utils::g_log << "database size:" << RuleDatabase.get_count() << endl; 
    return IN_PROGRESS;
}



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

int IdastarSearch::sub_search(vector<pair<StateID,OperatorID>> &path, int g) {
    // dump(path);
    if (debug)
        dump(path);
    pair<StateID, OperatorID> p = path.at(path.size()-1);
    StateID current_state_id = p.first;
    State current_state = state_registry.lookup_state(current_state_id);
    OperatorID current_operator_id = p.second;
    SearchNode current_node = search_space.get_node(current_state);
    if (current_operator_id == OperatorID::no_operator){
        if (current_node.is_new()){
            current_node.open_initial();
            
            statistics.inc_expanded();   
        }else {
            current_node.reopen_initial();
            statistics.inc_reopened();
        }
    } else {
        pair<StateID, OperatorID> pre_p = path.at(path.size()-2);
        StateID predecessor_state_id = pre_p.first;
        State predecessor = state_registry.lookup_state(predecessor_state_id);
        SearchNode parent_node = search_space.get_node(predecessor);
        OperatorProxy current_operator = task_proxy.get_operators()[current_operator_id];
        if (current_node.is_new()){
            current_node.open(parent_node, current_operator, get_adjusted_cost(current_operator));
            statistics.inc_expanded();
            
        }else {
            current_node.reopen(parent_node, current_operator, get_adjusted_cost(current_operator));
            statistics.inc_reopened();
        }
    }
    
    // computing the heuristic, think about updating the heurisitic inside the computing_h func,
    // during such process, the lookahead is also checked, and updating the rules too.
    // thus we could seperate the solver and the eval, good for test and compare different evals
    // any drawbacks? like, we are updating before checking the step_bounds
    // also, we will get_ops more times, which shoule be ordered based on h of next state, more calc of h
    // add h to cache, and clear cache before adding a rule? next state will not be affected by current rule
    // or we just build another database inside idastar, jumpover the inbuilt frame of evals, hs....
    EvaluationContext eval_context(current_state, g, false, &statistics);
    int h = eval_context.get_evaluator_value(evaluator.get());
    if (update){
        h = max(computeRuleDatabaseHeuristic(current_state, -1).first, h);
    }
    statistics.inc_evaluated_states();
    int f = g + h;
    if ( f > step_bound){
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

        pair<EvaluationContext, OperatorID> info = openlist.remove_min();
        EvaluationContext eval_context = info.first;
        State successor = eval_context.get_state();
        
        SearchNode node = search_space.get_node(successor);
        if (node.is_open()){
            continue;
        }
        int new_g = eval_context.get_g_value();
        OperatorProxy op = task_proxy.get_operators()[info.second];

        path.push_back(make_pair(successor.get_id(), info.second));
        int t = sub_search(path, new_g);
        if (iterated_found_solution){
            return -1;
        }  
        if (t<threshold)
            threshold = t;
        path.pop_back();
    }
    
    if (update && !timing_of_update) {
        h = max(computeRuleDatabaseHeuristic(current_state, -1).first, h);
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

void IdastarSearch::print_statistics() const {
    utils::g_log << "Cumulative statistics:" << endl;
    statistics.print_detailed_statistics();
    search_space.print_statistics();
}

void add_options_to_parser(OptionParser &parser) {
    //SearchEngine::add_pruning_option(parser);
    SearchEngine::add_options_to_parser(parser);
}

}
