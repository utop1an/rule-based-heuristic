#ifndef RULE_TREE_EX_H
#define RULE_TREE_EX_H

#include <sstream>
#include <iostream>
#include <fstream>
#include "../task_proxy.h"
#include "../utils/hash.h"
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include "../utils/timer.h"
// json
#include <../utils/rapidjson/document.h>
#include <../utils/rapidjson/stringbuffer.h>
#include <../utils/rapidjson/writer.h>


using namespace std;
using namespace rapidjson;

namespace rule_tree_ex{
typedef map<int, set<int>> Q;

typedef struct Node{
    int heuristic = 0;
    int var ;
    set<int> vals;
    unordered_map<uint64_t, Node*> children;
    Node* parent=nullptr;
    // how many times this rule has been used for calculating the heuristic
    int visited=0;
    bool is_init;

    Node(Node* parent) :parent(parent){}
    Node(Node* parent, int var, set<int> vals):parent(parent), var(var), vals(vals){}
}Node;

class RuleTree_ex {
protected:
    int size;
    int memory_usage;
    map<int, Node*> buckets;
    fstream fout;
    unordered_map<int, Node*> cache;

    void insert( Node* parent, int h, Q& q, bool is_init);
    Node* search(Node* node, vector<int>& state_values, StateID id, bool is_statistical) ;
    Q& translate(Node* node,Q& q) const;
    void dump_single(Node* node) const;
    void output_statitical_single(Node* node) ;
    void read_a_rule(string line);
    
public:
    explicit RuleTree_ex();
    virtual ~RuleTree_ex() = default;
    utils::Timer update_timer;
    utils::Timer calculate_timer;
    utils::Timer read_timer;

    void update(int h, Q& q, bool is_init);
    pair<int, Q> calculate(State &state,int bound, bool is_statistical);
    int inline get_count() const {return size;}; 
    void dump() const;
    void output_statiticals() ;
    void read_file();


};

}

#endif