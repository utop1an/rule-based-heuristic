#ifndef RULE_TREE_EX_H
#define RULE_TREE_EX_H

#include "../task_proxy.h"
#include "../utils/hash.h"
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <algorithm>


using namespace std;
namespace rule_tree_ex{
typedef map<int, set<int>> Q;

typedef struct Node{
    int heuristic = 0;
    int var ;
    set<int> vals;
    unordered_map<uint64_t, Node*> children;
    Node* parent=nullptr;

    Node(Node* parent) :parent(parent){}
    Node(Node* parent, int var, set<int> vals):parent(parent), var(var), vals(vals){}
}Node;

class RuleTree {
protected:
    int size;
    map<int, Node*> buckets;

    void insert( Node* parent, int h, Q& q);
    Node* search(Node* node, vector<int>& state_values) const;
    Q& translate(Node* node,Q& q) const;
    void dump_single(Node* node) const;
    
public:
    explicit RuleTree();
    virtual ~RuleTree() = default;

    void update(int h, Q& q);
    pair<int, Q> calculate(State &state,int bound) const;
    int inline get_count() const {return size;}; 
    void dump() const;

};

}

#endif