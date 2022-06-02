#include "idastar_search.h"
#include "search_common.h"

#include "../option_parser.h"
#include "../plugin.h"

using namespace std;

namespace plugin_idastar {
static shared_ptr<SearchEngine> _parse(OptionParser &parser) {
    parser.document_synopsis(
        "Iterative Deepening A* search",
        "TODO: correct this--A* is a special case of eager best first search that uses g+h "
        "as f-function. "
        "We break ties using the evaluator. Closed nodes are re-opened.");
    parser.document_note(
        "lazy_evaluator",
        "When a state s is taken out of the open list, the lazy evaluator h "
        "re-evaluates s. If h(s) changes (for example because h is path-dependent), "
        "s is not expanded, but instead reinserted into the open list. "
        "This option is currently only present for the A* algorithm.");
    parser.document_note(
        "Equivalent statements using general eager search",
        "\n```\n--search astar(evaluator)\n```\n"
        "is equivalent to\n"
        "```\n--evaluator h=evaluator\n"
        "--search eager(tiebreaking([sum([g(), h]), h], unsafe_pruning=false),\n"
        "               reopen_closed=true, f_eval=sum([g(), h]))\n"
        "```\n", true);
    parser.add_option<shared_ptr<Evaluator>>("h", "heurisitc");
    parser.add_option<bool>("u", "update");
    parser.add_option<bool>("d", "debug");
    parser.add_option<bool>("t", "update timing");
    SearchEngine::add_options_to_parser(parser);
    Options opts = parser.parse();

    shared_ptr<idastar_search::IdastarSearch> engine;
    if (!parser.dry_run()) {
        // add openlist?
        
        engine = make_shared<idastar_search::IdastarSearch>(opts);
    }

    return engine;
}

static Plugin<SearchEngine> _plugin("idastar", _parse);
}