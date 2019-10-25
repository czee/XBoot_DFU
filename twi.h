#include <asf.h>
#include <ext/xboot/xbootapi.h>

#define TWI_SLAVE        TWIC
#define TWI_SLAVE_ADDR   0x34 // 0x68 when using 8-bit addressing read/write bit included
//#define TWI_SLAVE_ADDR   0x4A // 0x94 when using 8-bit addressing read/write bit included

#define FIFO_BUFFER_LENGTH  128

union buffer_element {
	uint8_t  byte;
	uint16_t halfword;
	uint32_t word;
};

enum TWI_Command
{
	SET_FSIZE = 1 << 0,
	SET_CRC = 1 << 1,
	START_FIRMWARE_UPDATE = 1 << 2
} typedef TWI_Command;

typedef struct TWI_Packet_t {
	//volatile uint8_t command;
	//volatile uint8_t* data;
	uint8_t command;
	union buffer_element data[FIFO_BUFFER_LENGTH];
} TWI_Packet_t;

extern TWI_Slave_t slave;
extern TWI_Packet_t twi_packet;
extern volatile uint16_t bytesReceived;
extern fifo_desc_t fifo_desc;
void init_twi_slave(void);
