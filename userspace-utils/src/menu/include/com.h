#ifndef COM_H
#define COM_H

#include <stdint.h>
typedef enum {
	HANDSHAKE_OK = 0,        /* Handshake successful */
	HANDSHAKE_ERROR = -1,    /* Error during handshake */
	HANDSHAKE_TIMEOUT = -2,  /* Handshake timed out */
	HANDSHAKE_INVALID = -3   /* Invalid response during handshake */
} handshakeRet_t;

typedef struct {
	char cpuArch[8];         /* CPU Archtecture (e.g. "PowerPC") */
	char cpuName[64];        /* CPU Name (e.g. "750CL") */
	char memSize[16];        /* Memory Size (e.g. "88MB") */
	char kernelVer[64];      /* Kernel Version (e.g. "4.19.325-cip121-st5-wii+") */
	char deviceModel[32];    /* Device Model (e.g. "Nintendo Wii") */
	char distroName[64];     /* Distro name (e.g. "ArchPOWER") */
} comDeviceInfo_t;

static uint8_t __attribute__((unused)) PKT_MAGIC[] = {0x7F, 'P', 'K', 'T'};
#define PKT_TYPE_OFFSET 4
#define PKT_DATA_LEN_OFFSET 5
#define PKT_HDR_SIZE 7
#define MAX_PKT_SIZE 65535

#define PKT_INIT_C 0xFF        /* Host to Client: Initialize connection */
#define PKT_INIT_H 0xFE        /* Client to Host: Acknowledge initialization */
#define PKT_TX_INFO 0x01       /* Either: Transmit device info */
#define PKT_TX_INFO_ACK 0x02   /* Either: Acknowledge device info */

extern int COM_Init(void);
extern void COM_Shutdown(void);
extern handshakeRet_t COM_Handshake(void);

extern comDeviceInfo_t comClientDeviceInfo;
extern comDeviceInfo_t comHostDeviceInfo;

#ifdef CLIENT_APP
#define TTY_PATH "/dev/ttyACM0"
#else
#define TTY_PATH "/dev/ttyGS0"
#endif

#endif
