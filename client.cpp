#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<cstring>
#include<arpa/inet.h>
#include<string>
#include<vector>
#include "./transport/transport.hpp"

Transport transport;

int main(){
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9000);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return 1;
    }

    std::cout << "Connected to server\n";

    while(true){
        std::string message;
        std::getline(std::cin, message);

        if(message == "exit") break;

        message += "\n";  

        std::vector<uint8_t> buffer(message.begin(), message.end());

        transport.write_data(sockfd, buffer);

        std::vector<uint8_t> res = transport.read_data(sockfd);

        std::string response(res.begin(), res.end());
        std::cout << response << std::endl;
    }

    close(sockfd);
}