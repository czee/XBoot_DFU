#include <ext/xboot/xbootapi.h>
#include <asf.h>
#include "firmware.h"
#include "twi.h"

int main (void)
{
	sysclk_init();

	init_twi_slave();
	fifo_init(&fifo_desc, twi_packet.data, FIFO_BUFFER_LENGTH);

	irq_initialize_vectors();
	cpu_irq_enable();
	sleepmgr_init();
	sleepmgr_lock_mode(SLEEPMGR_IDLE);

	if (xboot_app_temp_erase() != XB_SUCCESS)
	{
		return;
	}

	while(1)
	{
		if (slave.result == TWIS_RESULT_OK)
		{
			fifo_pull_uint8(&fifo_desc, &twi_packet.command);
			upgrade_firmware();
			twi_packet.command = 0;
			bytes_received = 0; // Prepare for a new TWI packet
		}

		sleepmgr_enter_sleep();
	}
}
