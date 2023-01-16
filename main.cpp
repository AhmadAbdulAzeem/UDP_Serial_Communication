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

using namespace std;



void *receive_func(void *ptr)
{
	UdpServer *s = (UdpServer *)ptr;
    int i = 1;
    unsigned char* buf;
	while (1) {
		int rc = s->receiveUdpMessage();
        buf = s->getData();
		if (rc < 0) {
			continue;
		}
		printf("<<< %09d received %d bytes\n", i, rc);
        for(int i =0;i<10;i++)
            printf("%d  ", buf[i]);
        printf("\n");
		i++;
	}
	return NULL;
}



int main()
{
    UdpServer *s = new UdpServer("127.0.0.1", 10001, "127.0.0.1", 10001);
    int i,rc;
    s->setupUdpSocket();
    unsigned char buf[10];
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, receive_func, s);
    // prepare send buffer
	for (i = 0 ; i < 10 ; i++)
		*(buf + i) = (unsigned char)i;

	i = 1;
	while (1) {
		printf(">>> %09d send %d bytes\n", i, 10);
		rc = s->sendUdpMessage(buf, 10);
		if (rc < 0) {
			break;
		}
		i++;
	}
    return 0;
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           