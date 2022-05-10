#include "my_best_first_open_list.h"

using namespace std;

namespace my_best_first_open_list {

MyBestFirstOpenList::MyBestFirstOpenList()
    : size(0)
      {}

void MyBestFirstOpenList::do_insertion(
    pair<EvaluationContext,OperatorID> info, const int h) {
    
    buckets[h].push_back(info);
    ++size;
}

pair<EvaluationContext,OperatorID> MyBestFirstOpenList::remove_min() {
    assert(size > 0);
    auto it = buckets.begin();
    assert(it != buckets.end());
    Bucket &bucket = it->second;
    assert(!bucket.empty());
    pair<EvaluationContext,OperatorID> result = bucket.front();
    bucket.pop_front();
    if (bucket.empty())
        buckets.erase(it);
    --size;
    return result;
}

bool MyBestFirstOpenList::empty() const {
    return size == 0;
}

void MyBestFirstOpenList::clear() {
    buckets.clear();
    size = 0;
}

}
