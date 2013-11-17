
//  software bitbang of serial protocols
//  currently supported:
//        - i2c master
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/
//  license:         GNU GPLv3


#include "pyro_bitbang.h"
#include "timer_a0.h"

// returns one of I2C_OK, I2C_MISSING_SCL_PULLUP and/or I2C_MISSING_SDA_PULLUP
uint8_t i2c_pyro_init(void)
{
    pyro_p = 0;
    // set both SCL and SDA pins as inputs
    I2C_PYRO_DIR &= ~(I2C_PYRO_SCL | I2C_PYRO_SDA);
    I2C_PYRO_OUT &= ~(I2C_PYRO_SCL | I2C_PYRO_SDA);
    // trigger on a Hi/Lo edge
    I2C_PYRO_IES |= I2C_PYRO_SCL;
    // clear IFG
    I2C_PYRO_IFG &= ~I2C_PYRO_SCL;
    // enable interrupt
    I2C_PYRO_IE |= I2C_PYRO_SCL;

    return I2C_PYRO_OK;
}

__attribute__ ((interrupt(PORT1_VECTOR)))
void PORT1_ISR(void)
{
    uint16_t iv = P1IV;

    switch (iv) {
    case I2C_PYRO_SCL_IV:
        // clear IFG
        P1IFG &= ~I2C_PYRO_SCL;
        if (timer_a0_last_event & TIMER_A0_EVENT_CCR2) {
            // if no CLK signal is received for 1ms, consider the connection lost
            timer_a0_last_event &= ~TIMER_A0_EVENT_CCR2;
            pyro_p = 0;
        }
        if (pyro_p == 0) {
            pyro_rx[0] = 0;
            pyro_rx[1] = 0;
            pyro_rx[2] = 0;
            pyro_rx[3] = 0;
            pyro_rx[4] = 0;
        }
        if (I2C_PYRO_IN & I2C_PYRO_SDA) {
            pyro_rx[pyro_p / 8] |= 1 << (7 - (pyro_p % 8));
        }
        pyro_p++;

        // set up TA0CCR2 for a 1ms trigger
        // this is kinda like timer_a0_delay_noblk(1000), but it's ISR friendly
        TA0CCTL2 &= ~CCIE;
        TA0CCR2 = TA0R + 32; // 1000us/30.51 = 32
        TA0CCTL2 = 0;
        TA0CCTL2 = CCIE;

        // one sentence has 8*5 bits
        if (pyro_p == 40) {
            pyro_p = 0;
            pyro_last_event |= I2C_PYRO_SCL_IV;
            _BIC_SR_IRQ(LPM3_bits);
        }
    default:
        break;
    }
}

