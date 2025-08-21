
#include "mcc_generated_files/mcc.h"

// ------------------------------ Adresses CAN
#define Vehicule_Demarre 0x11    // 0 = false, 1 = true
#define Cle_Inseree 0x12         // 2 = false, 3 = true
#define Vehicule_Verrouille 0x13 // 4 = false, 5 = true

#define Clignotant_Gauche 0x51 // 0 = false, 1 = true
#define Clignotant_Droit 0x52  // 0 = false, 1 = true
#define Lumiere_Ext 0x54       // level 0 to 3

#define CheckADC    0x61 // test Lux Interieur

// ------------------------------ CAN
uCAN_MSG Lux_EXT_Rx;
uint32_t idCAN;
uint8_t dataCAN0, lux_State;

uCAN_MSG Lux_check_Tx;

// ------------------------------ Booléennes
bool vStart = false,
        vLock = false,
        vKey = false,
        cligneLock = true,
        pwm_Switch = false,
        cligneLeft = false,
        cligneRight = false;

// ------------------------------ Variables
uint8_t threeLock = 3 , timerCAN = 38;

// ------------------------------ Fonctions
void Lock_300ms(void);
void Clignotant_PWM(void);
void CligneL(void);
void CligneR(void);
void CheckVehiculeState(void);
void Default_Output_State(void);
void Etat_LUX_EXT(void);
void printCAN(void);

// ------------------------------ Main

void main(void) {
    // Initialize the device
    SYSTEM_Initialize();

    // interuptions
    TMR1_SetInterruptHandler(Lock_300ms); // Clignote quand on de/verrouille
    TMR3_SetInterruptHandler(Clignotant_PWM); // Clignote quand on veut tourner

    // Enable the Interrupts
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    Lux_check_Tx.frame.id = CheckADC;
    Lux_check_Tx.frame.dlc = 0x00;

    Default_Output_State();

    while (1) {

        // Reception des donnees
        if (CAN_receive(&Lux_EXT_Rx)) {
            idCAN = Lux_EXT_Rx.frame.id;
            dataCAN0 = Lux_EXT_Rx.frame.data0;
            IO_RA5_Toggle();

            if (!vStart) { // Check Vehicule Demarrer?
                switch (idCAN) {

                    case CheckADC:
                        printf("\n Lux interieur :"
                                "data 0 : %02u    "
                                "data 1 : %02u  \n\r", Lux_EXT_Rx.frame.data0, Lux_EXT_Rx.frame.data1);
                        break;

                        //------------------------------------                    
                    case Vehicule_Demarre: // Verifie si vehicule est demarree
                        if (dataCAN0 == 0x01) {
                            vStart = true;
                        } else if (dataCAN0 == 0x00) {
                            vStart = false;
                        }
                        printCAN();
                        break;
                        //------------------------------------
                    case Cle_Inseree: // Verifie si cle est inseree
                        if (dataCAN0 == 0x01) {
                            vKey = true;
                        } else if (dataCAN0 == 0x00) {
                            vKey = false;
                        }
                        printCAN();
                        break;
                        //------------------------------------
                    case Vehicule_Verrouille: // Verifie si vehicule est verroulliee
                        if (dataCAN0 == 0x01) {
                            vLock = false;
                        } else if (dataCAN0 == 0x00) {
                            vLock = true;
                        }
                        printCAN();
                        break;
                        //------------------------------------ 
                }
            } else {
                switch (idCAN) {

                    case CheckADC:
                        printf("\n Lux interieur :"
                                "data 0 : %02u    "
                                "data 1 : %02u  \n\r", Lux_EXT_Rx.frame.data0, Lux_EXT_Rx.frame.data1);
                        break;

                        //------------------------------------
                    case Vehicule_Demarre: // Verifie si vehicule est demarree
                        if (dataCAN0 == 0x01) {
                            vStart = true;
                        } else if (dataCAN0 == 0x00) {
                            vStart = false;
                        }
                        printCAN();
                        break;
                        //------------------------------------
                    case Clignotant_Gauche: // Verifie etat du clignotant gauche
                        if (dataCAN0 == 0x01) {
                            cligneLeft = true;
                        } else if (dataCAN0 == 0x00) {
                            cligneLeft = false;
                        }
                        printCAN();
                        CligneL();
                        break;
                        //------------------------------------
                    case Clignotant_Droit: // Verifie etat du clignotant droit
                        if (dataCAN0 == 0x01) {
                            cligneRight = true;
                        } else {
                            cligneRight = false;
                        }
                        printCAN();
                        CligneR();
                        break;
                        //------------------------------------
                    case Lumiere_Ext: // Verifie etat des lumieres 
                        lux_State = dataCAN0;
                        printCAN();
                        Etat_LUX_EXT();
                        //------------------------------------
                }
            }
            ///////////////////////////////////////////////////////////////////
        }
    }
}

