/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/

/** INCLUDES *******************************************************/
#include "system.h"

#include "usb.h"
#include "usb_device_hid.h"

#include "app_device_joystick.h"
#include "app_led_usb_status.h"

void initialiseInterrupts()
{    
    TRISBbits.TRISB0 = 1; // set RB0 as input - external interrupt 0 - INT1
    
    INTCON2bits.RBPU = 0;
    
    TRISBbits.TRISB1 = 1; // set RB1 as input so can read
    
    TRISAbits.TRISA0 = 1; 
    
    TRISDbits.TRISD1 = 1;
    TRISDbits.TRISD2 = 1;
    TRISDbits.TRISD3 = 0;
    LATDbits.LATD3 = 1;
    
    ADCON1bits.PCFG = 0b1111; // set all pins to digital
    
    UCFGbits.FSEN = 1;
    INTCON2bits.INTEDG0 = 0x00; // interrupt on falling edge    
    INTCONbits.INT0IF = 0; // clear interrupt (must be cleared in software)
    INTCONbits.INT0IE = 1; // enable external interrupt INT0
    INTCONbits.GIE = 1; // enable global interrupt bit
}

void initialiseADC()
{
//    TRISAbits.RA0 = 1; // set RA0 as input
//    ADCON1bits.VCFG0 = 0; // set vref+ source as vdd
//    ADCON1bits.VCFG01 = 0; // set vref- source as vss
//    ADCON1bits.PCFG = 0b1110; // configure RA0 as analog pin
//    
//    ADCON0bits.CHS = 0b0000; // set analog channel 0
//    
//    // if this is not set, the wrong value will be read
//    ADCON2bits.ADCS = 0b100; // a/d clock acquisition Fosc/4
//    
//    ADCON2bits.ADFM = 1; // set result to right justified
//    ADCON2bits.ACQT = 0b010; // set acquisition time select to 4 TAD
//    
//    ADRESH = 0; // flush result reg
//    ADRESL = 0; // flush result reg
    //
    //
    //
    //
    TRISBbits.RB3 = 1; //input pin AN9
    TRISBbits.RB4 = 1; //input pin AN8
    
    ADCON1bits.VCFG0 = 0; // set vref+ source as vdd
    ADCON1bits.VCFG01 = 0; // set vref- source as vss
    
    ADCON1bits.PCFG = 0b0101; // set A0 - A9 as analog inputs
    // set analog selection bit in joystick
    
    // if this is not set, the wrong value will be read
    ADCON2bits.ADCS = 0b100; // a/d clock acquisition Fosc/4
    ADCON2bits.ADFM = 1;
    ADCON2bits.ACQT = 0b010; // set acquisition time select to 4 TAD
    ADRESH = 0; // flush result reg
    ADRESL = 0; // flush result reg
}

MAIN_RETURN main(void)
{
    
    initialiseADC();
    initialiseInterrupts();
    SYSTEM_Initialize(SYSTEM_STATE_USB_START);


    // setup usb bits
    UCFGbits.UTRDIS = 0;
    
    
    USBDeviceInit();
    USBDeviceAttach();

    while(1)
    {
        SYSTEM_Tasks();

        #if defined(USB_POLLING)
            // Interrupt or polling method.  If using polling, must call
            // this function periodically.  This function will take care
            // of processing and responding to SETUP transactions
            // (such as during the enumeration process when you first
            // plug in).  USB hosts require that USB devices should accept
            // and process SETUP packets in a timely fashion.  Therefore,
            // when using polling, this function should be called
            // regularly (such as once every 1.8ms or faster** [see
            // inline code comments in usb_device.c for explanation when
            // "or faster" applies])  In most cases, the USBDeviceTasks()
            // function does not take very long to execute (ex: <100
            // instruction cycles) before it returns.
            USBDeviceTasks();
        #endif
        /* If the USB device isn't configured yet, we can't really do anything
         * else since we don't have a host to talk to.  So jump back to the
         * top of the while loop. */
        if( USBGetDeviceState() < CONFIGURED_STATE )
        {
            /* Jump back to the top of the while loop. */
            continue;
        }

        /* If we are currently suspended, then we need to see if we need to
         * issue a remote wakeup.  In either case, we shouldn't process any
         * keyboard commands since we aren't currently communicating to the host
         * thus just continue back to the start of the while loop. */
        if( USBIsDeviceSuspended() == true )
        {
            /* Jump back to the top of the while loop. */
            continue;
        }
            
        //Application specific tasks
        APP_DeviceJoystickTasks();

    }//end while
}//end main

/*******************************************************************************
 End of File
*/

