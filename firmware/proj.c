
//  sensor control board based on a MSP430F5510 uC
//   - compatible with hardware rev 03 -
//
//  author:          Petre Rodan <2b4eda@subdimension.ro>
//  available from:  https://github.com/rodan/
//  license:         GNU GPLv3

#include <stdio.h>
#include <string.h>

#include "proj.h"
#include "calib.h"
#include "drivers/sys_messagebus.h"
#include "drivers/pmm.h"
#include "drivers/rtc.h"
#include "drivers/timer_a0.h"
#include "drivers/uart1.h"
#include "drivers/diskio.h"
#include "drivers/mmc.h"
#include "drivers/adc.h"
#include "drivers/hal_sdcard.h"

#ifdef CONFIG_PYRO_MX
#include "drivers/pyro_mx_bitbang.h"
#endif

// DIR is defined as "0x0001 - USB Data Response Bit" in msp430 headers
// but it's also used by fatfs
#undef DIR
#include "fatfs/ff.h"

char str_temp[64];
FATFS fatfs;
DIR dir;
FIL f;

void die(uint8_t loc, FRESULT rc)
{
    sprintf(str_temp, "l=%d rc=%u\r\n", loc, rc);
    uart1_tx_str(str_temp, strlen(str_temp));
}

static void trigger_measurements(enum sys_message msg)
{
    uint16_t q_vin = 0, q_vin2 = 0, q_vin3 = 0;
    float v_in = 0, v_in2 = 0, v_in3 = 0;

    //pyro_mx_act_low;

    P6SEL |= BIT0;
    P6DIR &= ~BIT0;

    /*
    adc10_read(0, &q_vin, REFVSEL_2);
    if (q_vin < 613) {
        adc10_read(0, &q_vin, REFVSEL_0);
        v_in = q_vin * VREF_1_5_6_0 / 1.023;
    } else if (q_vin < 818) {
        adc10_read(0, &q_vin, REFVSEL_1);
        v_in = q_vin * VREF_2_0_6_0 / 1.023;
    } else {
        v_in = q_vin * VREF_2_5_6_0 / 1.023;
    }
    */

    adc10_read(0, &q_vin, REFVSEL_2);
    v_in = q_vin * VREF_2_5_6_0 / 1.023;

    adc10_read(0, &q_vin2, REFVSEL_1);
    v_in2 = q_vin2 * VREF_2_0_6_0 / 1.023;

    adc10_read(0, &q_vin3, REFVSEL_0);
    v_in3 = q_vin3 * VREF_1_5_6_0 / 1.023;

   

    snprintf(str_temp, 63, "v_in  %d %01d.%03d\r\n", q_vin, (uint16_t) v_in / 1000, (uint16_t) v_in % 1000);
    uart1_tx_str(str_temp, strlen(str_temp));
    snprintf(str_temp, 63, "v_in2 %d %01d.%03d\r\n", q_vin2, (uint16_t) v_in2 / 1000, (uint16_t) v_in2 % 1000);
    uart1_tx_str(str_temp, strlen(str_temp));
    snprintf(str_temp, 63, "v_in3 %d %01d.%03d\r\n", q_vin3, (uint16_t) v_in3 / 1000, (uint16_t) v_in3 % 1000);
    uart1_tx_str(str_temp, strlen(str_temp));
    uart1_tx_str("\r\n",3);
}

#ifdef CONFIG_PYRO_MX
static void display_mx_pyro(enum sys_message msg)
{
    snprintf(str_temp, 64,"rem %u %03d.%01dgC\r\n", pyro_mx.rem_temp_raw, (uint16_t)pyro_mx.rem_temp_float / 10, (uint16_t)pyro_mx.rem_temp_float % 10);
    uart1_tx_str(str_temp, strlen(str_temp));
    snprintf(str_temp, 64,"ref %u %03d.%01dgC\r\n", pyro_mx.ref_temp_raw, (uint16_t)pyro_mx.ref_temp_float / 10, (uint16_t)pyro_mx.ref_temp_float % 10);
    uart1_tx_str(str_temp, strlen(str_temp));
    pyro_mx_act_high;
}
#endif

int main(void)
{
    main_init();
    uart1_init();

#ifdef CONFIG_PYRO_MX
    i2c_pyro_mx_init();
    sys_messagebus_register(&display_mx_pyro, SYS_MSG_PYRO_RDY);
#endif

    sys_messagebus_register(&trigger_measurements, SYS_MSG_RTC_SECOND);

    while (1) {
        sleep();
        //__no_operation();
        //wake_up();
#ifdef USE_WATCHDOG
        // reset watchdog counter
        WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL;
#endif
        check_events();
    }
}

