#include "wal.hpp"


void WAL::write_data(const vector<uint8_t> data){
    size_t len = 0;
    while(len < data.size()){
        ssize_t n = write(fw, data.data() + len, data.size() - len);
        if(n < 0){
            cerr<<"Failed to write data to wal file" << endl;
            return;
        }
        len += n;
    }
    this->size += len;
}

vector<uint8_t> WAL::read_data(bool is_start, int data_size){
    if(is_start) lseek(fr, 0, SEEK_SET);
    vector<uint8_t> buffer(data_size);
    size_t len = 0;
    while(len < data_size){
        ssize_t n = read(fr, buffer.data() + len, data_size - len);
        if(n < 0){
            cerr <<"Failed to read data from WAL file" << endl;
            return {};
        }
        if(n == 0){
            buffer.resize(len);
            return buffer;
        }
        len += n;
    }
    return buffer;
}



// int main(){
//     string s1 = "Sagar Kumar test";
//     string s2 = "next append";
//     vector<uint8_t> buffer1(s1.begin(), s1.end());
//     vector<uint8_t> buffer2(s2.begin(), s2.end());
//     string dir = "wal";
//     string name = "walfile";
//     WAL wal(name, dir);
//     wal.write_data(buffer1);
//     wal.write_data(buffer2);

//     return 0;
// }