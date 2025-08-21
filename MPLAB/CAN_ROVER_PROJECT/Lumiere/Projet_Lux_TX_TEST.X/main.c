
#include "mcc_generated_files/mcc.h"

void Interuption_10ms(void);

bool intermitent10ms = false;

uCAN_MSG potLVLtx;

uint8_t potLVL = 0;
/*
                         Main application
 */
void main(void)
{
    // Initialize the device
    SYSTEM_Initialize();

    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts
    // Use the following macros to:

    TMR1_SetInterruptHandler(Interuption_10ms);
    
    // Enable the Interrupts
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();


    while (1)
    {
        
        if (IO_RB4_GetValue()){
            potLVL = 1;
        }else if (IO_RB5_GetValue()){
            potLVL = 2;
        }else if (IO_RB6_GetValue()){
            potLVL = 3;
        }else{
            potLVL = 0;
        }
        
         if (intermitent10ms==true) {
            potLVLtx.frame.id = 0x54;
            potLVLtx.frame.dlc = 2;
//            potLVLtx.frame.data0 = 0x01;
            potLVLtx.frame.data0 = potLVL;
            intermitent10ms = false;

            CAN_transmit(&potLVLtx);
            IO_RA5_Toggle();
        }
    }
}



void Interuption_10ms(void) {
    intermitent10ms = true;
};


/**
 End of File
*/