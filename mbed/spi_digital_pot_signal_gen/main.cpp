/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <cstdint>
#include <cstdio>



#define END_PROGRAM while(1);
#define wait(x) ThisThread::sleep_for(x)

#define MAX 60 //quantité de données transmisent

bool SPI_flag = false; //activation / déactivation de la transmission SPI

uint8_t sinus[MAX] ={ // valeurs constituant une fonction sinusoidale
    254, 254, 252, 249, 244, 238, 231, 222, 213, 202,
    191, 179, 167, 154, 141, 127, 114, 101, 88, 76,
    64, 53, 42, 33, 24, 17, 11, 6, 3, 1,
    0, 1, 3, 6, 11, 17, 24, 33, 42, 53,
    64, 76, 88, 101, 114, 128, 141, 154, 167, 179,
    191, 202, 213, 222, 231, 238, 244, 249, 252, 254

};
uint8_t car[MAX] = { // valeurs constituant une fonction carré
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};
uint8_t tri[MAX] = { // valeurs constituant une fonction triangulaire
    9, 17, 26, 34, 43, 51, 60, 68, 77, 85,
    94, 102, 111, 119, 128, 136, 145, 153, 162, 170,
    179, 187, 196, 204, 213, 221, 230, 238, 247, 255,
    247, 238, 230, 221, 213, 204, 196, 187, 179, 170,
    162, 153, 145, 136, 128, 119, 111, 102, 94, 86,
    77, 68, 60, 51, 43, 34, 26, 17, 9, 0
};

uint8_t *ptr = sinus; // pointer vers la fonction transmise

uint8_t lettre; // entrer du clavier
uint8_t valeurPot = 0; // valeur intermédiaire pour déterminer la fréquence
uint8_t type = 1; //valeur pour identifier le type de fonction

Ticker TMR_Interrupt; // créaction d'un object horloge
BufferedSerial pc(USBTX, USBRX, 9600); // initialisaion de l'objet PC (USART)
SPI pot(MOSI, MISO, SCLK); // initialisaion de l'objet pot (SPI)
DigitalOut EXT_CS(CS); // initialisation de l'objet EXT_CS (chip select)

//déclaration de nos fonctions
void readClient(void); 
void out_dig(uint8_t x);
void myTimer1_ISR(void);
void dataTreatment(void);
void switch_frequence(void);
void print_type(void);

int main()
{

    while (true) {
        readClient();
        if (SPI_flag){ //si la communication SPI est active
            static uint8_t i;
            out_dig(ptr[i]); // envoie de la donnée pointée
            i++; //incrémentation

            if (i == MAX) { //si l'incrémentation est au maximum (60)
                i = 0; //remettre incrémentation a 0
            }
            SPI_flag = false; //désactivation de la communication SPI
        }
    }
}

// lecture du clavier
void readClient(void){ 
    if (pc.readable()){ //si l'ordinateur recoi du clavier
            pc.read(&lettre, 1); // écrit le caractere recu dans la variable lettre
            //printf("*%c*", lettre); (code test)
            dataTreatment(); // traite la donnée recu
        }
}

// communication SPI vers le Potentiomètre MCP42010l 
void out_dig(uint8_t x) {
    EXT_CS = 0; // selection de la communication au potentiometre
    pot.write(0x11); // ecriture, pot. 0
    pot.write(x); //écriture sur le port SPI
    EXT_CS = 1; // désactivation de la communication au potentiometre
}

//traitement des entrées du clavier
void dataTreatment(void) { 
    switch (lettre) { // dépendament de lettre
        case '+': // si '+'
            if (valeurPot >= 1){ // si la fréquence n'est pas au plus petit
                valeurPot--; //décrémente la fréquence
                switch_frequence(); //modifie la fréquence
            }
            break; 
        case '-': // si '-'
            if (valeurPot <= 3){ // si la fréquence n'est pas au plus grand
                valeurPot++; //incrémente la fréquence
                switch_frequence(); //modifie la fréquence
            }
            break; 
        case 't': // si 't'
            ptr = tri; // pointer la fonction triangulaire  
            type = 3; // type = 3 = triangulaire
            break; 
        case 'c': // si 'c'
            ptr = car; // pointer la fonction carré  
            type = 2; // type = 2 = carré
            break; 
        case 's': // si 's'
            ptr = sinus; // pointer la fonction sinusoidale  
            type = 1; // type = 1 = sinusoidale
            break;    
    }
    print_type();
    
}

//changement de la fréquence
void switch_frequence(void){
    switch (valeurPot) { // selon la valeur du pot
        case 0: // si 0
            //fréquence de 100hz
            TMR_Interrupt.attach(&myTimer1_ISR, 166us);
            break;
        case 1: // si 1
            //fréquence de 80hz
            TMR_Interrupt.attach(&myTimer1_ISR, 208us);
            break;
        case 2: // si 2
            //fréquence de 60hz
            TMR_Interrupt.attach(&myTimer1_ISR, 277us);
            break;
        case 3: // si 3
            //fréquence de 40hz
            TMR_Interrupt.attach(&myTimer1_ISR, 416us);
            break;
        case 4: // si 4
            //fréquence de 20hz
            TMR_Interrupt.attach(&myTimer1_ISR, 833us);
            break;
    }
}

//écriture du type de la fonction
void print_type(void){
    switch(type){ //imprime le type de fonction afficher sur la console
                    case 1: // si 1
                        printf("\n\r Sinus"); //imprime te type sinusoidale
                        break;
                    case 2: // si 2
                        printf("\n\r Carre"); //imprime te type carré
                        break;
                    case 3: // si 3
                        printf("\n\r Triangulaire"); //imprime te type triangulaire
                        break;
                }
                printf(" %dHz", 100 - valeurPot*20); //imprime la fréquence sur la console
}

// activation de la communication SPI
void myTimer1_ISR(void) {
   SPI_flag = true;
}