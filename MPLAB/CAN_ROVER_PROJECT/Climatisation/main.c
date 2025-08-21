
//mode_transmission 0x21
//Vehicule_demarre 0x11

#include "mcc_generated_files/mcc.h"
#define vitesse_max 25;

//Out
uCAN_MSG temperature_interieure; //Créer une variable de type uCAN_MSG
uCAN_MSG temperature_consigne; //Créer une variable de type uCAN_MSG
uCAN_MSG status; //Créer une variable de type uCAN_MSG
//In
uCAN_MSG rxMessage;
//uCAN_MSG mode_transmission; //Créer une variable de type uCAN_MSG
uCAN_MSG vehicule_demarre; //Créer une variable de type uCAN_MSG

uint16_t adcInterieur = 0; //Initialisation adcValeur1
uint16_t adcConsigne = 0; //Initialisation adcValeur1

uint8_t adcValeur1_8b = 0; //Initialisation adcValeur1

int secondes = 0;

void compteur(void) {
    secondes++;
}

void check_engine(void) {

}

void main(void) {

    SYSTEM_Initialize();
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    //Changement de masque
//    RXM0SIDH = 0xF9; 
//    RXM0SIDL = 0xE3; // masque = 1111 1001 111X 0011
//    
//    RXF0SIDH = 0x04;
//    RXF0SIDL = 0x20; // filtre = 0000 0000 001X 0000
    RXB0CON = 0X60;
    
    TMR0_SetInterruptHandler(compteur);
    INT0_SetInterruptHandler(check_engine);

    temperature_interieure.frame.dlc = 0x01; //Configuaration de la longueur du message vitesse
    temperature_interieure.frame.id = 0x41; //Configuaration de l'identificateur du message

    temperature_consigne.frame.dlc = 0x01;
    temperature_consigne.frame.id = 0x42; //Configuaration de l'identificateur du message

    status.frame.dlc = 0x01;
    status.frame.id = 0x43; //Configuaration de l'identificateur du message
    
    while (1) {
        
        while (CAN_messagesInBuffer() < 1) { //Ne fait rien si pas de message reçu
        }
        
        CAN_receive(&rxMessage); //Reception message
        if (rxMessage.frame.id == 0x11)
        {
            vehicule_demarre = rxMessage;
        }
        
        if(vehicule_demarre.frame.data0 == 0x01)
        {

            adcInterieur = ADC_GetConversion(0x00);
            adcConsigne = ADC_GetConversion(0x01);
           
            float temperature_interieure = (((float) adcInterieur * 400 / 4096) - 200);
            float temperature_consigne = (((float) adcConsigne * 400 / 4096) - 200);
            float difference_temp = ((float)temperature_consigne - temperature_interieure);
//            status.frame.data0 = ((float) adcValeur1 * 100 / 4096);
            
            printf("Temperature Interrieur: %-4.1f", temperature_interieure);
            printf("Temperature Consigne: %-4.1f", temperature_consigne);
            printf("Difference Temperature: %-4.1f", difference_temp);
            
            if(difference_temp <= 0,5){
                IO_RA3_SetHigh(); 
                IO_RA5_SetLow();    
                status.frame.data0 = 0x03; //Chauffage on
            }
            else if(difference_temp >= 0,5){
                IO_RA5_SetHigh();
                IO_RA3_SetLow();
                status.frame.data0 = 0x02; //Climatisation on
            }else {
                IO_RA5_SetLow();
                IO_RA3_SetLow();
                status.frame.data0 = 0x01; //Chauffage et climatisation off
            }
            
            if(temperature_interieure < 0)  //----Interrieur
            {    
                temperature_interieure.frame.data0 = 0x00; //-
                temperature_interieure.frame.data1 = 0x00;
            }
            else if (temperature_interieure >= 0)
            {
                temperature_interieure.frame.data0 = 0x01; //+
                temperature_interieure.frame.data1 = 0x00;
            }
            
            if(temperature_consigne < 0)    //----Consigne
            {    
                temperature_consigne.frame.data0 = 0x00; //-
                temperature_consigne.frame.data1 = 0x00;
            }
            else if (temperature_consigne >= 0)
            {
                temperature_consigne.frame.data0 = 0x01; //+
                temperature_consigne.frame.data1 = 0x00;
            }
            
            if (secondes == 20)     //----seconde
            {
                secondes = 0;
                CAN_transmit(&temperature_interieure); //Transmition du message vitesse_vehicule
                CAN_transmit(&temperature_consigne); //Transmition du message diagnostic_moteur
                CAN_transmit(&status); //Transmition du message pourcentage_commande
            }
        }
    }
}
