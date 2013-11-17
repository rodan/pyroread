#ifndef __PYRO_BITBANG_
#define __PYRO_BITBANG_

#include "proj.h"

// i2c_pyro_init error levels
#define I2C_PYRO_OK                  0x0
/*
#define I2C_PYRO_ACK                 0x10
#define I2C_PYRO_NAK                 0x20
#define I2C_PYRO_MISSING_SCL_PULLUP  0x40
#define I2C_PYRO_MISSING_SDA_PULLUP  0x80

#define delay_s     { _NOP(); }
#define delay_c     { _NOP(); _NOP(); _NOP(); }
*/

volatile uint8_t pyro_rx[5];
volatile uint16_t pyro_p;

enum pyro_tevent {
    PYRO_RX = BIT0,
};

volatile enum pyro_tevent pyro_last_event;

/*
// define the SDA/SCL ports
#define I2C_PYRO_DIR P1DIR
#define I2C_PYRO_OUT P1OUT
#define I2C_PYRO_IN  P1IN
#define IC2_PYRO_SCL BIT2
#define I2C_PYRO_SDA BIT3
*/

// read 'length' number of bytes and place them into buf
uint8_t i2c_pyro_rx(uint8_t *buf, const uint16_t length);

// send a 'start' sequence
uint8_t i2c_pyro_init(void);

#endif
