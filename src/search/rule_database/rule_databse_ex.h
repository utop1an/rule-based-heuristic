#ifndef RULE_DATABASE_EX_H
#define RULE_DATABASE_EX_H

#include <deque>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;
namespace rule_database_ex {
typedef map<int, vector<int>> Q;

class Distribution;

class Layer{

public:
    void update();
    void calculate();
    void dump();
};

class Container: public Layer {
    map<int, Distribution> distributions;
    vector<int> vals;
    int heuristic;
    int size;
public:
    explicit Container();
    int update(int h, Q q);
    pair<int,Q> calculate(vector<int> state_values, Q candidates);
    void dump(Q q);
    void setVals(vector<int> vs){
        vals = vs;
    }
};

class Distribution: public Layer {
    map<vector<int>, Container> containers;
    int var;
    int size;
public:
    explicit Distribution();
    int update(int h, Q q);
    pair<int,Q> calculate(vector<int> state_values, Q candidates);
    void dump(Q q);
    void setVar(int v){
        var =v;
    }
};

class Bucket: public Layer{
    map<int, Distribution> distributions;
    int size;
public:
    explicit Bucket();
    int update(int h, Q q);
    pair<int,Q> calculate(vector<int> state_values);
    void dump();
};

class RuleDatabaseEx: public Layer {
    map<int, Bucket> buckets;
    int size;
    int count;
    
public:
    explicit RuleDatabaseEx();
    int update(int h, Q q);
    pair<int,Q> calculate(vector<int> state_values,int bound);
    void dump();
};

}

#endif