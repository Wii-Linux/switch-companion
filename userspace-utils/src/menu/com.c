#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <poll.h>
#include <stdint.h>
#include <assert.h>
#include <arpa/inet.h>

#include "com.h"

static int fd;
static struct pollfd pollFd;
comDeviceInfo_t comClientDeviceInfo;
comDeviceInfo_t comHostDeviceInfo;
static struct termios tty;

int COM_Init(void) {
	if (fd > 0) close(fd); /* just in case */

	fd = open(TTY_PATH, O_RDWR);
	if (fd < 0) {
		perror("Failed to open " TTY_PATH);
		return -1;
	}

	pollFd.fd = fd;
	pollFd.events = POLLIN;

	if (tcgetattr(fd, &tty) != 0) {
		perror("Failed to get terminal attributes");
		return -1;
	}

	/* 115200, 8n1 */
	cfsetispeed(&tty, B115200);
	cfsetospeed(&tty, B115200);
	tty.c_cflag &= ~PARENB; /* No parity */
	tty.c_cflag &= ~CSTOPB; /* 1 stop bit */
	tty.c_cflag &= ~CSIZE;  /* Clear size bits */
	tty.c_cflag |= CS8;     /* 8 data bits */

	/* Turn off modem features */
	tty.c_cflag |= CREAD | CLOCAL;

	/* Turn off canonical mode */
	tty.c_lflag &= ~ICANON;

	/* Turn off echoing (shouldn't matter since icanon is off, but just in case) */
	tty.c_lflag &= ~(ECHO | ECHOE | ECHONL);

	/* Disable signal characters */
	tty.c_lflag &= ~(ISIG | IEXTEN);

	/* Disable software flow control */
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);

	/* Disable all special character handling */
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

	/* Don't mangle output */
	tty.c_oflag &= ~(OPOST | ONLCR);

	/* Set up VMIN and VTIME */
	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 50; /* 5 seconds timeout */

	/* Apply the settings */
	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		perror("Failed to set terminal attributes");
		return -1;
	}

	return 0;
}

void COM_Shutdown(void) {
	if (fd > 0) {
		close(fd);
		fd = -1;
	}

	memset(&comClientDeviceInfo, 0, sizeof(comDeviceInfo_t));
	memset(&pollFd, 0, sizeof(struct pollfd));
	memset(&tty, 0, sizeof(struct termios));
}

static uint8_t buf[MAX_PKT_SIZE];

static void sendPkt(uint8_t pktType, const void *data, uint16_t dataLen) {
	assert(dataLen <= MAX_PKT_SIZE - PKT_HDR_SIZE); /* Ensure we have enough space */
	memcpy(buf, PKT_MAGIC, sizeof(PKT_MAGIC));
	buf[PKT_TYPE_OFFSET] = pktType;
	*(uint16_t *)&buf[PKT_DATA_LEN_OFFSET] = htons(dataLen);
	if (data && dataLen > 0) {
		memcpy(buf + PKT_HDR_SIZE, data, dataLen);
	}

	if (write(fd, buf, PKT_HDR_SIZE + dataLen) < 0) {
		perror("Failed to send packet");
	}
}

static int waitPkt(void) {
	uint16_t dataLen;
	int ret = read(fd, buf, PKT_HDR_SIZE);
	if (ret < 0) {
		perror("Failed to read packet header");
		return 0;
	}
	if (ret < PKT_HDR_SIZE) {
		fprintf(stderr, "Received incomplete packet header (%d < %d)\n", ret, PKT_HDR_SIZE);
		return 0;
	}

	/* Check valid magic */
	if (memcmp(buf, PKT_MAGIC, sizeof(PKT_MAGIC)) != 0) {
		fprintf(stderr, "Invalid packet magic\n");
		return 0;
	}

	/* Check data length, read more if needed */
	dataLen = ntohs(*(uint16_t *)&buf[PKT_DATA_LEN_OFFSET]);
	if (dataLen > MAX_PKT_SIZE - PKT_HDR_SIZE) {
		fprintf(stderr, "Data length exceeds buffer size\n");
		return 0;
	}
	if (dataLen > 0) {
		ret = read(fd, buf + PKT_HDR_SIZE, dataLen);
		if (ret < 0) {
			perror("Failed to read packet data");
			return 0;
		}
		if (ret < dataLen) {
			fprintf(stderr, "Received incomplete packet data (%d < %d)\n", ret, dataLen);
			return 0;
		}
	}
	return 1; /* Successfully read packet */
}

static int cmpPkt(uint8_t pktType) {
	return buf[PKT_TYPE_OFFSET] == pktType;
}

static const uint8_t *pktData(void) {
	assert(ntohs(*(uint16_t *)&buf[PKT_DATA_LEN_OFFSET]) > 0);
	return buf + PKT_HDR_SIZE; /* Return pointer to packet data */
}

handshakeRet_t COM_Handshake(void) {
#ifdef CLIENT_APP
	/* Wait for INIT_C from host */
	if (!waitPkt()) {
		return HANDSHAKE_TIMEOUT;
	}
	if (!cmpPkt(PKT_INIT_C)) {
		return HANDSHAKE_INVALID;
	}

	/* Send INIT_H response */
	sendPkt(PKT_INIT_H, NULL, 0);

	/* Wait for TX_INFO from host */
	if (!waitPkt()) {
		return HANDSHAKE_TIMEOUT;
	}
	if (!cmpPkt(PKT_TX_INFO)) {
		return HANDSHAKE_INVALID;
	}

	/* Send TX_INFO_ACK to host with our info */
	sendPkt(PKT_TX_INFO_ACK, &comClientDeviceInfo, sizeof(comDeviceInfo_t));

	/* Send TX_INFO to host */
	sendPkt(PKT_TX_INFO, NULL, 0);

	/* Wait for TX_INFO_ACK from host */
	if (!waitPkt()) {
		return HANDSHAKE_TIMEOUT;
	}
	if (!cmpPkt(PKT_TX_INFO_ACK)) {
		return HANDSHAKE_INVALID;
	}
	memcpy(&comHostDeviceInfo, pktData(), sizeof(comDeviceInfo_t));
#else
	sendPkt(PKT_INIT_C, NULL, 0); /* Host sends INIT_C to kick off handshake */

	/* Wait for INIT_H from client */
	if (!waitPkt()) {
		return HANDSHAKE_TIMEOUT;
	}
	if (!cmpPkt(PKT_INIT_H)) {
		return HANDSHAKE_INVALID;
	}

	/* Send TX_INFO to client */
	sendPkt(PKT_TX_INFO, NULL, 0);

	/* Wait for TX_INFO_ACK from client */
	if (!waitPkt()) {
		return HANDSHAKE_TIMEOUT;
	}
	if (!cmpPkt(PKT_TX_INFO_ACK)) {
		return HANDSHAKE_INVALID;
	}
	memcpy(&comClientDeviceInfo, pktData(), sizeof(comDeviceInfo_t));

	/* Wait for TX_INFO from client */
	if (!waitPkt()) {
		return HANDSHAKE_TIMEOUT;
	}
	if (!cmpPkt(PKT_TX_INFO)) {
		return HANDSHAKE_INVALID;
	}

	/* Send TX_INFO_ACK to client with our info */
	sendPkt(PKT_TX_INFO_ACK, &comHostDeviceInfo, sizeof(comDeviceInfo_t));
#endif
	return HANDSHAKE_OK;
}
