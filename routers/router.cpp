#include "router.hpp"

vector<uint8_t> Router::pass_data(const vector<uint8_t>& request){
    int validation = service->validate_request(request);


    if(validation < 0){
        string s = "bad request";
        return vector<uint8_t>(s.begin(), s.end());
    }
    pair<string, vector<uint8_t>> parsed = service->parse_request(request);
    command = int(parsed.first[0] - '0');
    key = parsed.first.substr(1);
    value = parsed.second;
    
    vector<uint8_t> wal_data = service->transform_to_wal(command, key, value);
    bool res = memtable->set_wal(wal_data);
    if(!res){
        string s = "failed to write data to wal";
        return vector<uint8_t>(s.begin(), s.end());
    }
    if(command == 1){
        bool res1 = memtable->set_skiplist(key, value);
        if(!res1){
            string s = "failed to set data";
            return vector<uint8_t>(s.begin(), s.end());
        }else{
            string s = "data set successfully";
            return vector<uint8_t>(s.begin(), s.end());
        }
    }else if(command == 2){
        vector<uint8_t> res1 = memtable->get(key);
        cout << "data get successfully: " << endl;
        for(auto & it: res1){
            cout << int(it) << " ";
        }
        cout << endl;
        return res1;
    }else if(command == 3){
        bool res1 = memtable->remove(key);
        return {};
    }else{
        string s = "no query possible";
        return vector<uint8_t>(s.begin(), s.end());
    }
}