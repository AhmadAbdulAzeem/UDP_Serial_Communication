#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <iostream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <net/if.h>
#include <unistd.h>
#include <string>





const int _ServerPort = 13400;
const unsigned int _MaxDataSize = 4294967294;

class UdpServer {

public:
    UdpServer(std::string local_ip, int local_port, std::string remote_ip, int remote_port);
    void setupUdpSocket();
  
    int receiveUdpMessage();
 
    void closeUdpSocket();
    int sendUdpMessage(unsigned char* message, int messageLength);
    unsigned char* getData();
    int getDataLength() const;

private:
    std::string local_ip;
    std::string remote_ip;
    int local_port;
    int remote_port;
    unsigned char data[_MaxDataSize];
    int dataLength;
    int server_socket_udp;
    struct sockaddr_in serverAddress, clientAddress;
};

#endif /* UDPSERVER_H */