#ifndef MY_BEST_FIRST_OPEN_LIST_H
#define MY_BEST_FIRST_OPEN_LIST_H

#include <deque>
#include <map>

#include "../evaluation_context.h"

using namespace std;
namespace my_best_first_open_list {
class MyBestFirstOpenList {
    typedef deque<pair<EvaluationContext, OperatorID>> Bucket;

    map<int, Bucket> buckets;
    int size;
    

public:
    virtual void do_insertion(pair<EvaluationContext, OperatorID> info,
                              const int h);
    explicit MyBestFirstOpenList();
    virtual pair<EvaluationContext, OperatorID> remove_min();
    virtual bool empty() const ;
    virtual void clear() ;
};
}

#endif