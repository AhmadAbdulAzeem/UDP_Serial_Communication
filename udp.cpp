#include "udp.h"
#include <iostream>

UdpServer::UdpServer(std::string local_ip, int local_port, std::string remote_ip, int remote_port)
{
    this->local_ip = local_ip;
    this->local_port = local_port;
    this->remote_ip = remote_ip;
    this->remote_port = remote_port;
}

void UdpServer::setupUdpSocket()
{

    server_socket_udp = socket(AF_INET, SOCK_DGRAM, 0);

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(this->local_ip.c_str());
    serverAddress.sin_port = htons(this->local_port);

    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = inet_addr(this->remote_ip.c_str());
    clientAddress.sin_port = htons(this->remote_port);
    

    if (server_socket_udp < 0)
        std::cout << "Error setting up a udp socket" << std::endl;

    //binds the socket to any IP Address and the Port Number 13400
    bind(server_socket_udp, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    // //setting the IP Address for Multicast
    // setMulticastGroup("224.0.0.2");
}



void UdpServer::closeUdpSocket()
{
    close(server_socket_udp);
}




/*
 * Receives a udp message and calls reactToReceivedUdpMessage method
 * @return      amount of bytes which were send back to client
 *              or -1 if error occurred     
 */
int UdpServer::receiveUdpMessage()
{

    unsigned int length = sizeof(serverAddress);
    int readedBytes = recvfrom(server_socket_udp, data, _MaxDataSize, 0, (struct sockaddr *)&serverAddress, &length);
    
    if (readedBytes > 0 )
    {
        return readedBytes;
        this->dataLength = readedBytes;
    }
    return -1;
}



int UdpServer::sendUdpMessage(unsigned char *message, int messageLength)
{ //sendUdpMessage after receiving a message from the client
    //if the server receives a message from a client, than the response should be send back to the client address and port

    int result = sendto(server_socket_udp, message, messageLength, 0, (struct sockaddr *)&clientAddress, sizeof(clientAddress));
    return result;
}



/*
 * Getter to the last received data
 * @return  pointer to the received data array
 */
unsigned char *UdpServer::getData()
{
    return data;
}

/*
 * Getter to the length of the last received data
 * @return  length of received data
 */
int UdpServer::getDataLength() const
{
    return dataLength;
}

