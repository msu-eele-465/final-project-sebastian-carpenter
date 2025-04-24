#ifndef I2C_H
#define I2C_H

#include <stdint.h>

// setup P1.2 and P1.3 for i2c communication
// ---> run before turning on GPIO
void init_i2c_pins(void);


/* --- master --- */


// setup the i2c for master mode
// ---> run after turning on GPIO
void configure_i2c_master(void);

/*
*   reading or writing while a read or write operation is in progess
*   will result in a corrupted transmission
*/

// write the given buffer out to the given device
// the buffer will be copied to a transmit buffer and execution will resume immediately
//  the buffer WILL NOT be changed
void i2c_write(uint8_t device_address, uint8_t register_address, uint8_t *buffer, uint8_t count);

// read data from a given device into a given buffer
// the buffer will be given as a pointer which will be read into
//  the buffer WILL be changed
void i2c_read(uint8_t device_address, uint8_t register_address, uint8_t *buffer, uint8_t count);


/* --- slave --- */

// setup I2C for slave mode
// the slave address (self) will be set to what is provided
// the buffer will be what is read and written from
// each register address will store a cetain number of bytes
// ---> run after turning on GPIO
void configure_i2c_slave(uint8_t slave_address, uint8_t *buffer, uint8_t size_of_registers);

#endif
