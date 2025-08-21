/**
//       module demarrage
//       resaux
//       Alexis Brosseau
//       remis a Julien
//       College de Maisonneuve
//       TGE
//       Automne 2022
 */


// les libreries //
#include "mcc_generated_files/mcc.h"
#include "stdio.h"

// les definition 
#define Vehicule_demarre_envoi 0x11 ;   // ID Vehicule_demarre
#define Cle_inseree_envoi 0x12 ;  // ID Cle_inseree
#define Vehicule_Verrouille_envoi 0x13 ;   //  ID Vehicule_Verrouille

//IO_RA0_PORT   = (verrouiller/deverouiller)
//IO_RA1_PORT   = start/stop
//IO_RA2_PORT   = cle_inseree

//// les variables //
uCAN_MSG txMessage;
uCAN_MSG rxMessage;




//inputs
uint8_t mode_transmition = 0;
int8_t vitesse_vehicule = 0;

// output
uint8_t vehicule_demarre = 0x00;
uint8_t cle_inseree = 0x00;
uint8_t vehicule_verrouille = 0x00;

uint32_t MSG_id_recever;

// slot pour le TTCAN 

#define SLOT1_DEMARRAGE 2
#define SLOT2_DEMARRAGE 6
#define SLOT3_DEMARRAGE 11
#define SLOT4_DEMARRAGE 15
#define SLOT1_CLE_verrouille 5
#define SLOT2_CLE_verrouille 14


int compteurTTCAN = 0; // incrémenter cette variable dans l'interrupt du timer 1 (interrurpt a tous les 20ms)

// filtre et mask

//RXM0SIDH = 0xFF;
//RXM0SIDL = 0xE1; // masque = 1111 1111 1110 1111
//
//RXF0SIDH = 0x00;
//RXF0SIDL = 0x20; // filtre = 0000 0000 001X 0001

// message attendue :   de:  0000 0000 0010 0001         0x21 transmition
// message attendue :   a:   0000 0000 0011 0001         0x31 moteur

int flag_interrupt_2 = 0; // variable interruption
int flag_interrupt_1 = 0;
int flag_bouton_verrouille_deja_appuye = 0;
int flag_bouton_demarre_deja_appuye = 0;

// Les fonctions appeles

void interruption_2();
void interruption_1();
void can_transmit();
void reception_TTCAN();


// modeles de fonctions

void interruption_2(void) // interruption de demarage durant 1 seconde pour le changement d etat des variables
{
    flag_interrupt_2 = 1;
}

void interruption_1(void) // interruption TTCAN a chaque 20ms
{
    if (MSG_id_recever != 0x00) // si on recoit le message de referance
    { // pas sure que cette ligne de code va fonctionner
        compteurTTCAN++;

    }

}

void can_transmit(void) { // fonction pour transmettre les données selon la matrice de communication TTCAN

    switch (compteurTTCAN) {

        case SLOT1_DEMARRAGE:

            txMessage.frame.id = Vehicule_demarre_envoi;
            txMessage.frame.dlc = 1;
            txMessage.frame.data0 = vehicule_demarre;
            CAN_transmit(&txMessage);

            break;

        case SLOT2_DEMARRAGE:

            txMessage.frame.id = Vehicule_demarre_envoi;
            txMessage.frame.dlc = 1;
            txMessage.frame.data0 = vehicule_demarre;
            CAN_transmit(&txMessage);


            break;

        case SLOT3_DEMARRAGE:

            txMessage.frame.id = Vehicule_demarre_envoi;
            txMessage.frame.dlc = 1;
            txMessage.frame.data0 = vehicule_demarre;
            CAN_transmit(&txMessage);


            break;


        case SLOT4_DEMARRAGE:

            txMessage.frame.id = Vehicule_demarre_envoi;
            txMessage.frame.dlc = 1;
            txMessage.frame.data0 = vehicule_demarre;
            CAN_transmit(&txMessage);


            break;

        case SLOT1_CLE_verrouille:

            txMessage.frame.id = Cle_inseree_envoi;
            txMessage.frame.dlc = 1;
            txMessage.frame.data0 = cle_inseree;
            CAN_transmit(&txMessage);

            txMessage.frame.id = Vehicule_Verrouille_envoi;
            txMessage.frame.dlc = 1;
            txMessage.frame.data0 = vehicule_verrouille;
            CAN_transmit(&txMessage);


            break;

        case SLOT2_CLE_verrouille:

            txMessage.frame.id = Cle_inseree_envoi;
            txMessage.frame.dlc = 1;
            txMessage.frame.data0 = cle_inseree;
            CAN_transmit(&txMessage);

            txMessage.frame.id = Vehicule_Verrouille_envoi;
            txMessage.frame.dlc = 1;
            txMessage.frame.data0 = vehicule_verrouille;
            CAN_transmit(&txMessage);

            break;

            //default:


    }


}

