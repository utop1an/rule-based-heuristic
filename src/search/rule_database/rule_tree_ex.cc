#include "rule_tree_ex.h"

namespace rule_tree_ex {
    typedef map<int, set<int>> Q;

    RuleTree_ex::RuleTree_ex()
       :size(0){
        update_timer.stop();
        calculate_timer.stop();
        read_timer.stop();
       }

    void RuleTree_ex::update(int h, Q& q, bool is_init) {
        update_timer.resume();
        cache.clear();
        if (buckets.find(h) == buckets.end()){
            Node* root = new Node(nullptr);
            buckets[h] = root;
        }
        insert(buckets[h],h,q, is_init);
        update_timer.stop();
    }
    
    void RuleTree_ex::insert( Node* parent, int h, Q& q, bool is_init){
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
    
    pair<int, Q> RuleTree_ex::calculate(State &state,int bound, bool is_statistical) {
        calculate_timer.resume();
        state.unpack();
        vector<int> state_values = state.get_unpacked_values();
        Q emptyQ;
        if (size == 0){
            return {0, emptyQ};
        }
        if (bound==-1) {
            auto iter=cache.find(state.get_id().getValue());
            if (iter!=cache.end()){
                Node* cacheNode = iter->second;
                return {cacheNode->heuristic, translate(cacheNode,emptyQ)};
            }
            for(auto iter=buckets.rbegin();iter!=buckets.rend();iter++){
                Node* node = search(iter->second, state_values, state.get_id(), is_statistical);
                if (node!=nullptr){
                    cache[state.get_id().getValue()]=node;
                    return {node->heuristic, translate(node,emptyQ)};
                }
                    
            }
        }
        else {
            for(const auto& kv: buckets){
                if (kv.first>=bound){
                    Node* node = search(kv.second, state_values, state.get_id(), is_statistical);
                    if (node!=nullptr)
                        return {node->heuristic, translate(node,emptyQ)};
                }
            }
            for(auto iter=buckets.rbegin();iter!=buckets.rend();iter++){
                if (iter->first < bound){
                    Node* node = search(iter->second, state_values, state.get_id(), is_statistical);
                    if (node!=nullptr)
                        return {node->heuristic, translate(node,emptyQ)};
                }
            }
        }
        calculate_timer.stop();
        return {0, emptyQ};
    }

    Node* RuleTree_ex::search(Node* node, vector<int>& state_values, StateID id, bool is_statistical) {
        if(node->heuristic>0){
            if (is_statistical){
                node->visited++;
            }
            return node;
        }
        for(auto c:node->children){
            int var = c.second->var;
            if (c.second->vals.find(state_values.at(var))==c.second->vals.end()){
                Node* result = search(c.second, state_values, id, is_statistical);
                if (result!=nullptr){            
                    return result;
                }
                
            }
        }
        return nullptr;

    }

    Q& RuleTree_ex::translate(Node* node, Q& q) const{
        if (node->parent != nullptr){
            q[node->var] = node->vals;
            return translate(node->parent, q);
        }
        else   
            return q;

    }

    void RuleTree_ex::dump_single(Node* node) const {
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

    void RuleTree_ex::dump() const {
        for (auto it:buckets){
            cout<< "Buckets [" << it.first << "]:"<< endl;
            dump_single(it.second);
        }

    }

    // void RuleTree_ex::output_statitical_single(Node* node)  {
       
    //     if (node->heuristic > 0){
    //         Q q;
    //         q = translate(node, q);
    //         fout << "Q:[";
    //         for (map<int,set<int>>::iterator iter=q.begin();iter!=q.end();iter++){
    //             fout << "var" << iter->first << ":vals<";
    //             for(int val: iter->second){
    //                 fout<< val << " ";
    //             }
    //             fout << ">";
    //         }
    //         fout << "]," << node->heuristic << "," << node->visited << "," <<node->is_init<< "\n";
    //     }
    //     for (auto c:node->children){
    //         if (c.second != nullptr)
    //             output_statitical_single(c.second);
    //     }
    // }

    // void RuleTree_ex::output_statiticals()  {
    //     fout.open("ruledatabase.csv", ios::out);
    //     fout << "Rule,Heuristic,Count,IsInit" << "\n";
    //     for (auto it:buckets){
    //         output_statitical_single(it.second);
    //     }
    //     fout.close();
        
    // }

    void RuleTree_ex::output_statitical_single(Node* node)  {
       
        if (node->heuristic > 0){
            Q q;
            q = translate(node, q);
            Document d;
            Document::AllocatorType& allocator = d.GetAllocator();

            Value root(kObjectType);
            
            Value key(kStringType);
            Value value(kArrayType);

            
            for (map<int,set<int>>::const_iterator iter=q.begin();iter!=q.end();iter++){
                stringstream ss;
                ss << iter->first;
                string s = ss.str();
                key.SetString(s.c_str() , allocator);
                value.SetArray();
                for(int val: iter->second){
                    value.PushBack(val,allocator);
                }
                root.AddMember(key, value, allocator);
            }

            root.AddMember("h", node->heuristic, allocator);
            
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            root.Accept(writer);
            fout << buffer.GetString() << "\n";
        }
        for (auto c:node->children){
            if (c.second != nullptr)
                output_statitical_single(c.second);
        }
    }

    void RuleTree_ex::output_statiticals()  {
        fout.open("ruledatabase.json", ios::out);
        for (auto it:buckets){
            output_statitical_single(it.second);
        }
        fout.close();
        
    }



    void RuleTree_ex::read_file() {
        read_timer.resume();
        string line;
        ifstream fin;
        fin.open("ruledatabase.json", ios::in);
        assert(fin.is_open());
        while(getline(fin, line)){
            read_a_rule(line);
            
        }
        fin.close();
        read_timer.stop();
        cout << "successfully read: " << size << " rules!" << endl; 
    }

    

    void RuleTree_ex::read_a_rule(string line){
        Q q;
        int h;
        Document d;
        d.Parse<0>(line.c_str());
        
        for (Value::ConstMemberIterator it=d.MemberBegin(); it < d.MemberEnd(); ++it){
            if (strcmp(it->name.GetString(), "h")){
                
                set<int> vals;
                for (Value::ConstValueIterator itr = it->value.Begin(); itr != it->value.End(); ++itr){
                    vals.insert(itr->GetInt());
                }
                int var;
                sscanf(it->name.GetString(), "%d", &var);
                q[var] = vals;
            }else {
                h = it->value.GetInt();
                
            }
        }


        update(h, q, false);
    }
    
}