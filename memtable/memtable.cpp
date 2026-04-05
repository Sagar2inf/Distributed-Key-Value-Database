#include "memtable.hpp"

bool Memtable::set_skiplist(string key, vector<uint8_t>& value){
    try{
        skiplist->insert(key, value);
        return true;
    } catch(...){
        return false;
    }
}
bool Memtable::set_wal(const vector<uint8_t>& data){
    try{
        wal->write_data(data);
        return true;
    } catch(...){
        return false;
    }
}
bool Memtable::remove(string key){
    try{
        skiplist->remove(key);
        return true;
    } catch(...){
        return false;
    }
}
vector<uint8_t> Memtable::get(string key){
    try{
        vector<uint8_t> res = skiplist->get(key);
        return res;
    } catch(...){
        string s = "Failed to get data";
        return vector<uint8_t>(s.begin(), s.end());
    }
}

void Memtable::recover_memtable(){
    cout << "Recovering data" << endl;
    bool is_start = true;
    auto buff_to_val = [&](const vector<uint8_t>& buff){
        if(buff.size() < 4) return uint32_t(0);
        uint32_t net_len;
        memcpy(&net_len, buff.data(), sizeof(uint32_t));
        uint32_t len = ntohl(net_len);
        return len;
    };
    bool is_set = true;
    while(true){
        vector<uint8_t> total_len_buff = wal->read_data(is_start, sizeof(uint32_t)); 
        if(total_len_buff.size() < sizeof(uint32_t)) break;

        uint32_t total_len = buff_to_val(total_len_buff);
        is_start = false;

        vector<uint8_t> cmd_buff = wal->read_data(is_start, 1);
        if(cmd_buff.empty()) break;
        uint8_t command = cmd_buff[0];

        vector<uint8_t> key_len_buff = wal->read_data(is_start, sizeof(uint32_t));
        uint32_t key_len = buff_to_val(key_len_buff);

        vector<uint8_t> key_buff = wal->read_data(is_start, key_len);
        string key(key_buff.begin(), key_buff.end());
        cout << "Recovering key: " << key << endl;
        cout << "Command: " << int(command) << endl;
        if(command == 1){
            vector<uint8_t> value_len_buff = wal->read_data(is_start, sizeof(uint32_t));
            uint32_t value_len = buff_to_val(value_len_buff);

            vector<uint8_t> value_buff = wal->read_data(is_start, value_len);
            cout << "Recovering value: ";
            for(auto & it: value_buff){
                cout << int(it) << " ";
            }
            cout << endl;
            if(total_len == (1 + 4 + key_len + 4 + value_len)){
                set_skiplist(key, value_buff);
            }else{
                cerr << "WAL is currepted" << endl;
                break;
            }
        }else if(command == 3){
            if(total_len == (1 + 4 + key_len)){
                remove(key);
            }else{
                cerr << "WAL is currepted" << endl;
                break;
            }
        }
    }
}

