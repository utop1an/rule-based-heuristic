#ifndef SEARCH_ENGINES_IDASTAR_SEARCH_H
#define SEARCH_ENGINES_IDASTAR_SEARCH_H

#include "../open_list.h"
#include "../option_parser_util.h"
#include "../search_engine.h"

#include "../options/registries.h"
#include "../options/predefinitions.h"

namespace options {
class Options;
}

namespace idastar_search {
class IdastarSearch : public SearchEngine {
    // todo: correct the openlist
    std::unique_ptr<StateOpenList> open_list;
    int bound;
    int threshold;
    bool iterated_found_solution;
    

    std::shared_ptr<SearchEngine> get_search_engine(int engine_configs_index);
    std::shared_ptr<SearchEngine> create_current_phase();
    int sub_search();

protected:
    virtual void initialize() override;
    virtual SearchStatus step() override;

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
