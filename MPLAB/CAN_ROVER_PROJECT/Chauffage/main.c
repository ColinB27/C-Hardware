/**      Revision A 19/09/22
//       Poste chauffage
//       Mohamed A
//       remis a Julien Regis Boriasse
//       Apres activation, on mesure la temperature dans le vehicule et on la 
//       compare avec une temperature consignie avec un seuil de 0.5C simulé
//       par 2 potentiometres 10kOhms sur une gamme de [-200 a 200] et une 
//       plage de 400. Traite et envoye sur 3 addresses vers 
//       le poste de l information
//       College de Maisonneuve
//       TGE
//       Automne 022
//       adress de la station 0x40 0x41 0x42 0x43 
//       ils sont utilisé pour : reserver, temp. vehicule, temp. consigne,statue
 */
// les libreries //
#include "mcc_generated_files/mcc.h"
#include "stdio.h"
// les definition 
////                           les variables                                ////
//---------------------------------- CAN ---------------------------------------
uCAN_MSG txMessage, rxMessage; // variables pour la communication ecan
//------------------------------------- unsigned int8bits ----------------------
uint32_t udemarrage[2]; // demarrage alexis
// temperature a mesurer et a modifiee 
// temperature assigner et a comparer sec
int uT_vehicule_CAN, uT_consigne_CAN;
uint16_t uT_vehicule_ADC, uT_consigne_ADC;
//----------------------------------- float ------------------------------------
float fT_vehicule, fT_consigne, fvariation;
//-------------------------------------- Etat ----------------------------------
//les etats de la station 
typedef enum {
    idle, eteint, chauffage, climatisation
} mode;
mode statue;
//-------------------------- variables ----------------------------------------
int itemps; //variable incremente par interruption a chq 49.9ms
// -------------- variable interruption et autres etats ------------------------
// Les fonctions appelees en ordre, les modeles en bas du main
//les fonctions d operations du systeme
void LCD();
void clear_screan();
void interruption();
// fonction eCAN
void reception();
//void machine_dEtats();
// les fonctions de mesures
void mesure();
float temperature(uint16_t uibrute);
void controle_seuil();
// les testes
//void test_affichage();
//void test_variation_temperature();
//void bus_statue();
//// ---------------------- PROGRAMME PEINCIPALE ---------------------------////

