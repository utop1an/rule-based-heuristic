#ifndef SEARCH_ENGINES_IDASTAR_SEARCH_H
#define SEARCH_ENGINES_IDASTAR_SEARCH_H

#include "../option_parser_util.h"

#include "../options/registries.h"
#include "../options/predefinitions.h"

#include "../evaluation_context.h"
#include "../evaluator.h"
#include "../open_list.h"
#include "../operator_id.h"
#include "../state_id.h"
#include "../search_engine.h"
#include "../search_progress.h"
#include "../search_space.h"


#include "../rule_database/rule_tree_ex.h"
#include "../rule_database/my_best_first_open_list.h"

namespace options {
class Options;
}

namespace idastar_search {
class IdastarSearch : public SearchEngine {
protected:    


    std::shared_ptr<Evaluator> evaluator;
    int step_bound;
    bool iterated_found_solution;
    rule_tree_ex::RuleTree_ex RuleDatabase;
    int shrink_count;

    // params
    bool update;
    int timing_of_update;
    bool debug;
    bool do_shrink;
    bool do_init_rules;
    bool output_ruledatabase;
    bool use_exist_ruledatabase;
    
    
    
    
    virtual void initialize() override;
    virtual SearchStatus step() override;
    int sub_search(StateID cur_s, OperatorID cur_o, StateID prev_s, int g);
    std::vector<OperatorID> get_successor_operators(State &state) const;
    std::pair<int,my_best_first_open_list::MyBestFirstOpenList> get_lookahead(State &state, std::vector<OperatorID> applicable_operators,int g);
    
    void updateRule(State &state,std::vector<OperatorID> applicable_operators, int lookahead);
    std::map<int, std::set<int>> shrink(std::map<int, std::set<int>> q);
    void init_rules(std::set<std::pair<int,int>> candidate,  int current_h);
    void dump(std::vector<std::pair<StateID,OperatorID>> &path);
    std::pair<int, std::map<int, std::set<int>>> computeRuleDatabaseHeuristic(State &state, int bound, bool is_statistical);
public:
    utils::Timer shrink_timer;
    utils::Timer rule_compute_timer;
    explicit IdastarSearch(const options::Options &opts);
    virtual ~IdastarSearch() = default;
    virtual void print_statistics() const override;
    void dump_search_space() const;
};
extern void add_options_to_parser(options::OptionParser &parser);
}

#endif
