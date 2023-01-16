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



int main()
{
    UdpServer *s = new UdpServer("127.0.0.1", 10001, "127.0.0.1", 15001);
    int i = 1;
    s->setupUdpSocket();
    const unsigned char* buf;
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
    return 0;
}