void main(void) {
    SYSTEM_Initialize(); // Initialize the device
    TMR0_Initialize();
    INTERRUPT_GlobalInterruptEnable(); // Enable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable(); // Disable the Global Interrupts
    INTERRUPT_PeripheralInterruptEnable(); // Enable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();//Disable the Peripheral Interrupts
    TMR0_SetInterruptHandler(interruption); // fonction d interruption  
    // du chauffage ou de la climatisation apres 3 secondes les leds rouge verte
    LCD(); // pour l affichage des testes
    mode statue = idle; // on ne fait qu attendre le demarrage du systeme
//    RXB0CON = 0x60; // pour l instant
    RXM0SIDH = 0xF9; 
    RXM0SIDL = 0xE3; // masque = 1111 1001 111X 0011
    RXF0SIDH = 0x04;// filtre = 0000 0000 001X 0000
    RXF0SIDL = 0x20;
    printf("SYSTEME ON");
    reception();
    while (1) {
        reception();
        if (udemarrage[1] == 0) // les bornes statiques
        {
            IO_RB6_SetLow(); // on eteint l indicateur chauffage si il etait on
            mode statue = eteint; // on a pas besoin d activer le systeme constament
            IO_RB7_SetLow(); // allume l indicateur vert pour la climatisation
        }
        while (CAN_messagesInBuffer() > 0){
            printf("BUS OFF");
        } //Ne fait rien si pas de message reçu
        reception();
        if (udemarrage[1] == 1){
            mesure();
            if(itemps == 4) // 1 seconde
            {
            txMessage.frame.id = 0x41;
            txMessage.frame.dlc = 2;
            txMessage.frame.data0 = (uint8_t)(uT_vehicule_CAN); //le + significatif
            txMessage.frame.data1 = (uint8_t)(uT_vehicule_CAN >>8); // envoyer en 1er
            CAN_transmit(&txMessage);
            }
        if(itemps == 6) // 3 secondes
            { // apres 1.5 secondes
            txMessage.frame.id = 0x42;
            txMessage.frame.dlc = 2;
            txMessage.frame.data0 = (uint8_t)(uT_consigne_CAN); // capteur 1 temperature 1
            txMessage.frame.data1 = (uint8_t)(uT_consigne_CAN >> 8); // data du capteur 1
            CAN_transmit(&txMessage);
        }
        if(itemps == 12) // 1.25 secondes
        {
            controle_seuil();
            itemps = 0; // on r
//            machine_dEtats();
            if (udemarrage[1] == 0) // les bornes statiques
            {
                IO_RB6_SetLow(); // on eteint l indicateur chauffage si il etait on
                mode statue = eteint; // on a pas besoin d activer le systeme constament
                IO_RB7_SetLow(); // allume l indicateur vert pour la climatisation
            }
        }
        }
    }
}
// modeles de fonctions
void LCD(void) {
    EUSART1_Write(0xFE);
    EUSART1_Write(0x41);
    EUSART1_Write(0xFE);
    EUSART1_Write(0x51);
    EUSART1_Write(0xFE);
    EUSART1_Write(0xFE);
    EUSART1_Write(0x46);
}
//interruption avec un flag pour determiner le temps 
// de transmission des trames des temp. et du statue
void interruption(void) {
    itemps++; // apres chaque 250ms on leve un flag
}
void reception(void) {
    //    EUSART1_Write(0xFE);  // ligne 2 x colonne 1
    //    EUSART1_Write(0x45);
    //    EUSART1_Write(0x0D);
    //    printf("RECEIVE"); // test
    //    void bus_test(void)
    CAN_receive(&rxMessage);
    //    printf("%u %d",rxMessage.frame.id); // test
    //    printf("%d %d ",rxMessage.frame.data0,rxMessage.frame.id);
    udemarrage[0] = rxMessage.frame.id;
    udemarrage[1] = rxMessage.frame.data0;
    if (udemarrage[0] == 0x011) {
        if (udemarrage[1] == 0) {
            IO_RB6_SetLow(); // on eteint l indicateur chauffage si il etait on
        mode statue = eteint; // on a pas besoin d activer le systeme constament
        IO_RB7_SetLow(); // allume l indicateur vert pour la climatisation
            
        } else {
            mode statue = idle;
        }
    }
}
//void machine_dEtats(void) {
//    switch (itemps) {
//        case 4: // apres 1 seconde
//            txMessage.frame.id = 0x41;
//            txMessage.frame.dlc = 4;
//            txMessage.frame.data0 = (uint8_t)(uT_vehicule_CAN); //le + significatif
//            txMessage.frame.data1 = (uint8_t)(uT_vehicule_CAN >>8); // envoyer en 1er
//            txMessage.frame.data2 = (uint8_t)(uT_vehicule_CAN >>16); // le decimal flotant
//            txMessage.frame.data3 = (uint8_t)(uT_vehicule_CAN >>24); // le decimal flotant
//            CAN_transmit(&txMessage);
//            break;
//        case 5: // apres 1.5 secondes
//            txMessage.frame.id = 0x42;
//            txMessage.frame.dlc = 4;
//            txMessage.frame.data0 = (uint8_t)(uT_consigne_CAN); // capteur 1 temperature 1
//            txMessage.frame.data1 = (uint8_t)(uT_consigne_CAN >> 8); // data du capteur 1
//            txMessage.frame.data2 = (uint8_t)(uT_consigne_CAN >> 16); // le decimal flotant
//            txMessage.frame.data3 = (uint8_t)(uT_consigne_CAN >> 24);
//            CAN_transmit(&txMessage);
//            break;
//        case 12: // apres 3 secondes
//            controle_seuil();
//            itemps = 0; // on remet le compteur a zero
//            break;
//    }
//}

void mesure(void) {
    uT_vehicule_ADC = ADC_GetConversion(0);
    uT_consigne_ADC = ADC_GetConversion(1);
    // conversion des donnee numerique en une echelle temperature en celsius
    fT_vehicule = temperature(uT_vehicule_ADC);
    fT_consigne = temperature(uT_consigne_ADC);
    // conversion des donnes pour transmission en ecan
    uT_vehicule_CAN = (int)(fT_vehicule);
    uT_consigne_CAN = (int)(fT_consigne);
    uT_vehicule_CAN = (uT_vehicule_CAN *10);
    uT_vehicule_CAN= (~uT_vehicule_CAN) +1;
    uT_consigne_CAN = (uT_consigne_CAN *10);
    uT_consigne_CAN = (~uT_consigne_CAN) +1;
}

