
#include "mcc_generated_files/mcc.h"

//******************************************************
// NE PAS CHANGER LES DÉFINES 						  //
//******************************************************

#define SLOT1_DEMARRAGE 2
#define SLOT2_DEMARRAGE 6
#define SLOT3_DEMARRAGE 11
#define SLOT4_DEMARRAGE 15
#define SLOT1_CLE 5
#define SLOT2_CLE 14

#define SLOT1_TRANSMISSION 1
#define SLOT2_TRANSMISSION 10

#define SLOT_CLIGNOTANTS 3
#define SLOT_LUMIERES 7

#define SLOT_TEMPERATUE_CONSIGNE 12
#define SLOT_TEMPERATURE_INT 13

#define SLOT_TEST_LUMIERE 16

#define SLOT1_VITESSE 9
#define SLOT2_VITESSE 17

#define SLOT_COMMANDE 8

//**********************************
// VARIABLE A METTRE DANS LE CODE 
//**********************************
int compteurTTCAN; // incrémenter cette variable dans l'interrupt du timer 1 (interrurpt a tous les 20ms)

//**********************************
// Fonction TT-CAN
//**********************************
void receptionTTCAN(void);
void TTCAN_20ms(void);

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
uint8_t dataCAN0, dataCAN1, lux_State;

uCAN_MSG Lux_check_Tx;

// ------------------------------ Booléennes
bool vStart = false,
        vLock = false,
        vKey = false,
        cligneLock = true,
        pwm_Switch = false,
        cligneLeft = false,
        delayLeft = false,
        changeLeft = false,
        cligneRight = false,
        delayRight = false,
        changeRight = false,
        test_Lux_Int = false;

// ------------------------------ Variables
uint8_t threeLock = 3, timerCAN = 38;

// ------------------------------ Fonctions
void Lock_Blink(void);
void Clignotant_PWM(void);
void CligneL(void);
void CligneR(void);
void CheckVehiculeState(void);
void Default_Output_State(void);
void Etat_LUX_EXT(void);
void GetADC_LuxINT(void);
void printCAN(void);

// ------------------------------ Main

void main(void) {
    // Initialize the device
    SYSTEM_Initialize();

    // interuptions
    TMR1_SetInterruptHandler(TTCAN_20ms); // Set TTCAN 
    ECAN_SetRXB0InterruptHandler(receptionTTCAN);

    TMR2_SetInterruptHandler(Lock_Blink); // Clignote quand on de/verrouille
    TMR3_SetInterruptHandler(Clignotant_PWM); // Clignote quand on veut tourner
    INT0_SetInterruptHandler(GetADC_LuxINT);

    // Enable the Interrupts
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    //    // Mask
    //    RXM0SIDH = 0x0E;
    //    RXM0SIDL = 0xE0;
    //    // Filter
    //    RXF0SIDH = 0x00;
    //    RXF0SIDL = 0x00;
    RXB0CON = 0x60;

    Lux_check_Tx.frame.id = CheckADC;
    Lux_check_Tx.frame.dlc = 0x00;

    Default_Output_State();

    while (1) {

        idCAN = Lux_EXT_Rx.frame.id;
        dataCAN0 = Lux_EXT_Rx.frame.data0;
        dataCAN1 = Lux_EXT_Rx.frame.data1;

        printf("\n Trame CAN :"
                "id     : %02u    "
                "data 0 : %02u    "
                "data 1 : %02u  \n\r", idCAN, dataCAN0, dataCAN1);


        if (!vStart) { // Check Vehicule Demarrer?
            switch (idCAN) {

                case CheckADC:
                    printf("\n Lux interieur :"
                            "data 0 : %02u    "
                            "data 1 : %02u  \n\r", dataCAN0, dataCAN1);
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
                            "data 1 : %02u  \n\r", dataCAN0, dataCAN1);
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
                    break;
                    //------------------------------------
                case Clignotant_Droit: // Verifie etat du clignotant droit
                    if (dataCAN0 == 0x01) {
                        cligneRight = true;
                    } else {
                        cligneRight = false;
                    }
                    printCAN();
                    break;
                    //------------------------------------
                case Lumiere_Ext: // Verifie etat des lumieres 
                    lux_State = dataCAN0;
                    Etat_LUX_EXT();
                    printCAN();
                    //------------------------------------
            }

            CligneL();
            CligneR();

            ///////////////////////////////////////////////////////////////////
        }

        switch (compteurTTCAN) {
            case SLOT_TEST_LUMIERE: // a changer pour mettre slot qui correspond à votre tâche
                if (test_Lux_Int) {
                    while (CAN_transmit(&Lux_check_Tx));
                    test_Lux_Int = false;
                }
                break;
            default:
                break;

        }
    }
}

/*
    End of Main
 */

void Lock_Blink(void) { // A chaque 300 ms

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

};

void Clignotant_PWM(void) {
    pwm_Switch = !pwm_Switch;
}

void CligneL(void) { // Clignote a chaque PWM ms
    if (pwm_Switch) {
        if (changeLeft) {// Clignote cote gauche
            IO_RC2_SetHigh();
        } else {
            IO_RC2_SetLow();
        }
    } else {// Eteint le clignotant
        IO_RC2_SetLow();
    }

    if (cligneLeft != delayLeft) {
        delayLeft = cligneLeft;
    } else {
        if (delayLeft != changeLeft) {
            changeLeft = delayLeft;
        }
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

    if (cligneRight != delayRight) {
        delayRight = cligneRight;
    } else {
        if (delayRight != changeRight) {
            changeRight = delayRight;
        }
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
    delayLeft = false;
    changeLeft = false;
    cligneRight = false;
    delayRight = false;
    changeRight = false;
    idCAN = 0x0000;
    dataCAN0 = 0x00;
    dataCAN1 = 0x00;
    lux_State = 0x00;
    test_Lux_Int = false;
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

//*************************************
//FONCTION A RAJOUTER DANS LE CODE 
//*************************************

void receptionTTCAN(void) {
    uCAN_MSG Message_recu;
    CAN_receive(&Message_recu);

    if (Message_recu.frame.id == 0x00) {
        compteurTTCAN = 0;
        IO_RA5_Toggle();
        TMR1_Reload();
    } else {
        Lux_EXT_Rx = Message_recu;
    }
}

void TTCAN_20ms(void) {
    compteurTTCAN++;
    IO_RA5_Toggle();
}

/*
 // Fonction pour tester fonctionement Lumiere Interne //
 */
void GetADC_LuxINT(void) {
    test_Lux_Int = true;
}

void printCAN(void) {
    printf("\n id: %02lu       data: %02u    \n\r", idCAN, dataCAN0);
}
