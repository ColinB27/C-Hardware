/**
 * Olivier De Thomasis                               Projet Rover
                Module de contôle des lumières intérieures                   **/

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

#define PorteID 0x13   //ID de porte vérrouillé/dévérouillé
#define ClefID 0x12    //ID clé insérée/non-insérée 
#define Lux_Int 0x53   //ID reception niveau des lumieres

#define CheckADC 0x61  //ID vérification des lumieres

//**********************************
// VARIABLE A METTRE DANS LE CODE 
//**********************************
int compteurTTCAN; // incrémenter cette variable dans l'interrupt du timer 1 (interrurpt a tous les 20ms

uCAN_MSG Lux_INT_Rx; //message de reception
uCAN_MSG Lux_INT_Tx; //message de transmission


uint16_t erreur_0; //erreur adc 0(pin A0)
uint16_t erreur_1; //erreur adc 1(pin A1)

uint16_t lum_precedant;

bool porte_UnLock = true; //vérification dévérouillé/vérouillé
bool clef = false; //vérification clé insérée
bool activation = false; //variable de l'attente de 2 secondes
bool delay2S = true; //variable de l'attente de 2 secondes          

int compteur; //variable de l'attente de 1 minute                   

void receptionTTCAN(void);
void gestion_lumiere(void); //Fonction de gestion des niveaux des lumieres
void erreur_adc(void); //Fonction de detection d'erreur avec l'ADC
void actif(void); //Fonction avec interrupt pour calcul de temps
void gestion_compteur(void); //Fonction avec interrupt pour calcul de temps
void compteurTT(void); //Fonction du compte pour le TTCAN

void main(void) { //rajouter un id qui vérifie les lumieres qaund c'est demandé
    // Initialize the device
    SYSTEM_Initialize();

    //mask
    //RXM0SIDH = 0xF1;
    //RXM0SIDL = 0x80;
    //filter
    RXB0CON = 0x60;
    //RXF0SIDH = 0x00;
    //RXF0SIDL = 0x00;


    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts
    // Use the following macros to:
    TMR2_SetInterruptHandler(gestion_compteur); //Chaque 6 secondes
    TMR1_SetInterruptHandler(compteurTT); //chaque 20 ms
    TMR0_SetInterruptHandler(actif); //Chaque 2 secondes
    ECAN_SetRXB0InterruptHandler(receptionTTCAN);
    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();
    Lux_INT_Tx.frame.id = 0x61; //adresse de transmission 0x61

    while (1) {
        switch (compteurTTCAN) {
            case SLOT_TEST_LUMIERE: //a changer pour mettre slot qui correspond à votre tâche
                ///////////////////////////////////////////////////////////////////
                ////// Transmission de l'état des lumières à chaque minute
                ///////////////////////////////////////////////////////////////////
                if (compteur >= 10) { //si ça fait 1 minute
                    erreur_adc(); //detection d'erreur avec l'ADC
                    (CAN_transmit(&Lux_INT_Tx)); //Transmettre la donnée sur 0x61
                    compteur = 0; //mettre le compteur à 0 
                }

                if (Lux_INT_Rx.frame.id == CheckADC) { //si demande de vérification
                    if (Lux_INT_Rx.frame.dlc == 0) //si la trame est vide
                        printf("\n\rDEMANDE DE VERIFICATION\n\n\r"); //afficher DEMANDE
                    erreur_adc(); //detection d'erreur avec l'ADC
                    CAN_transmit(&Lux_INT_Tx); //Transmettre la donnée sur 0x61
                }
                break;
        }
        //        if (CAN_receive(&Lux_INT_Rx)) { //S'il peut recevoir
        //            IO_RA5_Toggle(); //clignotement de la lumière verte

        if (!clef) { //Si la clef est fausse

            switch (Lux_INT_Rx.frame.id) {

                case PorteID: //véhicule vérouillé/dévérouillé

                    if (Lux_INT_Rx.frame.data0 == 0x00) { //si data0 = 0
                        porte_UnLock = true; //porte dévérouillé        
                    } else { //sinon
                        porte_UnLock = false; //porte vérouillé 
                    }
                    break;

                case ClefID: //clé insérée/non-insérée

                    if ((Lux_INT_Rx.frame.data0 == 0x00) && porte_UnLock) {
                        IO_RA2_SetHigh(); //lumiere avant allumé et arriere 
                        IO_RA3_SetLow(); //fermée
                        delay2S = true; //delay2S est vrai
                        clef = false; //clé insérée
                    } else { //sinon
                        clef = true; //clé non-insérée
                    }
                    break;
            }
        } else {

            if (delay2S) { //si delay2S est vrai
                __delay_ms(2000); //attendre 2 secondes
                delay2S = false; //delay2S est faux
                IO_RA2_SetLow(); //lumiere avant et arriere 
                IO_RA3_SetLow(); //fermée
            }

            switch (Lux_INT_Rx.frame.id) {

                case ClefID: //clé insérée/non-insérée
                    if (Lux_INT_Rx.frame.data0 == 0x00) { //si le data = 0
                        clef = false; //clé insérée
                    } else {//sinon
                        clef = true; //clé non-insérée
                    }
                    break;

                case Lux_Int: //gestion de lumiere
                    gestion_lumiere(); //fonction gestion de lumiere
                    break;
            }
        }
    }
}

