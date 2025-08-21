
#include "mbed.h"
#include "mbed.h"
#include <cstdint>
#include <cstdio>

#include <CMPS12.h>

// define
#define wait(x) ThisThread::sleep_for(x);
#define oneS wait(1000ms); // attendre 1 seconde#
#define moteur p26

#define POSITION_VOULU 45 // position recherché


int getCMPS12(void);

void redneck_compensate(int position, int consigne);
void compensate_proportionnel(int position, int consigne);

int iBEARING = 0, iPOSITION = 0;

float Kp = 5; // gain proportionnel
float K = 1500; // décalage



PwmOut m1(moteur); // objet PWM pour gestion du moteur
CMPS12 Boussole(I2C_SDA1, I2C_SCL1, 0xC0); // définition de l'oibjet pour la gestion de la boussole


int main()
{
    printf("\r-----------------------------------Reset-----------------------------------");  

    m1.period_us(20000); // période du PWM pour la gestion du moteur à 20ms
    
    while(1){
        compensate_proportionnel(getCMPS12(), POSITION_VOULU); // boucle de rétroaction proportionnelle pour ajustement de la position du moteur
    }

}

//---------------- Fonction getCMPS12
//
// Fais des requetes au capteur les valeurs:
//              --int iBEARING : lecture de l'angle au capteur
//              --int iPOSITION : angle traduit du capteur (en degrées 0 à 360)
//
int getCMPS12(void){
    iBEARING = Boussole.readBearing();
    iPOSITION = (iBEARING / 10);

    return iPOSITION;
}

//---------------- Fonction compensate_proportionnel
//
// Boucle de rétroaction proportionnelle pour ajustement de la position du moteur
//              --int position : position (en degrées 0 à 360)
//              --int consigne : position voulu (en degrées 0 à 360))
//Calcul la vitesse:
//              --int sortie : temps haut relatif aux vitesses nécessaire pour l'ajustement du servo  Parralax a rotation continuelle
//Écriture au moteur m1
//         
void compensate_proportionnel(int position, int consigne){
    int erreur = position - consigne;
    int sortie = (Kp *erreur)+ K;

    m1.pulsewidth_us(sortie);

}