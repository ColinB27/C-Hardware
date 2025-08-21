/**
 * @author Colin & Louis
 *
 * ITG3200 digital GYRO module.
 *
 * Question : Selon la présentation PowerPoint vue en classe, est-ce que le ITG3200 est un gyroscope ou un gyromètre? 
 * Réponse  : Le ITG3200 est un Gyromètre
 */

#include "mbed.h"
#include "ITG3200.h"


// Blinking rate in milliseconds
#define pause(x)     ThisThread::sleep_for(x);
#define halfS        500ms
#define quarterS        250ms



int main()
{
    // Initialise the digital pin LED1 as an output
    DigitalOut led(LED1);
    ITG3200 GYRO(p9 , p10, 0xD1);
    int16_t iTemp , iValX , iValY , iValZ;
    
    printf("\n\r--------------------------------RESET-------------------------\n\r");

    while (true) {
        led = !led;
        
        // Calcules
        iTemp = (int16_t)((float)((~GYRO.readTEMP())*85) /65535);
        iValX = ((~GYRO.readAXE_X()) * (2000*2) )/65535;
        iValY = ((~GYRO.readAXE_Y()) * (2000*2) )/65535;
        iValZ = ((~GYRO.readAXE_Z()) * (2000*2) )/65535;

        // Bornes Statiques de Temperature
        if (iTemp < -30){   // Borne de Temperature Minimal
            printf("\n\r Temperature minimal depassee ! : %+02i'C \n\n\r" , iTemp);
        }else if (iTemp > 85){  // Borne de Temperature Maximal
            printf("\n\r Temperature maximal depassee ! : %+02i'C \n\n\r" , iTemp);
        }else{
            printf("\n\r Temperature  : %+02i'C \n\n\r" , iTemp);
        }

        // Bornes Statiques d'Axe des X
        if (iValX < -2000){   // Borne de l'Axe X Minimal
            printf(" Axe X minimal depassee ! : %+02i'/s \n\r" , iValX);
        }else if (iValX > 2000){  // Borne de l'Axe X Maximal
            printf(" Axe X maximal depassee ! : %+02i'/s \n\r" , iValX);
        }else{
            printf(" Axe X  : %+02i'/s \n\r" , iValX);
        }

        // Bornes Statiques d'Axe des Y
        if (iValY < -2000){   // Borne de l'Axe Y Minimal
            printf(" Axe Y minimal depassee ! : %+02i'/s \n\r" , iValY);
        }else if (iTemp > 2000){  // Borne de l'Axe Y Maximal
            printf(" Axe Y maximal depassee ! : %+02i'/s \n\r" , iValY);
        }else{
            printf(" Axe Y  : %+02i'/s \n\r" , iValY);
        }

        // Bornes Statiques d'Axe des Z
        if (iValZ < -2000){   // Borne de l'Axe Z Minimal
            printf(" Axe Z minimal depassee ! : %+02i'/s \n\r" , iValZ);
        }else if (iTemp > 2000){  // Borne de l'Axe Z Maximal
            printf(" Axe Z maximal depassee ! : %+02i'/s \n\r" , iValZ);
        }else{
            printf(" Axe Z  : %+02i'/s \n\r" , iValZ);
        }

        pause(quarterS)
    }
}

