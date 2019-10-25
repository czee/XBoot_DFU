#include <asf.h>
#include "twi.h"
#include "firmware.h"

TWI_Packet_t twi_packet;
TWI_Slave_t slave = {0};
volatile uint16_t bytes_received = 0;
fifo_desc_t fifo_desc;

ISR(TWIC_TWIS_vect)
{
	TWI_SlaveInterruptHandler(&slave);
}

void slave_process(void)
{
	bytes_received++;

	// Strange implementation of slave.bytesReceived in ASF
	fifo_push_uint8_nocheck(&fifo_desc, slave.receivedData[slave.bytesReceived]);
}

void init_twi_slave(void)
{
	//TWIC_CTRL=0x00;
	sysclk_enable_peripheral_clock(&TWI_SLAVE);

	TWI_SlaveInitializeDriver(&slave, &TWI_SLAVE, *slave_process);
	TWI_SlaveInitializeModule(&slave, TWI_SLAVE_ADDR, TWI_SLAVE_INTLVL_HI_gc);
}
