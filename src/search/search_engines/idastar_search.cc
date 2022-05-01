#include "idastar_search.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "../task_utils/task_properties.h"
#include "../open_list_factory.h"
#include "../open_lists/best_first_open_list.h"
#include "../evaluators/g_evaluator.h"
#include "../task_utils/successor_generator.h"


#include "../utils/logging.h"

#include <iostream>

using namespace std;

namespace idastar_search {
IdastarSearch::IdastarSearch(const Options &opts)
    : SearchEngine(opts),
      evaluator(opts.get<shared_ptr<Evaluator>>("h")),
      bound(0),
      current_g(0),
      iterated_found_solution(false) {
    utils::g_log << "Launching IDA* Search..." << endl;
    
    
    
    // set<Evaluator*> evals;
    // evaluator->get_path_dependent_evaluators(evals);
    // utils::g_log << "Launch evaluator complete" << endl;  
    Options options;
    options.set("eval", evaluator);
    options.set("pref_only", false);
    open_list = make_shared<standard_scalar_open_list::BestFirstOpenListFactory>(options)->create_edge_open_list();
    
    utils::g_log << "Launch complete" << endl;    
}

void IdastarSearch::initialize(){
    assert(evaluator);
    assert(open_list);
    // initialize the bound value = h of initial state

    State init = state_registry.get_initial_state();
    EvaluationContext eval_context(init, current_g, false, &statistics);
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



SearchStatus IdastarSearch::step() {
    current_g = 0;
    State init = state_registry.get_initial_state();

    vector<pair<StateID,OperatorID>> path;
    path.push_back(make_pair(init.get_id(),OperatorID::no_operator));
    int sub_search_result = sub_search(path);
     // exit
    if (iterated_found_solution) {
        return SOLVED;
    }
    if (sub_search_result == INT32_MAX){
        return FAILED;
    }
    bound = sub_search_result;
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

int IdastarSearch::sub_search(vector<pair<StateID,OperatorID>> &path) {
    // dump(path);
    pair<StateID, OperatorID> p = path.at(path.size()-1);
    StateID current_state_id = p.first;
    State _current_state = state_registry.lookup_state(current_state_id);
    OperatorID current_operator_id = p.second;

    SearchNode current_node = search_space.get_node(_current_state);
    
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
            
            // current_node.reopen(parent_node, current_operator, get_adjusted_cost(current_operator));
            statistics.inc_reopened();
        }
    }
    EvaluationContext eval_context(_current_state, current_g, false, &statistics);
    int h = eval_context.get_evaluator_value(evaluator.get());
    statistics.inc_evaluated_states();
    int f = current_g + h;
    if ( f > bound){
        return f;
    }
    if (check_goal_and_set_plan(_current_state)) {
        iterated_found_solution = true;
        return -1;
    }
        
    int threshold = INT32_MAX;
    // TODO: compute lookahead, update h

    // get successor
    int prev_g = current_g;
    // OperatorID prev_opeartor_id = current_operator_id;
    // StateID prev_predecessor_id = current_predecessor_id;
    // current_predecessor_id = current_state.get_id();
    vector<OperatorID> ops = get_successor_operators(_current_state);
    for (OperatorID op_id: ops) {
        OperatorProxy op = task_proxy.get_operators()[op_id];
        State successor = state_registry.get_successor_state(_current_state, op);
        
        // current_operator_id = op_id;
        current_g += op.get_cost();
        // utils::g_log << "executing: " << op.get_name() << endl;
        // successor.unpack();
        // utils::g_log << "to state" << successor.get_unpacked_values() << endl;
        path.push_back(make_pair(successor.get_id(), op_id));
        int t = sub_search(path);
        if (iterated_found_solution)
            return -1;
        if (t<threshold)
            threshold = t;

        // current_state = state_registry.lookup_state(current_predecessor_id);
        // _current_state.unpack();
        // utils::g_log << "go back to state" << _current_state.get_unpacked_values() << endl;
        current_g = prev_g;
        // current_operator_id = prev_opeartor_id;
        // current_predecessor_id = prev_predecessor_id;
        // node.close();
        path.pop_back();
    }
    return threshold;

}

void IdastarSearch::print_statistics() const {
    utils::g_log << "Cumulative statistics:" << endl;
    statistics.print_detailed_statistics();
    search_space.print_statistics();
}

// void IdastarSearch::save_plan_if_necessary() {
//     // We don't need to save here, as we automatically save after
//     // each successful search iteration.
// }

void add_options_to_parser(OptionParser &parser) {
    SearchEngine::add_pruning_option(parser);
    SearchEngine::add_options_to_parser(parser);
}

}