float temperature(uint16_t uibrute) {
    float ftemperature;
    float fcelsius;
    ftemperature = (uibrute / 4095.0) * 500; // produit croisé afin d'obtenir la donnée brute en volt
    fcelsius = (ftemperature - 250); // la tension convertie gamme entre -250 a 250
    return fcelsius; // sortir la variable celsius 
}
void controle_seuil(void) {
    fvariation = (fT_consigne - fT_vehicule);
    if ((fvariation < 0.5) && (fvariation > -0.5)) // les bornes statiques
    {
        IO_RB6_SetLow(); // on eteint l indicateur chauffage si il etait on
        mode statue = eteint; // on a pas besoin d activer le systeme constament
        IO_RB7_SetLow(); // allume l indicateur vert pour la climatisation
    } 
    else if (fvariation > 0.5) {
        IO_RB6_SetLow(); // on eteint l indicateur chauffage si il etait on
        mode statue = chauffage; // on diminue la temperature interieur
        IO_RB7_SetHigh(); // allume l indicateur vert pour la climatisation
    }
    else if (fvariation < -0.5) {
        IO_RB7_SetLow();
        mode statue = climatisation;
        IO_RB6_SetHigh();
    }
}
//
//void test_affichage(void) {
//    EUSART1_Write(0xFE); // ligne 1 x colonne 1
//    EUSART1_Write(0x45);
//    EUSART1_Write(0x00);
//    printf("%.1fC %.1fC", fT_vehicule, fT_consigne);
//
//    EUSART1_Write(0xFE); // ligne 2 x colonne 1
//    EUSART1_Write(0x45);
//    EUSART1_Write(0x40);
//    printf("%d %d", uT_vehicule_CAN, uT_consigne_CAN);
//
//    EUSART1_Write(0xFE); // commande de l'ecran     
//    EUSART1_Write(0x45); // ligne 3 x colonne 1
//    EUSART1_Write(0x14);
//    printf("%.1f %d", fvariation, statue);
//
//    EUSART1_Write(0xFE); // commande de l'ecran     
//    EUSART1_Write(0x45); // ligne 4 x colonne 1
//    EUSART1_Write(0x14);
//    printf("%.1f %d", fvariation, statue);
//}
//
//void test_variation_temperature(void) {
//    fvariation = (fT_consigne - fT_vehicule);
//    printf("%f = %f - %f\n", fvariation, fT_consigne, fT_vehicule);
//
//    if (fvariation > 0.5) {
//        printf("(%f > -0.5)chaufage", fvariation);
//    } else if (fvariation < -0.5) {
//        printf("(%f < -0.5)clim", fvariation);
//    }
//}

//void bus_statue(void) {
//    uint8_t busOff;
//    uint8_t nrMsg;
//    IO_RA5_Toggle(); // test clignotement de la led verte
//    while (CAN_messagesInBuffer() < 1) // a verifier
//    {
//        busOff = CAN_isBusOff();
//        nrMsg = CAN_messagesInBuffer();
//
//        EUSART1_Write(0xFE);
//        EUSART1_Write(0x45);
//        EUSART1_Write(0x00);
//        printf("STATUE BUSY = %d", busOff);
//
//        EUSART1_Write(0xFE);
//        EUSART1_Write(0x45);
//        EUSART1_Write(0x40);
//        printf("NOMBRE MSG = %d", nrMsg);
//        break;
//    }
//    __delay_ms(100);
//    EUSART1_Write(0xFE); // test
//    EUSART1_Write(0x51); // test
//    while (CAN_messagesInBuffer() > 0) // a utilise?
//    {
//        busOff = CAN_isBusOff();
//        nrMsg = CAN_messagesInBuffer();
//
//        EUSART1_Write(0xFE); // commande de l'ecran     
//        EUSART1_Write(0x45); // ligne 3 x colonne 1
//        EUSART1_Write(0x14);
//        printf("STATUE LIBRE = %d ", nrMsg);
//
//        EUSART1_Write(0xFE); // commande de l'ecran     
//        EUSART1_Write(0x45); // ligne 4 x colonne 1
//        EUSART1_Write(0x14);
//        printf("NOMBRE MSG = %d", busOff);
//    }
//    __delay_ms(10);
//    EUSART1_Write(0xFE); // test
//    EUSART1_Write(0x51); // test
//}
//
//void bus_test(void) {
//    int test;
//    while (CAN_messagesInBuffer() != 1) // tant qu on n il a pas comunnication 
//    {
//        EUSART1_Write(0xFE);
//        EUSART1_Write(0x46);
//        printf("NON RECU");
//        test = 1;
//
//
//        if (test == 1) {
//            clear_screan();
//            printf("RECU");
//            IO_RA5_SetLow();
//            __delay_ms(1000);
//            IO_RA5_SetHigh();
//            test = 0;
//        }
//    }
//}
//
//void clear_screan(void) {
//    EUSART1_Write(0xFE);
//    EUSART1_Write(0x46);
//    EUSART1_Write(0xFE);
//    EUSART1_Write(0x51);
//}