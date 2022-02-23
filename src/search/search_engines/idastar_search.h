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

namespace options {
class Options;
}

namespace idastar_search {
class IdastarSearch : public SearchEngine {
protected:    
    // todo: correct the openlist
    std::unique_ptr<EdgeOpenList> open_list;
    std::shared_ptr<Evaluator> evaluator;
    //std::shared_ptr<Evaluator> h_evaluator;
    int bound;
    bool iterated_found_solution;
    State current_state;
    StateID current_predecessor_id;
    OperatorID current_operator_id;
    int current_g;
    int current_real_g;
    EvaluationContext current_eval_context;
    

    // std::shared_ptr<SearchEngine> get_search_engine(int engine_configs_index);
    


    virtual void initialize() override;
    virtual SearchStatus step() override;
    std::vector<OperatorID> get_successor_operators(State &state) const;
    std::vector<State> get_succesors();
    int sub_search();

public:
    explicit IdastarSearch(const options::Options &opts);
    virtual ~IdastarSearch() = default;

    virtual void save_plan_if_necessary() override;

    virtual void print_statistics() const override;
    void dump_search_space() const;
};
extern void add_options_to_parser(options::OptionParser &parser);
}

#endif
