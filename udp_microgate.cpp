#include "udp.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>
#include <bits/stdc++.h>
#include "synclink.h"
#include <iostream>

// Default max frame size for driver.
// This can be increased with driver load option.
#define MAX_FRAME_SIZE 4096
#define FRAME_SIZE 100

int fd;

using namespace std;


void sigint_handler(int sigid)
{
	printf("Ctrl-C pressed\n");
}

// queue of pair(msg, length)
queue<pair<unsigned char *, int> > serial_to_network_queue;
queue<pair<unsigned char *, int> > network_to_serial_queue;

void configure_port(int fd);

/* TODO: process should take config to network */
void *network_thread(void *ptr);
void *send_network(void *ptr);

/* TODO: process should take config to serial */
void *serial_thread(void *ptr);
void *send_serial(void *ptr);

int main()
{
    pthread_t thread1, thread2;
    
    pthread_create(&thread2, NULL, serial_thread, NULL);
    pthread_create(&thread1, NULL, network_thread, NULL);
    
    while (1);
    return 0;
}

void *network_thread(void *ptr)
{
    unsigned char *msg;
    int lenght;
    cout << "Network thread" << endl;
    UdpServer *s = new UdpServer("127.0.0.1", 10001, "127.0.0.1", 15001);
    s->setupUdpSocket();
    cout << "UDP node up and listening on: "
         << "10001" << endl;
    pthread_t send_thread;
    pthread_create(&send_thread, NULL, send_network, s);
    while (1)
    {
        int res = s->receiveUdpMessage();
        if (res > 0)
        {
            cout<<"Received network msg\n";
            msg = s->getData();
            lenght = res;
            printf("received data length: %d\n", lenght);
            printf("received data: ");
            for(int i =0;i<10;i++)
                printf("%d  ", msg[i]);
            printf("\n");
            network_to_serial_queue.push({msg, lenght});
        }
    }
    return NULL;
}

void *send_network(void *ptr)
{
    UdpServer *s = (UdpServer *)ptr;
    pair<unsigned char *, int> msg;
    cout << "Send network thread" << endl;
    while (1)
    {
        if (serial_to_network_queue.size() > 0)
        {
            msg = serial_to_network_queue.front();
            serial_to_network_queue.pop();
            cout << "sending network message\n";
            int res = s->sendUdpMessage(msg.first, msg.second);
        }
    }
    return NULL;
}


void *serial_thread(void *ptr)
{
    pthread_t thread3;
    char *devname;
    int rc;
    unsigned char buf[MAX_FRAME_SIZE];

    
	devname = "/dev/ttySLG0";
    // open with O_NONBLOCK to ignore DCD
	fd = open(devname, O_RDWR | O_NONBLOCK, 0);
	if (fd < 0) {
		printf("open error=%d %s\n", errno, strerror(errno));
	}
    configure_port(fd);
    signal(SIGINT, sigint_handler);
	siginterrupt(SIGINT, 1);
    ioctl(fd, MGSL_IOCRXENABLE, 1);

    cout<<"Serial node running on: /dev/ttySLG0" << endl;

    pthread_create(&thread3, NULL, send_serial, &fd);
    

    while(1)
    {
        rc = read(fd, buf, sizeof(buf));
		if (rc < 0) {
			continue;
		}
        cout<<"Received serial msg\n";
        serial_to_network_queue.push({buf, rc});
    }
    return NULL;
}   

void *send_serial(void *ptr)
{
    cout<<"send serial thread\n";
    int fd = *(int*)ptr;
    unsigned char buf[FRAME_SIZE];
    pair<unsigned char *, int> msg;
    for (int i = 0 ; i < FRAME_SIZE ; i++)
		*(buf + i) = (unsigned char)i;
    while(1)
    {
        if(network_to_serial_queue.size() >0 )
        {
            msg = network_to_serial_queue.front();
            network_to_serial_queue.pop();
            cout<<"Sending serial msg\n";
            int rc = write(fd, (unsigned char *)(msg.first), int(msg.second));
            // int rc = write(fd, buf, FRAME_SIZE);
            if (rc < 0) {
                cout<<"here continue\n";
                continue;
            }
            // block until all sent
            tcdrain(fd);
            cout<<"here after continue\n";
        }
    }
    return NULL;
}

void configure_port(int fd)
{
    int ldisc;
    MGSL_PARAMS params;

    // Set line discipline, a software layer between tty devices
    // and user applications that performs intermediate processing.
    // N_HDLC = frame oriented line discipline
    ldisc = N_HDLC;
    ioctl(fd, TIOCSETD, &ldisc);

    // get, modify and set device parameters
    ioctl(fd, MGSL_IOCGPARAMS, &params);
    params.mode = MGSL_MODE_HDLC;
    params.encoding = HDLC_ENCODING_NRZ;
    params.crc_type = HDLC_CRC_16_CCITT;
    params.loopback = 0;
    params.flags = HDLC_FLAG_RXC_RXCPIN + HDLC_FLAG_TXC_TXCPIN;
    params.clock_speed = 2400;
    ioctl(fd, MGSL_IOCSPARAMS, &params);

    // set transmit idle pattern (sent between frames)
    ioctl(fd, MGSL_IOCSTXIDLE, HDLC_TXIDLE_FLAGS);

    // set receive data transfer size: range=1-256, default=256
    // HDLC protocol must use default 256.
    ioctl(fd, MGSL_IOCRXENABLE, 256 << 16);

    // use blocking reads and writes
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
}