/*
    End of Main
 */

void Lock_300ms(void) { // A chaque 300 ms

    if (cligneLock != vLock) {
        threeLock = 3 * 2;
        cligneLock = vLock;
    }

    if ((threeLock) > 0) { // Verifie s'ils ont clignote
        threeLock--;
        IO_RC2_Toggle();
        IO_RC3_Toggle();
    } else {
        IO_RC2_SetLow();
        IO_RC3_SetLow();
    }

    if (timerCAN == 0){
        while (CAN_transmit(&Lux_check_Tx));
        timerCAN = 38;
    }else{
        timerCAN--;
    }
    
};

void Clignotant_PWM(void) {
    pwm_Switch = !pwm_Switch;
}

void CligneL(void) { // Clignote a chaque PWM ms
    if (pwm_Switch) {
        if (cligneLeft) {// Clignote cote gauche
            IO_RC2_SetHigh();
        } else {
            IO_RC2_SetLow();
        }
    } else {// Eteint le clignotant
        IO_RC2_SetLow();
    }
};

void CligneR(void) { // Clignote a chaque PWM ms
    if (pwm_Switch) {
        if (cligneRight) {// Clignote cote droit
            IO_RC3_SetHigh();
        } else {
            IO_RC3_SetLow();
        }
    } else {// Eteint le clignotant
        IO_RC3_SetLow();
    }

};

void CheckVehiculeState(void) { // Verifie Etat
    if (vStart) {
        IO_RC4_Toggle();
        IO_RC4_Toggle();
    } else if (!vKey) {
        if (lux_State != 0x00) {
            lux_State = 0x00;
        }
        if (vLock) {
            IO_RC0_SetLow();
            IO_RC1_SetLow();
            IO_RC4_SetLow();
        } else {
            IO_RC0_SetHigh();
            IO_RC1_SetLow();
            IO_RC4_SetHigh();
        }
    } else {
        Default_Output_State(); // Reinitialise tous les varibles
    }
}

void Default_Output_State(void) { // Reinitialise les varibles
    IO_RC0_SetLow();
    IO_RC1_SetLow();
    IO_RC2_SetLow();
    IO_RC3_SetLow();
    IO_RC4_SetLow();
    vStart = false;
    vLock = false;
    vKey = false;
    cligneLock = false;
    pwm_Switch = false;
    cligneLeft = false;
    cligneRight = false;
    idCAN = 0x0000;
    dataCAN0 = 0x00;
    lux_State = 0x00;
}

void Etat_LUX_EXT(void) { // Verifie niveau des lumieres
    switch (lux_State) {
        case 0x00:
            IO_RC0_SetLow();
            IO_RC1_SetLow();
            break;
        case 0x01:
            IO_RC0_SetHigh();
            IO_RC1_SetLow();
            break;
        case 0x02:
            IO_RC0_SetLow();
            IO_RC1_SetHigh();
            break;
        case 0x03:
            IO_RC0_SetHigh();
            IO_RC1_SetHigh();
            break;
    }

}

void printCAN(void) {
    printf("\n id: %02lu       data: %02u    \n\r", idCAN, dataCAN0);
}
