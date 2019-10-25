#include <ext/xboot/xbootapi.h>
#include "firmware.h"
#include "twi.h"
#include <avr/interrupt.h>

// Modified from the XBoot example
void upgrade_firmware(void)
{
	static uint8_t page_buffer[SPM_PAGESIZE];
	static uint32_t addr = 0;
	static uint16_t page_addr = 0;

	static uint16_t firmware_bytes_recv = 0;
	static uint16_t fsize = 0;
	static uint16_t target_crc;

	uint8_t lsb;
	uint8_t msb;
	uint16_t crc;
	int data_size;

	switch(twi_packet.command)
	{
		case SET_FSIZE: // Set firmware size, reset addr and reset amount of firmware received
			fifo_pull_uint8(&fifo_desc, &lsb);
			fifo_pull_uint8(&fifo_desc, &msb);
			addr = 0; // May never write past a boundary when reading upto fsize bytes and sanity check.
			page_addr = 0;
			firmware_bytes_recv = 0;
			target_crc = 0;
			fsize = msb << 8 | lsb;
			if (fsize > XB_APP_TEMP_SIZE)
			{
				fsize = 0;
			}
			break;
		case SET_CRC: // Set CRC
			fifo_pull_uint8(&fifo_desc, &lsb);
			fifo_pull_uint8(&fifo_desc, &msb);
			target_crc = msb << 8 | lsb;
			break;
		case START_FIRMWARE_UPDATE:
			// Ignore this command if fsize, CRC not set or total firmware received in previous frame.
			if (!fsize || !target_crc || firmware_bytes_recv == fsize)
			{
				break;
			}

			data_size = bytes_received - sizeof(uint8_t);
			if (firmware_bytes_recv + data_size > fsize)
			{
				// Ignore trailing bytes past fsize bytes
				data_size -= (firmware_bytes_recv + data_size) % fsize;
			}

			firmware_bytes_recv += data_size;

			for (int i = 0; i < data_size; i++) // Loop through data we received this frame
			{
				fifo_pull_uint8(&fifo_desc, &page_buffer[page_addr++]);
				if (page_addr >= SPM_PAGESIZE)
				{
					// We now have one full SPM_PAGESIZE page. Write the page, advance one page and reset the index
					if (xboot_app_temp_write_page(addr, page_buffer, 0) != XB_SUCCESS)
					{
						return;
					}
					addr += SPM_PAGESIZE;
					page_addr = 0;
				}
			}

			if (firmware_bytes_recv == fsize)
			{
				// This writes the last page with 0xFF padding
				if (page_addr > 0)
				{
					while (page_addr < SPM_PAGESIZE)
					{
						page_buffer[page_addr++] = 0xFF;
					}
					if (xboot_app_temp_write_page(addr, page_buffer, 0) != XB_SUCCESS)
					{
						return;
					}
				}

				xboot_app_temp_crc16(&crc);

				if (crc != target_crc)
				{
					return;
				}

				if (xboot_install_firmware(target_crc) != XB_SUCCESS)
				{
					return;
				}

				//cleanup();

				xboot_reset();
			}
			break;
		default:
			break;
	}
}