#include "rule_database_ex.h"

using namespace std;

namespace rule_database_ex {
    typedef map<int, set<int>> Q;
    
    RuleDatabaseEx::RuleDatabaseEx():
        size(0),
        count(0){}
    Bucket::Bucket():
        size(0){}
    Distribution::Distribution(int var):
        var(var){}
    Distribution::Distribution(){}
    Container::Container():
        heuristic(0){}
    Container::Container(set<int> vals):
        vals(vals),
        heuristic(0){}
    

    int RuleDatabaseEx::update(int h, Q &q){
        if(buckets.count(h)==0){
            Bucket bucket;
            buckets[h] =bucket;
        }
        int i = buckets[h].update(h,q);
        count+=i;
        return i;
    }

    int Bucket::update(int h, Q &q){
        int var = q.begin()->first;
        if (distributions.count(var)==0){
            Distribution distribution(var);
            distributions[var] = distribution;
        }
        return distributions[var].update(h,q);
    }

    int Distribution::update(int h, Q &q){
        set<int> vals = q.begin()->second; 
        if(containers.count(vals)==0){
            Container container(vals);
            containers[vals]=container;
        }
        return containers[vals].update(h,q);
    }

    int Container::update(int h, Q& q){
        q.erase(q.begin());
        if (q.size()==0){
            if(heuristic==0){
                    heuristic =h;
                    return 1;
                }else{
                   return 0; 
                }
        }else{
            int var = q.begin()->first;
            if (distributions.count(var)==0){
                Distribution distribution(var);
                distributions[var] = distribution;
            }
            return distributions[var].update(h,q);
        }
    }

    pair<int,Q> RuleDatabaseEx::calculate(State &state,int bound){
        // bound -1 means greaterQ value first
        state.unpack();
        vector<int> state_values = state.get_unpacked_values();
        if (count == 0){
            Q emptyQ;
            return make_pair(0, emptyQ);
        }
        if (bound==-1) {
            for(map<int,Bucket>::reverse_iterator iter=buckets.rbegin();iter!=buckets.rend();iter++){
                
                pair<int,Q> rule = iter->second.calculate(state_values);
                if (rule.first >0)
                    return rule;
            }
        }else {
            for(map<int,Bucket>::iterator iter=buckets.begin();iter!=buckets.end();iter++){
                if (iter->first>=bound){
                    pair<int,Q> rule = iter->second.calculate(state_values);
                    if (rule.first >0)
                        return rule;
                }
            }
            for(map<int,Bucket>::reverse_iterator iter=buckets.rbegin();iter!=buckets.rend();iter++){
                if (iter->first < bound){
                    pair<int,Q> rule = iter->second.calculate(state_values);
                    if (rule.first >0)
                        return rule;
                }
            }
        }
        Q emptyQ;
        return make_pair(0, emptyQ);
    }

    pair<int,Q> Bucket::calculate(vector<int> &state_values){
        for(map<int,Distribution>::iterator iter=distributions.begin();iter!=distributions.end();iter++){
            Q emptyQ;
            pair<int,Q> rule = iter->second.calculate(state_values, emptyQ);
            if (rule.first >0)
                return rule;
        }
        Q emptyQ;
        return make_pair(0, emptyQ);
    }

    pair<int,Q> Distribution::calculate(vector<int> &state_values, Q &candidates){
        for(map<set<int>,Container>::iterator iter=containers.begin();iter!=containers.end();iter++){
            set<int> vals=iter->first;
            if (find(vals.begin(), vals.end(), state_values.at(var))==vals.end()){
                candidates[var] = vals;
                pair<int,Q> rule = iter->second.calculate(state_values, candidates);
                if (rule.first>0)
                    return rule;
                candidates.erase(var);
            }
        }
        
        return make_pair(0,candidates);
    }
    
    pair<int,Q> Container::calculate(vector<int> &state_values, Q &candidates){
        if (heuristic!=0)
            return make_pair(heuristic,candidates);
        for(map<int,Distribution>::iterator iter=distributions.begin();iter!=distributions.end();iter++){
            pair<int,Q> rule = iter->second.calculate(state_values, candidates);
            if (rule.first >0)
                return rule;
        }
        return make_pair(0, candidates);
    }

    void RuleDatabaseEx::dump(){
        map<int,Bucket>::iterator iter;
        for (iter=buckets.begin();iter!=buckets.end();iter++){
            cout << "Bucket[" << iter->first << "]" <<endl;
            iter->second.dump();
        }
    }
    void Bucket::dump(){
        map<int,Distribution>::iterator iter;
        for (iter=distributions.begin();iter!=distributions.end();iter++){
            Q q;
            iter->second.dump(q);
        }
    }
    void Distribution::dump(Q q){
        map<set<int>,Container>::iterator iter;
        for (iter=containers.begin();iter!=containers.end();iter++){
            q[var] = iter->first;
            iter->second.dump(q);
        }
    }
    void Container::dump(Q q){
        if (heuristic!=0){
            cout<< "Q:[";
            for (map<int,set<int>>::iterator iter=q.begin();iter!=q.end();iter++){
                cout << "var " << iter->first << ": vals <";
                for(int val: iter->second){
                    cout<< val << " ";
                }
                cout << ">";
            }
            cout << "], h=" << heuristic << endl;
        }
        for (map<int,Distribution>::iterator iter=distributions.begin();iter!=distributions.end();iter++){
            iter->second.dump(q);
        }
    }
}