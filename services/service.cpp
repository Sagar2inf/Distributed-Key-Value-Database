#include "service.hpp"

// query is valid only if i don't contain any control character and ':' and it should start with set/get/delete
int Service::validate_request(const vector<uint8_t> &request){
    string s = "";
    for(int i = 0; i < request.size(); i++){
        if(int(request[i]) < 32 || int(request[i]) > 126 || int(request[i]) == int(':')){
            return -1;
        }
        if(request[i] != ' ') s += char(request[i]);
    }
    if(s[0] == 's' || s[0] == 'S' || s[0] == 'g' || s[0] == 'G'){
        if(s.size() < 4) return -2;
        if(s[1] != 'e' && s[1] != 'E') return -1;
        if(s[2] != 't' && s[2] != 'T') return -1;
        if((s[0] == 's' || s[0] == 'S') && s.size() < 6) return -2;
        return 0;
    }else if(s[0] == 'd' || s[0] == 'D'){
        if(s.size() < 6) return -1;
        for(int i = 1; i < 6; i++){
            if(s[i] != "delete"[i] && s[i] != "DELETE"[i]) return -1;
        }
        if(s.size() < 7) return -2;
        return 0;
    }else {
        return -1;
    }
}
// commands 1 for set, 2 for get, 3 for delete
// it returns pair of command + key and value for get and delete are empty
pair<string, vector<uint8_t>> Service::parse_request(const vector<uint8_t>& request){
    string s = "";
    for(int i = 0; i < request.size(); i++){
        if(request[i] == ' ') continue;
        s += char(request[i]);
    }
    int command = 0;
    string key = "";
    vector<uint8_t> value;
    if(s[0] == 's' || s[0] == 'S') command = 1;
    else if(s[0] == 'g' || s[0] == 'G') command = 2;
    else if(s[0] == 'd' || s[0] == 'D') command = 3;
    if(command == 1 || command == 2){
        bool f = 0;
        for(int i = 3; i < s.size(); i++){
            if(s[i] == '='){
                f = 1; continue;
            }
            if(f) value.push_back(uint8_t(s[i]));
            else key += s[i];
        }
    }else{
        for(int i = 6; i < s.size(); i++){
            key += s[i];
        }
    }
    key = char('0' + command) + key;
    return {key, value};
}

/*
transformed data:
[total_len][command][key_len][key][value_len][value]
*/
vector<uint8_t> Service::transform_to_wal(int command, string key, vector<uint8_t>& value) {
    vector<uint8_t> res;

    auto push_u32 = [&](uint32_t val) {
        uint8_t buf[4];
        std::memcpy(buf, &val, 4);
        res.insert(res.end(), buf, buf + 4);
    };

    uint32_t k_size = key.size();
    uint32_t v_size = (command == 1) ? value.size() : 0;

    uint32_t total_val = 1 + 4 + k_size;
    if (command == 1) total_val += 4 + v_size;

    push_u32(htonl(total_val));

    res.push_back(static_cast<uint8_t>(command));

    push_u32(htonl(k_size));
    for (char c : key) res.push_back(static_cast<uint8_t>(c));

    if (command == 1) {
        push_u32(htonl(v_size));
        res.insert(res.end(), value.begin(), value.end());
    }

    return res;
}
