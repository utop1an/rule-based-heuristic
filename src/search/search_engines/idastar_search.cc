#include "idastar_search.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "../task_utils/task_properties.h"
#include "../open_list_factory.h"
#include "../open_lists/best_first_open_list.h"
#include "../evaluators/g_evaluator.h"
#include "../task_utils/successor_generator.h"

#include "../rule_database/my_best_first_open_list.h"



#include "../utils/logging.h"

#include <iostream>

using namespace std;
using namespace my_best_first_open_list;
using namespace rule_database_ex;
namespace idastar_search {
IdastarSearch::IdastarSearch(const Options &opts)
    : SearchEngine(opts),
      evaluator(opts.get<shared_ptr<Evaluator>>("h")),
      bound(0),
      current_g(0),
      // count for cases that if h<l
      count(0),
      update(true),
      iterated_found_solution(false) {
    utils::g_log << "Launching IDA* Search..." << endl;
    
    utils::g_log << "Launch complete" << endl;    
}

void IdastarSearch::initialize(){
    assert(evaluator);

    // initialize the bound value = h of initial state
    State init = state_registry.get_initial_state();
    EvaluationContext eval_context(init, 0, false, &statistics);
    bound = eval_context.get_evaluator_value(evaluator.get());
    utils::g_log << "initialize: bound =" <<bound << endl;
    // initialize the rule-database & initialize the initial-goal rules
   
}

vector<OperatorID> IdastarSearch::get_successor_operators(State &state) const {
    vector<OperatorID> applicable_operators;
    successor_generator.generate_applicable_ops(
        state, applicable_operators);
    return applicable_operators;
}


pair<int, MyBestFirstOpenList> IdastarSearch::get_lookahead(State &state, int g){
    int lookahed = 0;
    vector<OperatorID> applicable_operators;
    successor_generator.generate_applicable_ops(
        state, applicable_operators);

    MyBestFirstOpenList openlist;
    for (OperatorID op_id: applicable_operators) {
        OperatorProxy op = task_proxy.get_operators()[op_id];
        State successor = state_registry.get_successor_state(state, op);
        EvaluationContext eval_context(successor, g+get_adjusted_cost(op), false, &statistics);
        
        // compute rulebased h
        int h1 = eval_context.get_evaluator_value(evaluator.get());
        int h2 = 0;
        int h = max(eval_context.get_evaluator_value(evaluator.get()),h2);
        int l = h + get_adjusted_cost(op);
        if (l < lookahed)
            lookahed = l;
        openlist.do_insertion(make_pair(eval_context, op_id), h1);   
    }
    return make_pair(lookahed, openlist);
}

map<int, vector<int>> IdastarSearch::updateRule(State &state, int lookahed){
    map<int, vector<int>> Q;
    // extract Q for state
    count += RuleDatabase.update(lookahed, Q);
}


SearchStatus IdastarSearch::step() {
    current_g = 0;
    State init = state_registry.get_initial_state();

    vector<pair<StateID,OperatorID>> path;
    path.push_back(make_pair(init.get_id(),OperatorID::no_operator));
    int sub_search_result = sub_search(path, 0);
     // exit
    if (iterated_found_solution) {
        utils::g_log << "count:" << count << endl;  
        return SOLVED;
    }
    if (sub_search_result == INT32_MAX){
        return FAILED;
    }
    bound = sub_search_result;
    utils::g_log << "new bound:" << bound << endl; 
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
    pair<StateID, OperatorID> p = path.at(path.size()-1);
    StateID current_state_id = p.first;
    State current_state = state_registry.lookup_state(current_state_id);
    OperatorID current_operator_id = p.second;
    SearchNode current_node = search_space.get_node(current_state);
    if (current_operator_id == OperatorID::no_operator){
        if (current_node.is_new()){
            statistics.inc_expanded();
            current_node.open_initial();
        }else {
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
    // any drawbacks? like, we are updating before checking the bounds
    // also, we will get_ops more times, which shoule be ordered based on h of next state, more calc of h
    // add h to cache, and clear cache before adding a rule? next state will not be affected by current rule
    // or we just build another database inside idastar, jumpover the inbuilt frame of evals, hs....
    EvaluationContext eval_context(current_state, g, false, &statistics);
    int h = eval_context.get_evaluator_value(evaluator.get());
    if (update)
        h = max(RuleDatabase.calculate(current_state.get_unpacked_values(), -1).first, h);
        
    statistics.inc_evaluated_states();
    int f = g + h;
    utils::g_log << "f=" << f << endl;  
    if ( f > bound){
        return f;
    }
    if (check_goal_and_set_plan(current_state)) {
        utils::g_log << "plan length:" << path.size() << endl;  
        dump(path);
        iterated_found_solution = true;
        
        return -1;
    }
        
    int threshold = INT32_MAX;
    // TODO: compute lookahead, update h

    // get successor
    
    // OperatorID prev_opeartor_id = current_operator_id;
    // StateID prev_predecessor_id = current_predecessor_id;
    // current_predecessor_id = current_state.get_id();
    MyBestFirstOpenList openlist;
    pair<int, MyBestFirstOpenList> info= get_lookahead(current_state, g);
    if (update) {
        // check lookahed
        int lookahead = info.first;
        if (lookahead>h){
            // do update
            updateRule(current_state, lookahead);
        }
    }
    openlist = info.second;

    // todo, ordere the successors
    // vector<OperatorID> ops = get_successor_operators(current_state);
    // for (OperatorID op_id: ops)
    while (!openlist.empty()) {
        int prev_g = current_g;
        // OperatorProxy op = task_proxy.get_operators()[op_id];
        // State successor = state_registry.get_successor_state(current_state, op);
        // EvaluationContext eval_context(successor, current_g, false, &statistics);
        pair<EvaluationContext, OperatorID> info = openlist.remove_min();
        EvaluationContext eval_context = info.first;
        State successor = eval_context.get_state();
        
        // todo modify this
        bool flag = true;
        for (pair<StateID, OperatorID> x : path){
            StateID s = x.first;
            if (successor.get_id() == s) {
                flag = false;
                break;
            }
        }
        if (!flag) {
            continue;
        }
        // current_operator_id = op_id;
        int new_g = eval_context.get_g_value();
        utils::g_log << "g=" << new_g << endl;
        OperatorProxy op = task_proxy.get_operators()[info.second];
        utils::g_log << "op=" << op.get_name()<< endl;
        // utils::g_log << "executing: " << op.get_name() << endl;
        // successor.unpack();
        // utils::g_log << "to state" << successor.get_unpacked_values() << endl;
        path.push_back(make_pair(successor.get_id(), info.second));
        int t = sub_search(path, new_g);
        if (iterated_found_solution)
            return -1;
        if (t<threshold)
            threshold = t;
        // current_state = state_registry.lookup_state(current_predecessor_id);
        // current_state.unpack();
        // utils::g_log << "go back to state" << current_state.get_unpacked_values() << endl;


        current_g = prev_g;
        // current_operator_id = prev_opeartor_id;
        // current_predecessor_id = prev_predecessor_id;
        // if (current_operator_id != OperatorID::no_operator) 
        //     current_node.close();
        path.pop_back();
    }
    utils::g_log << "-----------------" << endl;
    return threshold;

}

void IdastarSearch::print_statistics() const {
    utils::g_log << "Cumulative statistics:" << endl;
    statistics.print_detailed_statistics();
    search_space.print_statistics();
}

void add_options_to_parser(OptionParser &parser) {
    SearchEngine::add_pruning_option(parser);
    SearchEngine::add_options_to_parser(parser);
}

}