void main_init(void)
{

    // watchdog triggers after 4 minutes when not cleared
#ifdef USE_WATCHDOG
    WDTCTL = WDTPW + WDTIS__8192K + WDTSSEL__ACLK + WDTCNTCL;
#else
    WDTCTL = WDTPW + WDTHOLD;
#endif
    SetVCore(3);

    // select XT1 and XT2 ports
    // select 12pF, enable both crystals
    P5SEL |= BIT5 + BIT4;
    
    // hf crystal
    /*
    uint16_t timeout = 5000;

    P5SEL |= BIT3 + BIT2;
    //UCSCTL6 |= XCAP0 | XCAP1;
    UCSCTL6 &= ~(XT1OFF + XT2OFF);
    UCSCTL3 = SELREF__XT2CLK;
    UCSCTL4 = SELA__XT1CLK | SELS__XT2CLK | SELM__XT2CLK;
    // wait until clocks settle
    do {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
        SFRIFG1 &= ~OFIFG;
        timeout--;
    } while ((SFRIFG1 & OFIFG) && timeout);
    // decrease power
    //UCSCTL6 &= ~(XT2DRIVE0 + XT1DRIVE0);
    */
    UCSCTL6 &= ~(XT1OFF | XT1DRIVE0);

    P1SEL = 0x0;
    P1DIR = 0xff;
    //P1DIR = 0x00;
    P1OUT = 0x00;
    P1REN = 0x00;

    P2SEL = 0x0;
    P2DIR = 0x1;
    P2OUT = 0x0;

    P3SEL = 0x0;
    P3DIR = 0x1f;
    P3OUT = 0x0;

    P4SEL = 0x0;
    P4DIR = 0xff;
    P4REN = 0x0;
    P4OUT = 0x0;

    //P5SEL is set above
    P5DIR = 0x2;
    P5OUT = 0x0;

    P6SEL = 0x0;
    P6DIR = 0x2;
    P6OUT = 0x2;

    PJOUT = 0x00;
    PJDIR = 0xFF;

#ifdef CALIBRATION
    // send MCLK to P4.0
    __disable_interrupt();
    // get write-access to port mapping registers
    //PMAPPWD = 0x02D52;
    PMAPPWD = PMAPKEY;
    PMAPCTL = PMAPRECFG;
    // MCLK set out to 4.0
    P4MAP0 = PM_MCLK;
    //P4MAP0 = PM_RTCCLK;
    PMAPPWD = 0;
    __enable_interrupt();
    P4DIR |= BIT0;
    P4SEL |= BIT0;

    // send ACLK to P1.0
    P1DIR |= BIT0;
    P1SEL |= BIT0;
#endif

    // disable VUSB LDO and SLDO
    USBKEYPID = 0x9628;
    USBPWRCTL &= ~(SLDOEN + VUSBEN);
    USBKEYPID = 0x9600;

    rtca_init();
    timer_a0_init();
}

void sleep(void)
{
    //opt_power_disable();
    // turn off internal VREF, XT2, i2c power
    //UCSCTL6 |= XT2OFF;
    //PMMCTL0_H = 0xA5;
    //SVSMHCTL &= ~SVMHE;
    //SVSMLCTL &= ~(SVSLE+SVMLE);
    //PMMCTL0_H = 0x00;
    _BIS_SR(LPM3_bits + GIE);
    __no_operation();
}

/*
void wake_up(void)
{
    uint16_t timeout = 5000;
    UCSCTL6 &= ~XT2OFF;
    // wait until clocks settle
    do {
        //UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
        UCSCTL7 &= ~(XT1LFOFFG + DCOFFG);
        SFRIFG1 &= ~OFIFG;
        timeout--;
    } while ((SFRIFG1 & OFIFG) && timeout);
}
*/

void check_events(void)
{
    struct sys_messagebus *p = messagebus;
    enum sys_message msg = 0;

    // drivers/rtca
    if (rtca_last_event) {
        msg |= rtca_last_event;
        rtca_last_event = 0;
    }
    // drivers/uart1
    if (uart1_last_event == UART1_EV_RX) {
        msg |= BITB;
        uart1_last_event = 0;
    }
#ifdef CONFIG_PYRO_MX
    // drivers/pyro_bitbang
    if (pyro_mx_last_event) {
        msg |= pyro_mx_last_event << 12;
        pyro_mx_last_event = 0;
    }
#endif
    while (p) {
        // notify listener if he registered for any of these messages
        if (msg & p->listens) {
            p->fn(msg);
        }
        p = p->next;
    }
}

void opt_power_enable()
{
    P1OUT &= ~BIT6;
}

void opt_power_disable()
{
    P1OUT |= BIT6;
    SSI_MASTER_DIR &= ~(SSI_MASTER_SCL + SSI_MASTER_SDA);
}

