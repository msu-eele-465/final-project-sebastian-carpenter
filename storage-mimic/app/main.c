#include <msp430fr2310.h>
#include <stdint.h>

#include "../src/storage_mimic_2310.h"
#include "intrinsics.h"

#define INDEX_MAX 3

int main(void)
{

    // stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

	init_storage_mimic();

    // turn on I/O
    PM5CTL0 &= ~LOCKLPM5;

	__enable_interrupt();

    while (1)
	{
		if (!(P2IN & BIT6))
		{
			storage_mimic_read_mode();
		}
	}
}

#pragma vector=PORT2_VECTOR
__interrupt void storage_ISR(void)
{
	// storage access and reset variables
	static uint8_t storage[INDEX_MAX * 4] = {0};
	static uint8_t *storage_pointer_1 = storage;
	static uint8_t *storage_pointer_2 = storage;
	static uint8_t *storage_pointer_1_start = storage;
	static uint8_t *storage_pointer_2_start = storage;

	// register location storage
	static uint8_t address_1 = 0;
	static uint8_t address_2 = 0;

	// should an address be read first?
	static uint8_t expect_address = 1;

	// storage limiter
	static uint8_t storage_index_1 = 0;
	static uint8_t storage_index_2 = 0;

	// read mode -> 2310 writes out
	if (P2IN & BIT6)
	{
		storage_mimic_write_mode();
		expect_address = 1;

		// buffer 2
		if (P2IN & BIT0)
		{
			// read from storage and send to 2355
			storage_mimic_update(*storage_pointer_2);
			storage_pointer_2++;
			storage_index_2++;

			// loop around once at the end
			if (storage_index_2 >= INDEX_MAX)
			{
				storage_index_2 = 0;
				storage_pointer_2 = storage_pointer_2_start;
			}
		}
		// buffer 1
		else
		{
			// read from storage and send to 2355
			storage_mimic_update(*storage_pointer_1);
			storage_pointer_1++;
			storage_index_1++;

			// loop around once at the end
			if (storage_index_1 >= INDEX_MAX)
			{
				storage_index_1 = 0;
				storage_pointer_1 = storage_pointer_1_start;
			}
		}
	}
	// write mode -> 2310 reads in
	else
	{
		storage_mimic_read_mode();

		// buffer 2
		if (P2IN & BIT0)
		{
			if (expect_address)
			{
				// store the beginning of where to write into
				storage_mimic_read(address_2);
				expect_address = 0;
				storage_pointer_2_start = storage;
				storage_pointer_2_start += (uint16_t)(address_2 & 0x03) * INDEX_MAX;
				storage_index_2 = 0;
				storage_pointer_2 = storage_pointer_2_start;
			}
			else
			{
				// write data from 2355 into storage
				storage_mimic_read(*storage_pointer_2);
				storage_pointer_2++;
				storage_index_2++;

				// loop around once at the end
				if (storage_index_2 >= INDEX_MAX)
				{
					storage_index_2 = 0;
					storage_pointer_2 = storage_pointer_2_start;
				}
			}
		}
		// buffer 1
		else
		{
			if (expect_address)
			{
				// store the beginning of where to write into
				storage_mimic_read(address_1);
				expect_address = 0;
				storage_pointer_1_start = storage;
				storage_pointer_1_start += (uint16_t)(address_1 & 0x03) * INDEX_MAX;
				storage_index_1 = 0;
				storage_pointer_1 = storage_pointer_1_start;
			}
			else
			{
				// write data from 2355 into storage
				storage_mimic_read(*storage_pointer_1);
				storage_pointer_1++;
				storage_index_1++;

				// loop around once at the end
				if (storage_index_1 >= INDEX_MAX)
				{
					storage_index_1 = 0;
					storage_pointer_1 = storage_pointer_1_start;
				}
			}
		}
	}

	P2IFG &= ~BIT7;
}
