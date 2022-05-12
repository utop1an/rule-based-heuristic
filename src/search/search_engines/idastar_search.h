#ifndef SEARCH_ENGINES_IDASTAR_SEARCH_H
#define SEARCH_ENGINES_IDASTAR_SEARCH_H

#include "../open_list.h"
#include "../option_parser_util.h"
#include "../search_engine.h"

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
#include "../algorithms/ordered_set.h"

#include "../rule_database/rule_database_ex.h"
#include "../rule_database/my_best_first_open_list.h"
#include <map>

namespace options {
class Options;
}

namespace rule_database_ex{
class RuleDatabaseEx;
}

namespace idastar_search {
class IdastarSearch : public SearchEngine {
protected:    
    // todo: correct the openlist
    std::shared_ptr<Evaluator> evaluator;

    //std::shared_ptr<Evaluator> h_evaluator;
    int step_bound;
    bool iterated_found_solution;
    int count;
    bool update;
    rule_database_ex::RuleDatabaseEx RuleDatabase;
    
    virtual void initialize() override;
    virtual SearchStatus step() override;
    std::vector<OperatorID> get_successor_operators(State &state) const;
    std::pair<int,my_best_first_open_list::MyBestFirstOpenList> get_lookahead(State &state, std::vector<OperatorID> applicable_operators,int g);
    int sub_search(std::vector<std::pair<StateID,OperatorID>> &path, int g);
    void updateRule(State &state,std::vector<OperatorID> applicable_operators, int lookahead);
    void dump(std::vector<std::pair<StateID,OperatorID>> &path);
    std::pair<int, std::map<int, std::set<int>>> computeRuleDatabaseHeuristic(State &state, int bound);
public:
    
    explicit IdastarSearch(const options::Options &opts);
    virtual ~IdastarSearch() = default;
    virtual void print_statistics() const override;
    void dump_search_space() const;
};
extern void add_options_to_parser(options::OptionParser &parser);
}

#endif
