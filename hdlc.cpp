// HDLC/SDLC protocol sample
//
// useage: hdlc [device name]
//
// - send data in main thread
// - receive data in receive thread
// 
// == Single Port Use ==
// Connect data and clock outputs to data and clock inputs with
// loopback plug or external cabling. Alternatively, set
// settings.internal_loopback = True.
//
// == Two Port Use ==
// Connect ports with crossover cable that:
// - connects data output of each port to data input of other port
// - connects clock output of one port to clock inputs of both ports
// Run sample on both ports.

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
#include "synclink.h"


// Default max frame size for driver.
// This can be increased with driver load option.
#define MAX_FRAME_SIZE 4096

// size of frames sent in this sample
#define FRAME_SIZE 100

int run = 1;

void sigint_handler(int sigid)
{
	printf("Ctrl-C pressed\n");
	run = 0;
}

void configure_port(int fd) {
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
	fcntl(fd, F_SETFL, fcntl(fd,F_GETFL) & ~O_NONBLOCK);
}

void *receive_func(void *ptr)
{
	int fd = *((int *)ptr);
	int rc;
	unsigned char buf[MAX_FRAME_SIZE];
	int i = 1;
	while (run) {
		rc = read(fd, buf, sizeof(buf));
		if (rc < 0) {
			break;
		}
		printf("<<< %09d received %d bytes\n", i, rc);
		i++;
	}
	return NULL;
}

int main(int argc, char* argv[])
{
	int fd;
	int rc;
	int i;
	unsigned char buf[FRAME_SIZE];
	char *devname;
	pthread_t receive_thread;

	if (argc > 1)
		devname = argv[1];
	else
		devname = "/dev/ttySLG0";

	printf("HDLC sample running on %s\n", devname);

	// open with O_NONBLOCK to ignore DCD
	fd = open(devname, O_RDWR | O_NONBLOCK, 0);
	if (fd < 0) {
		printf("open error=%d %s\n", errno, strerror(errno));
		return errno;
	}

	if (strstr(devname, "USB")) {
		int arg;
		ioctl(fd, MGSL_IOCGIF, &arg);
		// uncomment to select USB interface (RS232,V35,RS422)
		// arg = (arg & ~MGSL_INTERFACE_MASK) | MGSL_INTERFACE_RS422;
		// ioctl(fd, MGSL_IOCSIF, arg);
		if (!(arg & MGSL_INTERFACE_MASK)) {
			printf("USB serial interface must be selected.\n");
			return -1;
		}
	}

	configure_port(fd);

	printf("Press Ctrl-C to stop program.\n");
	signal(SIGINT, sigint_handler);
	siginterrupt(SIGINT, 1);

	ioctl(fd, MGSL_IOCRXENABLE, 1);
	pthread_create(&receive_thread, NULL, receive_func, &fd);

	// prepare send buffer
	for (i = 0 ; i < FRAME_SIZE ; i++)
		*(buf + i) = (unsigned char)i;

	i = 1;
	while (run) {
		printf(">>> %09d send %d bytes\n", i, FRAME_SIZE);
		rc = write(fd, buf, FRAME_SIZE);
		if (rc < 0) {
			break;
		}
		// block until all sent
		tcdrain(fd);
		i++;
	}

	close(fd);
	return 0;
}