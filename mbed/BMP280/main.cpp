#include "mbed.h"
#include "lcm12864.h"
#include "BMP280.h"
#include <cstdint>
#include <cstdio>

#define wait(x) ThisThread::sleep_for(x);
#define oneS wait(1000ms);
#define halfS wait(500ms);

// Machine etat : Depart / Loop
typedef enum{
    set,
    all
}storing;
storing storePRESSURE = set;


// Entrees & Sorties
DigitalOut led(LED1);
DigitalIn Bouton(p15);
BufferedSerial pc(USBTX, USBRX, 9600);
BMP280 capteur(I2C_SDA1, I2C_SCL1, 0x77);

// Variables
short sDATABARRE[32];
float fREFERENCE = 0, fUPDATES = 0,fHAUTEUR=0;
int iREMEMBER;
bool bCOMPENSATE = false;

// Fonctions
void store(void);
float get(void);
void screen_control(void);

int main()
{
    printf("\r-----------------------------------Reset-----------------------------------");
   
    while (true) {
        led = !led;
        switch(storePRESSURE){
            case set:
                // Attente Altitude de depart
                while(!Bouton.read());
                store();
                break;
            case all:
                while(!Bouton.read()){
                    // Calcul Hauteur/Altitude
                    fHAUTEUR= (44330 *( 1 -pow((get()/fREFERENCE) , (1/5.255))));
                    printf("\n\r  %f", fHAUTEUR);
                    if (fHAUTEUR > 100){   // Borne Statique Maximum
                        printf("Over upper range");
                    }else if (fHAUTEUR > 100){ // Borne Statique Minimum
                        printf("Under lower range");
                    }
                    
                    oneS;
                    // Affiche nouvel valeur & graphique
                    screen_control();
                }
                //Verifie si Bouton Appuyer
                store();
               
                break;
        }

    }
}

//
// Enregistre le nouvau point de Depart & Remet le tableau a zero  
//
void store(void){
    if(Bouton.read()){
            led = !led;
            fREFERENCE = capteur.getPressure();
            for (int i = 0; i<=32;i++){
                    sDATABARRE[i] = 0;
                }
                drawFftGraph(sDATABARRE); 
                storePRESSURE = all;
    }
}

//
// Retourne la valeur de pression du capteur & change la DEL
//
float get(void){
    led = !led;
    return  capteur.getPressure();
 
}

//
// Affiche le graphique sur l'ecran
//
void screen_control(void){
        for (int i = 31; i>0;i--){
            if(i!=1){
                sDATABARRE[i]=sDATABARRE[i-1];
            }else{
                sDATABARRE[i]=  15 + (int)fHAUTEUR;
            }
        }
    
     drawFftGraph(sDATABARRE); 
}