void reception_TTCAN(void) {

    CAN_receive(&rxMessage);
    MSG_id_recever = rxMessage.frame.id;

    if (MSG_id_recever == 0x00) { // si on recoit le message de referance

        TMR1_Reload();
        compteurTTCAN = 0;

    } else {

        if (MSG_id_recever == 0x21) // si on recoit le message de la transmition
        {
            mode_transmition = rxMessage.frame.data0;

        }

        else if (MSG_id_recever == 0x31) // si on recoit le message de la vitesse
        {
            vitesse_vehicule = (int8_t) rxMessage.frame.data0;

        }

    }

}

typedef enum //enumeration des etapes de la machine d'état
{
    etape_mode_park = 1,
    etape_cle_insere = 2,
    etape_fonction = 3,
    etape_verouiller = 4,
    etape_demarrage = 5,
} demarrage;

void main(void) {


    SYSTEM_Initialize(); // Initialize the device
    TMR2_Initialize();
    TMR1_Initialize();

    INTERRUPT_GlobalInterruptEnable(); // Enable the Global Interrupts
    INTERRUPT_PeripheralInterruptEnable(); // Enable the Peripheral Interrupts


    TMR2_SetInterruptHandler(interruption_2); // fonction d interruption  agit a chaque seconde
    TMR1_SetInterruptHandler(interruption_1); // fonction d interruption  agit a chaque 20ms
    ECAN_SetRXB0InterruptHandler(reception_TTCAN); // interruption lorsqu'il recoit des données sur RXB0

    TMR2_StopTimer();
    TMR1_StopTimer();

    RXB0CON = 0x60;


    demarrage prochaine_etape = etape_mode_park;


    vehicule_demarre = 0x00; // tout set a 0
    cle_inseree = 0x00;
    vehicule_verrouille = 0x00;
    IO_RA3_SetLow();
    IO_RA5_SetLow();

    TMR2_StopTimer();
    TMR1_StartTimer();

    while (1) {

        can_transmit();


        switch (prochaine_etape) //machine d'etat servant a detecter l etat du mode park, si la clee a ete inserer, le demarrage et l arret du moteur, et finalement le verouillage et le deverouillage
        {
            case etape_mode_park:

                vehicule_demarre = 0x00; // tout set a false
                cle_inseree = 0x00;
                vehicule_verrouille = 0x00;

                TMR2_StopTimer();

                IO_RA3_SetLow();
                IO_RA5_SetLow();




                while (mode_transmition != 0x03) { //  reste dans la boucle tant qu il n est pas en mode park

                    can_transmit();


                }

                prochaine_etape = etape_cle_insere;

                break;


            case etape_cle_insere:

                if (IO_RA2_PORT == true) {
                    prochaine_etape = etape_fonction;
                    cle_inseree = 0x01; //true

                } else if (IO_RA0_PORT == true) //condition de l'initiation de verrouiller
                {
                    TMR2_StartTimer();

                    while (flag_interrupt_2 != 1) {

                        can_transmit();
                    }

                    IO_RA3_SetHigh();
                    vehicule_verrouille = 0x01; // true
                    flag_interrupt_2 = 0;
                    TMR2_StopTimer();
                    flag_bouton_verrouille_deja_appuye = 1;
                } else if (IO_RA0_PORT && flag_bouton_verrouille_deja_appuye == 1) //condition de l'initiation deverrouiller
                {
                    TMR2_StartTimer();

                    while (flag_interrupt_2 != 1) {

                        can_transmit();
                    }

                    IO_RA3_SetLow();
                    vehicule_verrouille = 0x00; // false
                    flag_interrupt_2 = 0;
                    TMR2_StopTimer();
                    flag_bouton_verrouille_deja_appuye = 0;

                } else if (IO_RA2_PORT == false) {
                    prochaine_etape = etape_mode_park;

                }

                break;


            case etape_fonction:


                can_transmit();
                TMR2_StopTimer();

                if (IO_RA0_PORT == true) //condition de l'initiation de verrouiller
                {
                    TMR2_StartTimer();

                    while (flag_interrupt_2 != 1) {

                        can_transmit();
                    }

                    IO_RA3_SetHigh();
                    vehicule_verrouille = 0x01; // true
                    flag_interrupt_2 = 0;
                    prochaine_etape = etape_verouiller;
                } else if (IO_RA1_PORT == true && mode_transmition == 3) //condition de l'initiation du demarrage
                {
                    TMR2_StartTimer();

                    while (flag_interrupt_2 != 1) {

                        can_transmit();
                    }

                    IO_RA5_SetHigh();
                    vehicule_demarre = 0x01; // true
                    flag_interrupt_2 = 0;
                    flag_bouton_demarre_deja_appuye = 1;
                    prochaine_etape = etape_demarrage;
                } else if ((vitesse_vehicule >= 15) && (vehicule_demarre == 0x01)) { // si la vitesse du vehicule est superieur à 15km et qu'il est démaré et qu'il est deverouillé 

                    vehicule_verrouille = 0x01; // true
                    IO_RA3_SetHigh(); // il se verouille 

                } else if ((vitesse_vehicule != 0) && (vehicule_demarre == 0x00)) { // si le moteur avance pendant que vechicule n est pas demarrer
                    prochaine_etape = etape_mode_park;

                } else if ((vitesse_vehicule >= 15 || vitesse_vehicule <= -15) && vehicule_demarre == 0x01) { // si la vitesse du vehicule est superieur à 15km ou inferieur a -15km et qu'il est démaré 

                    vehicule_verrouille = 0x01; // true
                    IO_RA3_SetHigh(); // il se verouille 

                }




                if (IO_RA2_PORT == false) {

                    prochaine_etape = etape_mode_park;
                    cle_inseree = 0x00; //false

                }


                break;


            case etape_verouiller:


                can_transmit();
                TMR2_StopTimer();

                if (IO_RA0_PORT == true) // si on re appuit sur le bouton verouilleer  
                {
                    TMR2_StartTimer();

                    while (flag_interrupt_2 != 1) {

                        can_transmit();
                    }

                    IO_RA3_SetLow(); // il se deverouille 
                    vehicule_verrouille = 0x00; // false
                    flag_interrupt_2 = 0;
                    prochaine_etape = etape_fonction;
                }
                else if (IO_RA1_PORT == true && mode_transmition == 3) //condition de l'initiation de demarre
                {
                    TMR2_StartTimer();

                    while (flag_interrupt_2 != 1) {

                        can_transmit();
                    }

                    IO_RA5_SetHigh();
                    vehicule_demarre = 0x01; // true
                    flag_interrupt_2 = 0;
                    flag_bouton_demarre_deja_appuye = 1;
                    prochaine_etape = etape_demarrage;
                } else if ((vitesse_vehicule >= 15 || vitesse_vehicule <= -15) && vehicule_demarre == 0x01) { // si la vitesse du vehicule est superieur à 15km ou inferieur a -15km et qu'il est démaré 

                    vehicule_verrouille = 0x01; // true
                    IO_RA3_SetHigh(); // il se verouille 

                }



                if (IO_RA2_PORT == false) {

                    prochaine_etape = etape_mode_park;
                    cle_inseree = 0x00; //false
                }
                break;


            case etape_demarrage:

                TMR2_StopTimer();

                can_transmit();

                if (IO_RA1_PORT == true && mode_transmition == 3) // si on re appuit sur le bouton demarrer 
                {
                    TMR2_StartTimer();

                    while (flag_interrupt_2 != 1) {

                        can_transmit();
                    }

                    IO_RA5_SetLow(); // le moteur s arrete 
                    vehicule_demarre = 0x00; // false
                    flag_interrupt_2 = 0;
                    prochaine_etape = etape_fonction;
                } else if (IO_RA0_PORT == true) //condition de l'initiation de verrouiller
                {
                    TMR2_StartTimer();

                    while (flag_interrupt_2 != 1) {

                        can_transmit();
                    }

                    IO_RA3_SetHigh();
                    vehicule_verrouille = 0x01; // true
                    flag_interrupt_2 = 0;
                    prochaine_etape = etape_verouiller;

                } else if ((vitesse_vehicule >= 15 || vitesse_vehicule <= -15) && vehicule_demarre == 0x01) { // si la vitesse du vehicule est superieur à 15km ou inferieur a -15km et qu'il est démaré 
                    IO_RA3_SetHigh();
                    vehicule_verrouille = 0x01; // true
                } else if (IO_RA2_PORT == false) {

                    prochaine_etape = etape_mode_park;
                    cle_inseree = 0x00; //false

                }


                break;

        }
    }

}