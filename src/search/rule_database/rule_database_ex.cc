#include "rule_databse_ex.h"


using namespace std;

namespace rule_database_ex {
    typedef map<int, vector<int>> Q;
    
    RuleDatabaseEx::RuleDatabaseEx():
        size(0),
        count(0){}
    Bucket::Bucket():
        size(0){}
    Distribution::Distribution():
        size(0){}
    Container::Container():
        size(0),
        heuristic(0){}

    int RuleDatabaseEx::update(int h, Q q){
        
        Bucket bucket;
        buckets[h] = bucket;
        int i = bucket.update(h,q);
        count+=i;
        return i;
    }

    int Bucket::update(int h, Q q){
        Distribution distribution;
        int var = q.begin()->first;
        distributions[var] = distribution;
        distribution.setVar(var);
        return distribution.update(h,q);
    }

    int Distribution::update(int h, Q q){
        Container container;
        vector<int> vals = q.begin()->second; 
        containers[vals] = container;
        container.setVals(vals);
        return container.update(h,q);
    }

    int Container::update(int h, Q q){
        q.erase(q.begin());
        if (q.size()==0){
            if (heuristic > h)
                return 0;
            else {
                if(heuristic==0){
                    heuristic =h;
                    return 1;
                }else{
                   heuristic =h;
                    return 0; 
                }
            }
        }else{
            int var = q.begin()->first;
            Distribution distribution;
            distributions[var] = distribution;
            return distribution.update(h,q);
        }
    }

    pair<int,Q> RuleDatabaseEx::calculate(vector<int> state_values,int bound){
        // bound -1 means greaterQ value first
        if (count == 0){
            Q emptyQ;
            return make_pair(0, emptyQ);
        }
        if (bound ==-1) {
            for(map<int,Bucket>::reverse_iterator iter=buckets.rbegin();iter!=buckets.rend();iter++){
                return iter->second.calculate(state_values);
            }
        }else {
            for(map<int,Bucket>::iterator iter=buckets.begin();iter!=buckets.end() && iter->first>=bound;iter++){
                return iter->second.calculate(state_values);
            }
        }
    }

    pair<int,Q> Bucket::calculate(vector<int> state_values){
        for(map<int,Distribution>::iterator iter=distributions.begin();iter!=distributions.end();iter++){
            Q emptyQ;
            pair<int,Q> rule = iter->second.calculate(state_values, emptyQ);
            if (rule.first >0)
                return rule;
        }
        Q emptyQ;
        return make_pair(0, emptyQ);
    }

    pair<int,Q> Distribution::calculate(vector<int> state_values, Q candidates){
        for(map<vector<int>,Container>::iterator iter=containers.begin();iter!=containers.end();iter++){
            vector<int> vals=iter->first;
            if (find(vals.begin(), vals.end(), state_values.at(var))==vals.end()){
                candidates[var] = vals;
                pair<int,Q> rule = iter->second.calculate(state_values, candidates);
                if (rule.first>0)
                    return rule;
                //candidates.erase(var);
            }
        }
        
        return make_pair(0,candidates);
    }
    
    pair<int,Q> Container::calculate(vector<int> state_values, Q candidates){
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
            map<int, vector<int>> q;
            iter->second.dump(q);
        }
    }
    void Distribution::dump(Q q){
        map<vector<int>,Container>::iterator iter;
        for (iter=containers.begin();iter!=containers.end();iter++){
            q[var] = iter->first;
            iter->second.dump(q);
        }
    }
    void Container::dump(Q q){
        if (heuristic!=0){
            cout<< "Q:[";
            for (map<int,vector<int>>::iterator iter=q.begin();iter!=q.end();iter++){
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