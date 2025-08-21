#include "mcc_generated_files/mcc.h"

#define vitesse_max 25

#define SLOT_REFERENCE 0
#define SLOT1_FREE 4
#define SLOT2_FREE 19

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


uCAN_MSG vitesse_vehicule; //Créer une variable de type uCAN_MSG
uCAN_MSG diagnostic_moteur; //Créer une variable de type uCAN_MSG
uCAN_MSG pourcentage_commande; //Créer une variable de type uCAN_MSG

uCAN_MSG rxMessage;
uCAN_MSG mode_transmission; //Créer une variable de type uCAN_MSG
uCAN_MSG vehicule_demarre; //Créer une variable de type uCAN_MSG
uCAN_MSG message_reference; //Créer une variable de type uCAN_MSG

uint16_t adcValeur1 = 0; //Initialisation adcValeur1
int8_t vitesse;

uint8_t check = 0;
uint8_t adcValeur1_8b = 0; //Initialisation adcValeur1
uint8_t permission_envoie = 1;

int etat_engine = 0;

int compteurTTCAN = 0; // incrémenter cette variable dans l'interrupt du timer 1 (interrurpt a tous les 20ms)

void compteur(void) {

    permission_envoie = 1;
    compteurTTCAN++;
    if (compteurTTCAN >= 20) {
        compteurTTCAN = 0;
    }
}

void check_engine(void) {

    IO_RA5_Toggle();
    if (etat_engine == 0) {
        check = 1;
        etat_engine = 1;
    } else if (etat_engine == 1) {
        check = 0;
        etat_engine = 0;
    }
}

void ecan_rx_isr(void) {

    CAN_receive(&rxMessage);
    switch (rxMessage.frame.id) {
        case 0x11:
            vehicule_demarre = rxMessage;
        case 0x21:
            mode_transmission = rxMessage;
    }
}

void traitement(void) {
    
    adcValeur1 = ADC_GetConversion(0x00);
    if (vehicule_demarre.frame.data0 == 0x01) {
        if (check == 1) {
            diagnostic_moteur.frame.data0 = 1;
        } else if (check == 0) {
            diagnostic_moteur.frame.data0 = 0;
        }
        
        pourcentage_commande.frame.data0 = (uint8_t) ((float) adcValeur1 * 100 / 4096);
        if (mode_transmission.frame.data0 == 0x00) {
            vitesse = (int8_t) ((float) pourcentage_commande.frame.data0 * vitesse_max / 100);
            vitesse_vehicule.frame.data0 = vitesse;

        } else if (mode_transmission.frame.data0 == 0x01) {
            vitesse = (int8_t) ((float) pourcentage_commande.frame.data0 * vitesse_max / 100);
            vitesse = vitesse * -1;
            vitesse_vehicule.frame.data0 = vitesse;
        } else {
            vitesse_vehicule.frame.data0 = 0x00;
        }
    }
}

void main(void) {

    SYSTEM_Initialize();

    TMR1_SetInterruptHandler(compteur);
    INT0_SetInterruptHandler(check_engine);
    ECAN_SetRXB0InterruptHandler(ecan_rx_isr); // interrupt sur CAN
    
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    //Changement de masque
    RXM0SIDH = 0xF9;
    RXM0SIDL = 0xC3; // masque = 1111 1001 110X 0011

    RXF0SIDH = 0x00;
    RXF0SIDL = 0x00; // filtre = 0000 0000 000X 0000


    vitesse_vehicule.frame.dlc = 0x01; //Configuaration de la longueur du message vitesse
    vitesse_vehicule.frame.id = 0x31; //Configuaration de l'identificateur du message

    pourcentage_commande.frame.dlc = 0x01;
    pourcentage_commande.frame.id = 0x32; //Configuaration de l'identificateur du message

    diagnostic_moteur.frame.dlc = 0x01;
    diagnostic_moteur.frame.id = 0x33; //Configuaration de l'identificateur du message

    message_reference.frame.dlc = 0x40;
    message_reference.frame.id = 0x00; //Configuaration de l'identificateur du message
        
    while (1) {
        //        while (CAN_messagesInBuffer() > 0) { //Ne fait rien si pas de message reçu
        //        }
        

        switch (compteurTTCAN) {
//            case SLOT_REFERENCE:
//                if (permission_envoie == 1) {
//                    CAN_transmit(&message_reference); //Transmition du message message_reference
//                    permission_envoie = 0;
//                }
//                break;

            case SLOT1_FREE:
                if (CAN_messagesInBuffer() > 0) {
                    IO_RA5_Toggle();
                }
                break;

            case SLOT_COMMANDE:
                traitement();
                if (permission_envoie == 1) {
                    CAN_transmit(&pourcentage_commande); //Transmition du message pourcentage_commande
                    permission_envoie = 0;
                }
                break;

            case SLOT1_VITESSE: // a changer pour mettre slot qui correspond à votre tâche
                traitement();
                if (permission_envoie == 1) {
                    CAN_transmit(&vitesse_vehicule); //Transmition du message vitesse_vehicule 
                    permission_envoie = 0;
                }
                break;

            case SLOT2_VITESSE: // a changer pour mettre slot qui correspond à votre tâche
                traitement();
                if (permission_envoie == 1) {
                    CAN_transmit(&vitesse_vehicule); //Transmition du message vitesse_vehicule 
                    permission_envoie = 0;
                }
                break;

            case SLOT2_FREE:
                if (CAN_messagesInBuffer() > 0) {
                    IO_RA5_Toggle();
                }
                break;

            default:
                break;
        }
    }
}
