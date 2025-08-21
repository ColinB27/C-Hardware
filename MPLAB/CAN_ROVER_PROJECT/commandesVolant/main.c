/*
 Commandes Volant
 * Albert et OliQ
 * 
 *  Ce programme controle les commandes sur le volant de la navette
 *  Information envoyer : 
 *      - Clignotant Gauche allumer ou fermer 
 *      - Clignotant Droit allumer ou fermer 
 *      - État de la lumière Extérieur (valeur entre 0 et 3)
 *      - État de la lumière Intérieur (Valeur entre 0 et 3)
 *      
 *      Note : Aucune information ne peu etre envoyer tant que le démarrage n'a pas été fait
 */

#include "mcc_generated_files/mcc.h"

uint8_t flagClignotantD = 0;
uint8_t flagClignotantG = 0;
uint8_t flagExt = 0;
uint8_t flagInt = 0;
//uint8_t lumIntPrecedent = 5;
//uint8_t lumExtPrecedent = 5;
uint16_t lumiere_ext;
uint16_t lumiere_int;
uint8_t etatLumiereInt; // valeur transmise (entre 0,1,2,3) 
uint8_t etatLumiereExt; // valeur transmise (entre 0,1,2,3) 

uint8_t demarrage = 0; //


uCAN_MSG msg;

int compteur; // compter le delai entre chaque envoie

void lecture_lumieres(void);
void detectionClignotant(void);
void gestionClignotantD(void);
void gestionClignotantG(void);
void transmissionLumiereInt(void);
void transmissionLumiereExt(void);
void receptionDemarrage(void);

void main(void) {
    // Initialize the device
    SYSTEM_Initialize();

    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    RXB0CON = 0x60; // pour la réception 


    TMR1_SetInterruptHandler(detectionClignotant); // pour l'instant a tout les 100Ms mais on 
    // devrais faire une interrupt sur rising edge

    while (1) {



        //        if (compteur >= 10) // pour l'instant envoi l'info des clignotant a tout les secondes 
        //        {
        //            lecture_lumieres();
        //            gestionClignotant(); // encoi info sur clignotant 
        //            transmissionLumiereInt(); // envoi info sur lumière intérieur (0,1,2,3))
        //            transmissionLumiereExt(); // envoi info sur lumière extérieur (0,1,2,3))
        //            compteur = 0;
        //        }
        
        if (CAN_messagesInBuffer() < 1) { //Ne fait rien si pas de message reçu
            receptionDemarrage();   // regarde si la navette est démarrer
        }
        
        
        if (demarrage == 1) {
            if (compteur >= 10) // pour l'instant envoi l'info des clignotant a tout les secondes 
            {
                lecture_lumieres();

                transmissionLumiereInt(); // envoi info sur lumière intérieur (0,1,2,3))
                transmissionLumiereExt(); // envoi info sur lumière extérieur (0,1,2,3))  
                gestionClignotantD(); // 0x52 
                gestionClignotantG(); // envoi info sur clignotant 0x51
            }
        }

    }
}


// ******faire 2 fonctions une pour ext et int et mettre des returns 

void lecture_lumieres(void) { // mettre une condition pour qu'il ne change pas entre 2 valeurs quand il sont trop proche 

    lumiere_ext = ADC_GetConversion(channel_AN0);
    lumiere_int = ADC_GetConversion(channel_AN3);

    if (lumiere_ext > 4095) // pour éviter d'aller dans les chiffres négatif 
    {
        lumiere_ext = 0;
        etatLumiereExt = 0;
    }
    if (lumiere_int > 4095) {
        lumiere_int = 0;
        etatLumiereInt = 0;
    }

    if (lumiere_ext < 1023) {
        printf("ext = 0");
        etatLumiereExt = 0;
    }

    if (lumiere_ext > 1023 && lumiere_ext < 2046) {
        printf("ext = 1");
        etatLumiereExt = 1;
    }

    if (lumiere_ext > 2046 && lumiere_ext < 3069) {
        printf("ext = 2");
        etatLumiereExt = 2;
    }


    if (lumiere_ext > 3069) {
        printf("ext = 3");
        etatLumiereExt = 3;

    }
    printf("\n\r");
    //    printf("\n\r");
    //    printf("ext brute  %d", lumiere_ext);
    //    printf("\n\r");
    //    printf("int brute %d", lumiere_int);
    //    printf("\n\r");

    if (lumiere_int < 1023) {
        printf("int = 0");
        etatLumiereInt = 0;
    }

    if (lumiere_int > 1023 && lumiere_int < 2046) {
        printf("int = 1");
        etatLumiereInt = 1;
    }

    if (lumiere_int > 2046 && lumiere_int < 3069) {
        printf("int = 2");
        etatLumiereInt = 2;
    }

    if (lumiere_int > 3069) {
        printf("int 3");
        etatLumiereInt = 3;
    }
    printf("\n\r");
    printf("\n\r");


}

void transmissionLumiereInt(void) {

    msg.frame.id = 0x53;
    msg.frame.dlc = 1;
    msg.frame.data0 = etatLumiereInt; // envoi de la valeur de la lumiere interrieur
    while (CAN_transmit(&msg) == 0); // attend que la tansmission sois terminer avant de continuer


}

void transmissionLumiereExt(void) {

    msg.frame.id = 0x54;
    msg.frame.dlc = 1;
    msg.frame.data0 = etatLumiereExt; // envoi de la valeur de la lumiere exterieur
    while (CAN_transmit(&msg) == 0); // attend que la tansmission sois terminer avant de continuer

}

void gestionClignotantD(void) {
    if (flagClignotantD == 1) // pour débug
    {
        printf("\n\r btn D high");
        printf("\n\r");
    }
    msg.frame.id = 0x52;
    msg.frame.dlc = 1;
    msg.frame.data0 = flagClignotantD; // envoi de la valeur du clignotant Droite
    while (CAN_transmit(&msg) == 0); // attend que la tansmission sois terminer avant de continuer
}

void gestionClignotantG(void) {

    if (flagClignotantG == 1) // pour débug
    {
        printf("\n\r btn G high");
        printf("\n\r");
    }
    msg.frame.id = 0x51;
    msg.frame.dlc = 1;
    msg.frame.data0 = flagClignotantG; // envoi de la valeur du clignotant Gauche
    while (CAN_transmit(&msg) == 0);

}

void detectionClignotant(void) // interrupt 
{
    flagClignotantG = IO_RB5_GetValue(); // valeur entre 0 et 1  
    flagClignotantD = IO_RB7_GetValue(); // valeur entre 0 et 1 
    compteur++;
}

void receptionDemarrage(void) {
    uCAN_MSG Message_recu;
    CAN_receive(&Message_recu);

    if (Message_recu.frame.id == 0x11) {
        demarrage = Message_recu.frame.data0;
    }

}
