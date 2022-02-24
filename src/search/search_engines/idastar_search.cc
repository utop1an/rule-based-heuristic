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

// enum SubSearchStatus {SUB_IN_PROGRESS, SUB_FAILED, SUB_SOLVED};

namespace idastar_search {
IdastarSearch::IdastarSearch(const Options &opts)
    : SearchEngine(opts),
      evaluator(opts.get<shared_ptr<Evaluator>>("h")),
      bound(0),
      current_state(state_registry.get_initial_state()),
      current_eval_context(current_state, 0, false, &statistics),
      current_g(0),
      current_predecessor_id(StateID::no_state),
      current_operator_id(OperatorID::no_operator),
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
    
    current_state.unpack();
    utils::g_log << current_state.get_unpacked_values() << endl;
    
    bound = current_eval_context.get_evaluator_value(evaluator.get());
    utils::g_log << "initialize: bound =" <<bound << endl;
    // initialize the rule-database & initialize the initial-goal rules
   
}

vector<OperatorID> IdastarSearch::get_successor_operators(State &state) const {
    vector<OperatorID> applicable_operators;
    successor_generator.generate_applicable_ops(
        state, applicable_operators);

    return applicable_operators;
}

vector<State> IdastarSearch::get_succesors() {
    ordered_set::OrderedSet<State> successors;
    vector<OperatorID> applicable_operators = get_successor_operators(current_state);
    for(OperatorID op_id:applicable_operators) {
        OperatorProxy op = task_proxy.get_operators()[op_id];
        // assert(task_properties::is_applicable(op, current_state));
        
        State state = state_registry.get_successor_state(current_state, op);
        utils::g_log << state.get_id() << endl;
        state.unpack();
        successors.insert(state);
    }
    return successors.pop_as_vector();
}




SearchStatus IdastarSearch::step() {
    
    int sub_search_result = sub_search();

     // exit
    if (iterated_found_solution) {
        return SOLVED;
    }
    if (sub_search_result == INT32_MAX){
        return FAILED;
    }
    bound = sub_search_result;

    utils::g_log << "ending this step" << endl;
    return SOLVED;
    // return IN_PROGRESS;
    
}

int IdastarSearch::sub_search() {
    utils::g_log << "doing sub search" << endl;
    EvaluationContext eval_context(current_state, current_g, false, &statistics);
    int f = current_g + eval_context.get_evaluator_value(evaluator.get());
    if ( f > bound){
        return f;
    }
    if (check_goal_and_set_plan(current_state)) {
        iterated_found_solution = true;
        return -1;
    }
        
    int threshold = INT32_MAX;
    // get successor
    vector<OperatorID> ops = get_successor_operators(current_state);
    for (OperatorID op_id: ops) {
        OperatorProxy op = task_proxy.get_operators()[op_id];
        utils::g_log<< "op info: " << op.get_name() <<endl;
        
    }
    vector<State> succs = get_succesors();
    // for (StateID state_id: succs) {
        
    //     utils::g_log << "state..." << endl;
    // }

    return 2;

}

void IdastarSearch::print_statistics() const {
    utils::g_log << "Cumulative statistics:" << endl;
    statistics.print_detailed_statistics();
}

void IdastarSearch::save_plan_if_necessary() {
    // We don't need to save here, as we automatically save after
    // each successful search iteration.
}

void add_options_to_parser(OptionParser &parser) {
    SearchEngine::add_pruning_option(parser);
    SearchEngine::add_options_to_parser(parser);
}

}
