#ifndef RULE_TREE_EX_H
#define RULE_TREE_EX_H

#include <iostream>
#include <fstream>
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
    // how many times this rule has been used for calculating the heuristic
    int visited=0;
    bool is_init;

    Node(Node* parent) :parent(parent){}
    Node(Node* parent, int var, set<int> vals):parent(parent), var(var), vals(vals){}
}Node;

class RuleTree {
protected:
    int size;
    map<int, Node*> buckets;
    fstream fout;

    void insert( Node* parent, int h, Q& q, bool is_init);
    Node* search(Node* node, vector<int>& state_values, bool is_statistical) const;
    Q& translate(Node* node,Q& q) const;
    void dump_single(Node* node) const;
    void output_statitical_single(Node* node) ;
    
public:
    explicit RuleTree();
    virtual ~RuleTree() = default;

    void update(int h, Q& q, bool is_init);
    pair<int, Q> calculate(State &state,int bound, bool is_statistical) const;
    int inline get_count() const {return size;}; 
    void dump() const;
    void output_statiticals() ;


};

}

#endif