#include "idastar_search.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "../open_list_factory.h"

#include "../utils/logging.h"

#include <iostream>

using namespace std;

// enum SubSearchStatus {SUB_IN_PROGRESS, SUB_FAILED, SUB_SOLVED};

namespace idastar_search {
IdastarSearch::IdastarSearch(const Options &opts)
    : SearchEngine(opts),
      
      open_list(opts.get<shared_ptr<OpenListFactory>>("open")->
                create_state_open_list()),
      bound(0),
      threshold(0),
      iterated_found_solution(false) {
          // initialize bound here? or better when call this in the plugin astar approach
          utils::g_log << "Launching IDA* Search..." << endl;
}

void IdastarSearch::initialize(){
    // initialize the bound value
    bound = 0;

    // initialize the rule-database & initialize the initial-goal rules

   
    
}


SearchStatus IdastarSearch::step() {

    int sub_search_result = sub_search();

     // exit
    if (iterated_found_solution) {
        return SOLVED;
    }
    if (sub_search_result = INT32_MAX){
        return FAILED;
    }
    bound = sub_search_result;
    return IN_PROGRESS;
    
}

int IdastarSearch::sub_search() {
    
    
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
