#include "rule_tree_ex.h"

namespace rule_tree_ex {
    typedef map<int, set<int>> Q;

    RuleTree::RuleTree()
       :size(0){
        
       }

    void RuleTree::update(int h, Q& q, bool is_init) {
        if (buckets.find(h) == buckets.end()){
            Node* root = new Node(nullptr);
            buckets[h] = root;
        }
        insert(buckets[h],h,q, is_init);

    }
    
    void RuleTree::insert( Node* parent, int h, Q& q, bool is_init){
        uint64_t hash = utils::get_hash(make_pair(q.begin()->first, q.begin()->second));
        if (parent->children.find(hash) == parent->children.end()){
            Node* node = new Node(parent, q.begin()->first, q.begin()->second);
 
            parent->children[hash] = node;
        }
        q.erase(q.begin());
        if (q.size()==0){
            if (parent->children[hash]->heuristic==0)
                size++;
            parent->children[hash]->heuristic=h;
            parent->children[hash]->is_init=is_init;
        }   
        else{
            insert(parent->children[hash], h, q, is_init);
        }
    }
    
    pair<int, Q> RuleTree::calculate(State &state,int bound, bool is_statistical) const{
        state.unpack();
        vector<int> state_values = state.get_unpacked_values();
        Q emptyQ;
        if (size == 0){
            return {0, emptyQ};
        }
        if (bound==-1) {
            for(auto iter=buckets.rbegin();iter!=buckets.rend();iter++){
                Node* node = search(iter->second, state_values,is_statistical);
                if (node!=nullptr)
                    return {node->heuristic, translate(node,emptyQ)};
            }
        }else {
            for(const auto& kv: buckets){
                if (kv.first>=bound){
                    Node* node = search(kv.second, state_values, is_statistical);
                    if (node!=nullptr)
                        return {node->heuristic, translate(node,emptyQ)};
                }
            }
            for(auto iter=buckets.rbegin();iter!=buckets.rend();iter++){
                if (iter->first < bound){
                    Node* node = search(iter->second, state_values, is_statistical);
                    if (node!=nullptr)
                        return {node->heuristic, translate(node,emptyQ)};
                }
            }
        }
        return {0, emptyQ};
    }

    Node* RuleTree::search(Node* node, vector<int>& state_values, bool is_statistical) const{
        if(node->heuristic>0){
            if (is_statistical){
                node->visited++;
            }
            return node;
        }
        for(auto c:node->children){
            int var = c.second->var;
            if (c.second->vals.find(state_values.at(var))==c.second->vals.end()){
                Node* result = search(c.second, state_values, is_statistical);
                if (result!=nullptr)
                    return result;
            }
        }
        return nullptr;

    }

    Q& RuleTree::translate(Node* node, Q& q) const{
        if (node->parent != nullptr){
            q[node->var] = node->vals;
            return translate(node->parent, q);
        }
        else   
            return q;

    }

    void RuleTree::dump_single(Node* node) const {
        if (node->heuristic > 0){
            Q q;
            q = translate(node, q);
            cout<< "Q:[";
            for (map<int,set<int>>::iterator iter=q.begin();iter!=q.end();iter++){
                cout << "var " << iter->first << ": vals <";
                for(int val: iter->second){
                    cout<< val << " ";
                }
                cout << ">";
            }
            cout << "], h=" << node->heuristic << endl;
        }
        for (auto c:node->children){
            if (c.second != nullptr)
                dump_single(c.second);
        }
    }

    void RuleTree::dump() const {
        for (auto it:buckets){
            cout<< "Buckets [" << it.first << "]:"<< endl;
            dump_single(it.second);
        }

    }

    void RuleTree::output_statitical_single(Node* node)  {
        
        if (node->heuristic > 0){
            Q q;
            q = translate(node, q);
            fout << "Q:[";
            for (map<int,set<int>>::iterator iter=q.begin();iter!=q.end();iter++){
                fout << "var" << iter->first << ":vals<";
                for(int val: iter->second){
                    fout<< val << " ";
                }
                fout << ">";
            }
            fout << "]," << node->heuristic << "," << node->visited << "," <<node->is_init<< "\n";
        }
        for (auto c:node->children){
            if (c.second != nullptr)
                output_statitical_single(c.second);
        }
    }

    void RuleTree::output_statiticals()  {
        fout.open("ruledatabase.csv", ios::out);
        fout << "Rule,Heuristic,Count,IsInit" << "\n";
        for (auto it:buckets){
            output_statitical_single(it.second);
        }
        fout.close();
    }
    
}