#include "zero_heuristic.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../task_utils/task_properties.h"
#include "../utils/logging.h"

#include <cstddef>
#include <limits>
#include <utility>

using namespace std;

namespace zero_heuristic {
ZeroHeuristic::ZeroHeuristic(const Options &opts)
    : Heuristic(opts) {
    utils::g_log << "Initializing zero search heuristic..." << endl;
}

ZeroHeuristic::~ZeroHeuristic() {
}

int ZeroHeuristic::compute_heuristic(const State &ancestor_state) {
    return 0;
}

static shared_ptr<Heuristic> _parse(OptionParser &parser) {
    parser.document_synopsis("zero heuristic",
                             "Returns 0 for all state");

    Heuristic::add_options_to_parser(parser);
    Options opts = parser.parse();
    if (parser.dry_run())
        return nullptr;
    else
        return make_shared<ZeroHeuristic>(opts);
}

static Plugin<Evaluator> _plugin("zero", _parse);
}
