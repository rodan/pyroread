#ifndef __SC_H__
#define __SC_H__

#include <msp430.h>
#include <stdlib.h>
#include "config.h"

// ports
#define IR_SEL              P1SEL
#define IR_DIR              P1DIR
#define IR_IN               P1IN
#define IR_PIN              BIT0

#define OOK_SEL             P1SEL
#define OOK_DIR             P1DIR
#define OOK_OUT             P1OUT
#define OOK_PIN             BIT1

#define I2C_MASTER_DIR      P6DIR
#define I2C_MASTER_OUT      P6OUT
#define I2C_MASTER_IN       P6IN
//port pins
#define I2C_MASTER_SCL      BIT2
#define I2C_MASTER_SDA      BIT3
//Start and Stop delay, most devices need this
#define I2C_MASTER_SDLY		0x01
//for long lines or very fast MCLK, unremark and set
#define I2C_MASTER_DDLY		0x02

//  Micronix Plus pocket pyrometer

#define I2C_PYRO_DIR        P1DIR
#define I2C_PYRO_OUT        P1OUT
#define I2C_PYRO_IN         P1IN
#define I2C_PYRO_SCL        BIT0
#define I2C_PYRO_SDA        BIT1

#define I2C_PYRO_IES        P1IES
#define I2C_PYRO_IFG        P1IFG
#define I2C_PYRO_IE         P1IE
#define I2C_PYRO_SCL_IV     2

#define PYRO_MX_DIR         P1DIR
#define PYRO_MX_OUT         P1OUT
#define PYRO_MX_ACT         BIT2

// 

#define PS_SLAVE_ADDR       0x28
#define OUTPUT_MIN          0
#define OUTPUT_MAX          0x3fff              // 2^14 - 1
#define PRESSURE_MIN        0.0                 // min is 0 for sensors that give absolute values
#define PRESSURE_MAX        206842.7            // 30psi (and we want results in pascals)


#define MCLK_FREQ           8000000
#define USB_MCLK_FREQ       4000000

#define true                1
#define false               0

void main_init(void);

void sleep(void);
void wake_up(void);
void check_events(void);

void opt_power_enable(void);
void opt_power_disable(void);
void charge_enable(void);
void charge_disable(void);
void sw_enable(void);
void sw_disable(void);

uint8_t read_ps(void);

#endif
