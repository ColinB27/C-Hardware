/**
 * @author Colin & Louis
 *
 * HCSR04 digital ultrasonic sensor module.
 *
 * Datasheet:
 *  https://pdf1.alldatasheet.com/datasheet-pdf/view/1132203/ETC2/HC-SR04.html
 */

#include "mbed.h"
#include "HCSR04.h"

//define pour délais
#define wait(x)     ThisThread::sleep_for(x);
#define halfS        500ms
#define quarterS     250ms

//définition des pines
#define trigger      p12
#define echo         p13

//valeur pour calcul de la vitesse
#define TBS          0.5 //Time Between Samples

// -------------------------- Constantes

const uint8_t UI_MIN = 5 ,
              UI_MAX = 250 ,
              UI_VAR = 30 ;

// -------------------------- Variables

typedef enum{
    Plafonnement,
    Extrapolation,
    Ignorance
}detectionType;

detectionType eDETECTION = Ignorance;

float fDISTANCE[3] = {0,0,0};
float fVITESSE,
      fVARIATION;

// -------------------------- Fonctions 

float _variation_(float flast_value, float fnew_value); // Retourne en cm
float _vitesse_(float flast_value, float fnew_value); // Retourne en cm/ms

bool _detectionErreur_(detectionType erreurs);

void _changementErreur_(detectionType erreurs);
void _correctionErreur_(detectionType erreurs);
void _gestionErreur_(DigitalIn button, detectionType erreurs);

// Liaisons broches externes
    DigitalOut led(LED1);
    HCSR04 capEyes(trigger, echo);
    DigitalIn boutton(p20);

// Main
int main()
{
    
    capEyes.startSensor(10); // activer le capteur

    printf("\n\r--------------------------------RESET-------------------------\n\r");

    while (true) {
        wait(halfS)
        
        // Partie 1 : Lire Capteur

        for (int iPos = 2; iPos>0;iPos--){ // sauvegarder les 2 dernieres valeurs captées
            if(iPos!=0){
                fDISTANCE[iPos]=fDISTANCE[iPos-1];
            }
        }
        fVARIATION = _variation_(fDISTANCE[2],fDISTANCE[1]);

        fDISTANCE[0] = capEyes.getDistance(); // sauvegarder la valeur au présent
        printf("    Distance is :    %0.2f cm \n\r" , fDISTANCE[0]);
        //printf("    Variation = %f\n\r", fVARIATION );

        // Partie 2 : Detection d'erreur
        _gestionErreur_(boutton, eDETECTION);

        // Partie 3 : Detection Vitesse
        printf("    Vitesse = %f cm/s\n\r", _vitesse_(fDISTANCE[1],fDISTANCE[0]) ); //imprime la vitesse calculer avec la fonction du calcul de vitesse
        printf("\n\r    -   -   - New Data  -   -   - \n\r");
        
    }
}
//===================================================================================
// calcul une variation entre deux valeurs
float _variation_(float flast_value, float fnew_value){
    return (fnew_value-flast_value);
}

//===================================================================================
// calcul la vitesse
float _vitesse_(float flast_value, float fnew_value){
        return (_variation_(flast_value,fnew_value)/TBS);  
}

//===================================================================================
// détecte les erreurs selon le mode de correction et écrit des al;ertes lorsqu'une erreure est détecter
bool _detectionErreur_(detectionType erreurs){
    switch(erreurs){
        case Plafonnement :
            if ( fDISTANCE[0] < UI_MIN){
                printf("\n\t-\t Plafond Minimal Atteint\n\r");
                return true;
            }
            else if ( fDISTANCE[0] > UI_MAX){
                printf("\n\t-\t Plafond Maximal Atteint\n\r");
                return true;
            }
            break;

        case Extrapolation :
            if ( abs(_variation_(fDISTANCE[1],fDISTANCE[0])) > UI_VAR){
                printf("\n\t-\t Valeur Varie Excessivement\n\r");
                return true;
            }
            break;

        case Ignorance :
            if ( ( fDISTANCE[0] < UI_MIN) || ( fDISTANCE[0] > UI_MAX) || ( abs(_variation_(fDISTANCE[1],fDISTANCE[0])) > UI_VAR ) ){
                printf("\n\t-\t Didn't See That Comming\n\r");
                return true;
            }
            break; 
    }
    return false;
}

//===================================================================================
// corrige les erreurs selon le mode de correction d'erreur
void _correctionErreur_(detectionType erreurs){
    switch(erreurs){
        case Plafonnement :
            if ( fDISTANCE[0] < UI_MIN){
                fDISTANCE[0] = 5.0;
            }
            else if ( fDISTANCE[0] > UI_MAX){
                fDISTANCE[0] = 250.0;
            }
            break;

        case Extrapolation :
            fDISTANCE[0] = fDISTANCE[1] + fVARIATION;
            break;

        case Ignorance :
                fDISTANCE[0] = fDISTANCE[1];
            break; 
    }
    printf("    Corrected Distance is :    %0.2f \n\r" , fDISTANCE[0]);
}

//===================================================================================
// logique de correctionn d'erreur lorsqu'une erreure et logique de changement de mode de correction
// avec un bouton
void _gestionErreur_(DigitalIn button, detectionType erreurs){
    if (_detectionErreur_(erreurs)== true){
        _correctionErreur_(erreurs);
    }
    
    if (button.read() == 1){
        while(button.read()==1);

        if (erreurs == Plafonnement){eDETECTION = Extrapolation;
        }else if (erreurs == Extrapolation){eDETECTION = Ignorance;
        }else if (erreurs == Ignorance){eDETECTION = Plafonnement;
        }
        printf("\n\r %d\n\r" , eDETECTION);
        led = !led;
        }
}

//===================================================================================