void gestion_lumiere(void) {
    {
        switch (Lux_INT_Rx.frame.data0) {
            case 0x00: //niveau 0 
                IO_RA2_SetLow(); //lumiere avant et arriere 
                IO_RA3_SetLow(); //fermée
                break;
            case 0x01: //niveau 1 
                IO_RA2_SetHigh(); //lumiere avant allumée
                IO_RA3_SetLow(); //lumiere arriere fermée
                break;
            case 0x02: //niveau 2 
                IO_RA2_SetLow(); //lumiere avant fermée
                IO_RA3_SetHigh(); //lumiere arriere allumée
                break;
            case 0x03: //niveau 3 
                IO_RA2_SetHigh(); //lumiere avant et arriere 
                IO_RA3_SetHigh(); //allumée

                break;
        }
    }
}

void erreur_adc(void) {
    IO_RA2_SetHigh();
    erreur_0 = ADC_GetConversion(0); //Lecture de L'ADC sur AN0
    IO_RA2_SetLow();

    IO_RA3_SetHigh();
    erreur_1 = ADC_GetConversion(1); //Lecture de L'ADC sur AN1
    IO_RA3_SetLow();

    printf("Erreurs: %4d %4d\r\n\n", erreur_0, erreur_1);

    if (erreur_0 <= 1638 || erreur_0 >= 1966) //Si la lumiere est plus 
    { //petite que 2V et plus 
        Lux_INT_Tx.frame.data0 = 0x01; //grande que 2,4V => erreur
        printf("Etat lumiere avant : ERREUR \n\r");
    } else {
        Lux_INT_Tx.frame.data0 = 0x00; //Sinon état => stable
        printf("Etat lumiere avant: ACTIVE \n\r");
    }

    if (erreur_1 < 1638 || erreur_1 > 1966) //Si la lumiere est plus 
    { //petite que 2V et plus 
        Lux_INT_Tx.frame.data1 = 0x01; //grande que 2,4V => erreur
        printf("Etat lumiere arriere: ERREUR \n\r");
    } else {
        Lux_INT_Tx.frame.data1 = 0x00; //Sinon état => stable
        printf("Etat lumiere arriere: ACTIVE \n\n\r");
    }
}

void actif(void) {

    activation = false;
    IO_RA2_SetLow(); //lumiere avant et arriere 
    IO_RA3_SetLow(); //fermée
    TMR0_StopTimer();
    TMR0_WriteTimer(0);
}

void gestion_compteur(void) {
    compteur++;
}

//*************************************
//FONCTION A RAJOUTER DANS LE CODE 
//*************************************

void receptionTTCAN(void) {
    uCAN_MSG Message_recu;
    CAN_receive(&Message_recu);

    if (Message_recu.frame.id == 0x00) {
        TMR1_Reload();
        compteurTTCAN = 0;
    } else {
        Lux_INT_Rx = Message_recu;
        IO_RA5_Toggle();
    }
}

void compteurTT(void) {
    compteurTTCAN++;
}
