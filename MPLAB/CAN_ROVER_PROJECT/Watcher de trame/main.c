

#include "mcc_generated_files/mcc.h"

uCAN_MSG temperature; //Créer une variable de type uCAN_MSG
uCAN_MSG vitesse; //Créer une variable de type uCAN_MSG
uCAN_MSG vehicule_demarre; //Créer une variable de type uCAN_MSG
uCAN_MSG mode_transmission;
uCAN_MSG RX__;



//uint16_t adcValeur1=0; //Initialisation adcValeur1
//uint16_t adcValeur2=0; //Initialisation adcValeur2
//uint16_t adcValeur3=0; //Initialisation adcValeur3
//uint8_t adcValeur1_8b=0; //Initialisation adcValeur1
//uint8_t adcValeur2_8b=0; //Initialisation adcValeur2
//uint8_t adcValeur3_8b=0; //Initialisation adcValeur3
int temps=0;
int secondes=0;
uint32_t IDd;
uint8_t Transmission, ClignotantD, ClignotantG, Commande, Diagnostique;
uint8_t Direction, LumiereI, LumiereE, Demarre,LumiereP, Clee, Verouille, Reference;
int8_t Vitesse, TempI, TempC;

void CAN_Traitement(void);
void ETAT_CAN(void);

void compteur(void)
{
    temps++;
    secondes++;
}


void main(void)
{

    SYSTEM_Initialize();
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    
    TMR0_SetInterruptHandler(compteur);
    RXB0CON = 0X60;
    mode_transmission.frame.dlc = 0x01; //Configuaration de la longueur du message vitesse
    mode_transmission.frame.id = 0x21; //Configuaration de l'identificateur du message
    
    vehicule_demarre.frame.dlc = 0x01; //Configuaration de la longueur du message vitesse
    vehicule_demarre.frame.id = 0x11; //Configuaration de l'identificateur du message
    
    vitesse.frame.dlc = 0x01; //Configuaration de la longueur du message vitesse
    vitesse.frame.id = 0x54; //Configuaration de l'identificateur du message
                
    temperature.frame.dlc = 0x04; //Configuaration de la longueur du message vitesse
    temperature.frame.id = 0x44; //Configuaration du id du message temperature
    while (1)
    { 
            
        CAN_Traitement();
//        adcValeur1=ADC_GetConversion(0x00);
//        adcValeur2=ADC_GetConversion(0x01);
//        adcValeur3=ADC_GetConversion(0x02);
//        
//        adcValeur1_8b= ((float)adcValeur1*255/4096);
//        adcValeur2_8b= ((float)adcValeur2*255/4096);
//        adcValeur3_8b= ((float)adcValeur3*255/4096);
        
        if(temps==1)
        {
            temps=0;
            vehicule_demarre.frame.data0 = 0x01;
            vitesse.frame.data0 = 0x01; //valeur de la vitesse du moteur
            mode_transmission.frame.data0 = 0x01;
            //CAN_transmit(&vitesse); //Transmition du message
            //CAN_transmit(&vehicule_demarre); //Transmition du message
            //CAN_transmit(&mode_transmission); //Transmition du message
//            temperature.frame.data0 = 0x01; //Indique le numéro du capteur
//            temperature.frame.data1 = adcValeur1_8b; //valeur du capteur de température 
//            temperature.frame.data2 = 0x02; //Indique le numéro du capteur 
//            temperature.frame.data3 = adcValeur2_8b; //valeur du capteur de température
            
        }
        
        if (secondes == 100)       //pour 1 seconde
        {
            secondes=0;
            //CAN_transmit(&temperature); //Transmition du message temperature
            ETAT_CAN();
            
        }
    }
}


void CAN_Traitement(void) {
   if(CAN_receive(&RX__)){
       
    IDd = RX__.frame.id;
    if(RX__.frame.dlc == 2)
    {
        printf("\n\r ID:%lx Data0 %x Data1 %x ", IDd, RX__.frame.data0, RX__.frame.data1);
    }
    else
    {
        printf("\n\r ID:%lx Data0 %x ", IDd, RX__.frame.data0);
    }
    switch (IDd) {
        case 0x00: 
            Reference = RX__.frame.dlc;
            break;
        case 0x11: 
            Demarre = RX__.frame.data0;
            break;
            
        case 0x12: 
            Clee = RX__.frame.data0;
            break;
            
        case 0x13:
            Verouille = RX__.frame.data0;
            break;
            
        case 0x21: 
            Transmission = RX__.frame.data0;
            break;

        case 0x31: 
            Vitesse = RX__.frame.data0;
            break;

        case 0x32:
            Commande = RX__.frame.data0;
            break;

        case 0x33:
            Diagnostique = RX__.frame.data0;
            break;

        case 0x51:
            ClignotantG = RX__.frame.data0;
            break;

        case 0x52:
            ClignotantD = RX__.frame.data0;
            break;

        case 0x53:
            LumiereI = RX__.frame.data0;
            break;
            
        case 0x54:
            LumiereE = RX__.frame.data0;
            break;

        case 0x61:
            LumiereP = RX__.frame.data0;
            break;
            
        case 0x41:
            TempI = RX__.frame.data0;
            break;
            
        case 0x42:
            TempC = RX__.frame.data0;
            break;
            
    }
   }
}

void ETAT_CAN(void){
    printf("\n\r\n\r Demarre      %x", Demarre);
    printf("\n\r Clee         %x", Clee);
    printf("\n\r Verouille    %x", Verouille);
    printf("\n\r Transmission %x",Transmission );
    printf("\n\r Vitesse      %i", Vitesse);
    printf("\n\r Diagnostique %x", Diagnostique);
    printf("\n\r Commande     %u", Commande);
    printf("\n\r Temperature  %i", TempI);
    printf("\n\r Consigne T   %i", TempC);
    printf("\n\r Clignotant G %x", ClignotantG);
    printf("\n\r Clignotant D %x", ClignotantD);
    printf("\n\r Lumieres I   %x", LumiereI);
    printf("\n\r Lumieres E   %x", LumiereE);
    printf("\n\r Lum Brise    %x\n\r\n\r", LumiereP);

}
    