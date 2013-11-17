
//  software bitbang of serial protocols
//  currently supported:
//        - i2c master
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/
//  license:         GNU GPLv3


#include "pyro_bitbang.h"

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
        if (pyro_p == 40) {
            pyro_p = 0;
            pyro_last_event = I2C_PYRO_SCL_IV;
            _BIC_SR_IRQ(LPM3_bits);
        }
    default:
        break;
    }

    /*
    uint16_t iv = UCA0IV;
    enum uart0_tevent ev = 0;
    register char rx;

    // iv is 2 for RXIFG, 4 for TXIFG
    switch (iv) {
    case 2:
        rx = UCA0RXBUF;
        if (uart0_rx_enable && !uart0_rx_err) {
            if (rx == 0x0a) {
                return;
            } else if (rx == 0x0d) {
                ev = UART0_EV_RX;
                uart0_rx_enable = 0;
                uart0_rx_err = 0;
                _BIC_SR_IRQ(LPM3_bits);
            } else {
                uart0_rx_buf[uart0_p] = rx;
                uart0_p++;
            }
        } else {
            uart0_rx_err++;
            if (rx == 0x0d) {
                uart0_rx_err = 0; 
            }
        }
        break;
    case 4:
        ev = UART0_EV_TX;
        break;
    default:
        break;
    }
    uart0_last_event |= ev;
    */
}

