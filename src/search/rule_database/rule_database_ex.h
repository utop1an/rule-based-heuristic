#ifndef RULE_DATABASE_EX_H
#define RULE_DATABASE_EX_H

#include "../task_proxy.h"
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <algorithm>


using namespace std;
namespace rule_database_ex {
typedef map<int, set<int>> Q;

class Distribution;

class Layer{

public:
    void update();
    void calculate();
    void dump();
};

class Container: public Layer {
    map<int, Distribution> distributions;
    set<int> vals;
    int heuristic;
    int size;
public:
    explicit Container();
    explicit Container(set<int> vals);
    int update(int h, Q &q);
    pair<int,Q> calculate(vector<int> &state_values, Q &candidates);
    void dump(Q q);
};

class Distribution: public Layer {
    map<set<int>, Container> containers;
    int var;
    int size;
public:
    explicit Distribution();
    explicit Distribution(int var);
    int update(int h, Q &q);
    pair<int,Q> calculate(vector<int> &state_values, Q &candidates);
    void dump(Q q);
};

class Bucket: public Layer{
    map<int, Distribution> distributions;
    int size;
public:
    explicit Bucket();
    int update(int h, Q &q);
    pair<int,Q> calculate(vector<int> &state_values);
    void dump();
};

class RuleDatabaseEx: public Layer {
    map<int, Bucket> buckets;
    int size;
    int count;
    
public:
    explicit RuleDatabaseEx();
    int update(int h, Q &q);
    pair<int,Q> calculate(State &state,int bound);
    void dump();
    int get_count(){return count;}
};

}

#endif