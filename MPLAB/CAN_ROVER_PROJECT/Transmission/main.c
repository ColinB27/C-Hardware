/**
  ============================================================================
  Projet        : Gestion de transmission et feux via CAN
  Fichier       : main.c
  Auteur        : Colin Boule
  Microcontrôleur : PIC18F25K80
  Généré avec   : MCC (PIC10 / PIC12 / PIC16 / PIC18 MCUs - v1.81.8)
  Description   :
      Ce programme gère la transmission (PARK, DRIVE, REVERSE, NEUTRAL) et 
      l’état des feux associés. Il communique via le bus CAN avec d’autres 
      modules pour signaler l’état de la transmission et recevoir des 
      informations (ex. démarrage véhicule).
      
      Les interruptions externes gèrent les changements de transmission,
      et une machine d’état assure la gestion des feux selon l’état courant.
  ============================================================================
**/

#include "mcc_generated_files/mcc.h"

// ================== Énumérations ================== //

typedef enum {
    DRIVE,    // 0 - Transmission en marche avant
    REVERSE,  // 1 - Transmission en marche arrière
    NEUTRAL,  // 2 - Transmission au neutre
    PARK,     // 3 - Transmission au stationnement
    IDLE      // 4 - État inactif
} etat;

etat transmission = IDLE; // État courant de la transmission
etat Lumieres = IDLE;     // État courant des lumières

// ================== Booléens ================== //

bool actif = true;     // Active la mise à jour des lumières
bool Envoie = false;   // Active l’envoi CAN de la transmission
bool demarree = false; // Indique si le véhicule est démarré
bool matrice = false;  // Flag réservé (matrice 2 états)

// ================== CAN ================== //

uCAN_MSG MSG_Recu;
uCAN_MSG MSG_Envoie;

uint32_t IDd = 0xFF;  // Identifiant du message reçu
uint8_t Trame[8];     // Données reçues

int compteurTTCAN;    // Compteur de temporisation CAN

// ================== Fonctions ================== //

void gestion_lumieres(void);
void check_transmission(void);

void interupt_DRIVE(void);
void interupt_REVERSE(void);
void interupt_NEUTRAL(void);
void interupt_PARK(void);
void interupt_SEND(void);

void CAN_Reception(void);
void CAN_Envoie(void);
void CAN_Traitement(void);
void CAN_Filter(void);

// ===================================================================== //
//                                MAIN                                   //
// ===================================================================== //

void main(void) {
    // Initialisation du périphérique
    SYSTEM_Initialize();
    CAN_Filter();

    // Définition de l’ID et de la taille du message CAN
    MSG_Envoie.frame.id = 0x21;
    MSG_Envoie.frame.dlc = 0x01;

    // Configuration des interruptions
    INT0_SetInterruptHandler(interupt_PARK);
    INT1_SetInterruptHandler(interupt_DRIVE);
    INT2_SetInterruptHandler(interupt_REVERSE);
    INT3_SetInterruptHandler(interupt_NEUTRAL);
    TMR1_SetInterruptHandler(interupt_SEND);
    ECAN_SetRXB0InterruptHandler(CAN_Reception); // Réception CAN

    // Activation des interruptions globales et périphériques
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    check_transmission();
    gestion_lumieres();

    while (1) {
        CAN_Envoie(); // Gestion de l’envoi CAN

        if (actif) {
            gestion_lumieres(); // Mise à jour des feux
        }
    }
}

// ===================================================================== //
//                           GESTION LUMIÈRES                           //
// ===================================================================== //

void gestion_lumieres(void) {
    // Machine d’état : PARK forcé tant que le véhicule n’est pas démarré
    switch (demarree) {
        case false:
            Lumieres = PARK;
            break;
        case true:
            Lumieres = transmission;
            break;
    }

    // Commande des sorties en fonction de l’état courant
    switch (Lumieres) {
        case DRIVE:
            IO_RA0_SetLow();
            IO_RA1_SetHigh();
            IO_RA2_SetLow();
            IO_RA3_SetLow();
            break;
        case NEUTRAL:
            IO_RA0_SetLow();
            IO_RA1_SetLow();
            IO_RA2_SetLow();
            IO_RA3_SetHigh();
            break;
        case REVERSE:
            IO_RA0_SetLow();
            IO_RA1_SetLow();
            IO_RA2_SetHigh();
            IO_RA3_SetLow();
            break;
        case PARK:
            IO_RA0_SetHigh();
            IO_RA1_SetLow();
            IO_RA2_SetLow();
            IO_RA3_SetLow();
            break;
        case IDLE:
            IO_RA0_SetLow();
            IO_RA1_SetLow();
            IO_RA2_SetLow();
            IO_RA3_SetLow();
            break;
    }

    actif = false; // Réinitialisation du flag
}

// ===================================================================== //
//                        VÉRIFICATION TRANSMISSION                      //
// ===================================================================== //

void check_transmission(void) {
    if (IO_RB0_GetValue()) {
        transmission = PARK;
    } else if (IO_RB1_GetValue()) {
        transmission = DRIVE;
    } else if (IO_RB2_GetValue()) {
        transmission = REVERSE;
    } else if (IO_RB3_GetValue()) {
        transmission = NEUTRAL;
    }
}

// ===================================================================== //
//                            INTERRUPTIONS                              //
// ===================================================================== //

void interupt_DRIVE(void) {
    transmission = DRIVE;
    actif = true;
    Envoie = true;
}

void interupt_REVERSE(void) {
    transmission = REVERSE;
    actif = true;
    Envoie = true;
}

void interupt_NEUTRAL(void) {
    transmission = NEUTRAL;
    actif = true;
    Envoie = true;
}

void interupt_PARK(void) {
    transmission = PARK;
    actif = true;
    Envoie = true;
}

void interupt_SEND(void) {
    compteurTTCAN++;
    if ((compteurTTCAN == 1) || (compteurTTCAN == 10))
        Envoie = true;
}

// ===================================================================== //
//                                  CAN                                  //
// ===================================================================== //

void CAN_Reception(void) {
    if (CAN_receive(&MSG_Recu)) {
        IDd = MSG_Recu.frame.id;

        Trame[0] = MSG_Recu.frame.data0;
        Trame[1] = MSG_Recu.frame.data1;
        Trame[2] = MSG_Recu.frame.data2;
        Trame[3] = MSG_Recu.frame.data3;
        Trame[4] = MSG_Recu.frame.data4;
        Trame[5] = MSG_Recu.frame.data5;
        Trame[6] = MSG_Recu.frame.data6;
        Trame[7] = MSG_Recu.frame.data7;

        CAN_Traitement();
    }
}

void CAN_Traitement(void) {
    switch (IDd) {
        case 0x11: // Message : véhicule démarré
            switch (Trame[0]) {
                case 0x00:
                    demarree = false;
                    break;
                case 0x01:
                    demarree = true;
                    break;
            }
        case 0x00: // Reset timer CAN
            TMR1_Reload();
            compteurTTCAN = 0;
            break;
    }
}

void CAN_Envoie(void) {
    if (Envoie) {
        MSG_Envoie.frame.data0 = transmission;
        CAN_transmit(&MSG_Envoie);
    }
    Envoie = false;
}

void CAN_Filter(void) {
    // Configuration des masques
    RXM0SIDH = 0xFD;
    RXM0SIDL = 0xC0;

    // Configuration des filtres
    RXF0SIDH = 0x00;
    RXF0SIDL = 0x00;
}
