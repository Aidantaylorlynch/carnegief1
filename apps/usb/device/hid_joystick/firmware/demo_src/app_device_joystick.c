#ifndef USBJOYSTICK_C
#define USBJOYSTICK_C

#include "usb.h"
#include "usb_device_hid.h"
#include "system.h"
#include "app_led_usb_status.h"
#include "stdint.h"
#include "system.h"

int steeringValue = 0;
int previous = 0;

typedef union lol_INTPUT_CONTROLS_TYPEDEF
{
    struct
    {
        //struct
        //{
//            uint8_t square:1;
//            uint8_t x:1;
//            uint8_t o:1;
//            uint8_t triangle:1;
//            uint8_t L1:1;
//            uint8_t R1:1;
//            uint8_t L2:1;
//            uint8_t R2:1;//
//            uint8_t select:1;
//            uint8_t start:1;
            //uint8_t a:1;
            //uint8_t b:1;
            //uint8_t c:1;
            //uint8_t :5;    //filler
        //} buttons;
//        struct
//        {
//            uint8_t hat_switch:4;
//            uint8_t :4;//filler
//        } hat_switch;
        struct
        {
            int16_t X;
            int16_t accel;
            int16_t brake;
            uint8_t button1:1;//button
            uint8_t button2:1;//filler
            uint8_t button3:1;
            uint8_t filler:5;
        } analog_stick;
    } members;
} Wheel;

Wheel joystick_input JOYSTICK_DATA_ADDRESS;
USB_VOLATILE USB_HANDLE lastTransmission = 0;


void APP_DeviceJoystickInitialize(void)
{  
    lastTransmission = 0;
    USBEnableEndpoint(JOYSTICK_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
}


void APP_DeviceJoystickTasks(void)
{  
    if( USBGetDeviceState() < CONFIGURED_STATE )
    {
        return;
    }

    if( USBIsDeviceSuspended() == true )
    {
        return;
    }
    

    if(!HIDTxHandleBusy(lastTransmission)) 
    {
        //comment out below to make analog work
        ADCON0bits.CHS = 0b1000; // select channel 8
        ADRESH = 0;
        ADRESL = 0;
        ADCON0bits.GO = 1;
        ADCON0bits.ADON = 1; // enable a/d convertor module
        ADCON0bits.GODONE = 1; // begin
        while(ADCON0bits.GO_nDONE==1); // wait for conversion
        int accel = (ADRESH*256U) | (ADRESL);
        joystick_input.members.analog_stick.accel = accel;
        
        ADCON0bits.CHS = 0b1001; // select channel 9
        ADRESH = 0;
        ADRESL = 0;
        ADCON0bits.GO = 1;
        ADCON0bits.ADON = 1; // enable a/d convertor module
        ADCON0bits.GODONE = 1; // begin
        while(ADCON0bits.GO_nDONE==1); // wait for conversion
        int brake = (ADRESH*256U) | (ADRESL);
        joystick_input.members.analog_stick.brake = brake;
        
        if (steeringValue > 512)
        {
            steeringValue = 512;
        }
        else if (steeringValue < -512)
        {
            steeringValue = -512;
        }

        if (PORTDbits.RD2 == 0)
        {
            joystick_input.members.analog_stick.button3 = 1; //button 3 orange+grey == left shift
        }
        else
        {
          joystick_input.members.analog_stick.button3 = 0;
        }

        if (PORTDbits.RD1 == 0)
        {
            joystick_input.members.analog_stick.button2 = 1; //button 2 red+purple == right shift
        }
        else
        {
          joystick_input.members.analog_stick.button2 = 0;
        }

        if (PORTAbits.RA0 == 0)
        {
            joystick_input.members.analog_stick.button1 = 1; //button 1 drs
        }
        else
        {
          joystick_input.members.analog_stick.button1 = 0;
        }

        joystick_input.members.analog_stick.X = steeringValue;


        lastTransmission = HIDTxPacket(JOYSTICK_EP, (uint8_t*)&joystick_input, sizeof(joystick_input));
        previous = steeringValue;
    }
          
}

void __interrupt() SYS_InterruptHigh(void)
{
    // get other pin state
    static int8_t lookup[] = {-1, 1, 0, 0, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static uint8_t enc_val = 0;
    
    enc_val = enc_val << 2; //shift previous state bits to the left by 2 to make room for new reading
    
    enc_val = enc_val | PORTBbits.RB1;
    
    steeringValue = steeringValue + lookup[enc_val & 0b1111]; //bit mask 4 places
    
    INTCONbits.INT0IF = 0;
}

#endif
