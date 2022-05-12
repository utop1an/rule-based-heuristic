#ifndef HEURISTICS_ZERO_HEURISTIC_H
#define HEURISTICS_ZERO_HEURISTIC_H

#include "../heuristic.h"

namespace zero_heuristic {
class ZeroHeuristic : public Heuristic {
protected:
    virtual int compute_heuristic(const State &ancestor_state) override;
public:
    ZeroHeuristic(const options::Options &opts);
    ~ZeroHeuristic();
};
}

#endif
