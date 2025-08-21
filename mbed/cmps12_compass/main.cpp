
#include "mbed.h"
#include <cstdint>
#include <cstdio>

#include <CMPS12.h>
#include <KS0108.h>

// define
#define wait(x) ThisThread::sleep_for(x);
#define oneS wait(1000ms);
#define halfS wait(500ms);
#define quartS wait(250ms);
#define tenthS wait(100ms);
#define twentyS wait(50ms);

// Variables
int iBEARING = 0, iANGLE = 0 , iDEPLACEMENT = 0, iCALIBRATION = 0x0,
    iPOStopX , iPOStopY , iPOSrightX, iPOSrightY;

// Entrees & Sorties
CMPS12 Boussole(I2C_SDA1, I2C_SCL1, 0xC0); 
BufferedSerial pc(USBTX, USBRX, 9600);
KS0108 SCREEN(p20 ,p18, p19, p17, p21, p22, p30, p29, p28, p27, p26, p25, p24, p23);

// Fonctions
void getCMPS12(void);
void printScreen(void);
void debugg(void);

int main()
{

    printf("\r-----------------------------------Reset-----------------------------------") ;

    //calibration
    do{
        printf("\n\r\t-   Calibrating") ;
        iCALIBRATION = Boussole.readCalibration() ;
        halfS
    }while((iCALIBRATION & 0xFF)!=0xFF) ;

    printf("\r--------------------------------Calibrated---------------------------------") ;

    getCMPS12() ;
    
    printScreen() ;
    
    iDEPLACEMENT = iANGLE ;

    while(1){
        
        while(iDEPLACEMENT == iANGLE){ //rafraichis l'affichage lorsque l'angle change (empeche les rafraichissements inutiles)
            getCMPS12() ;
        }
        
        quartS
        
        debugg() ;

        printScreen() ;
    }
    

}
//---------------- Fonction getCMPS12
//
// Fais des requetes au capteur les valeurs:
//              --Bearing
//              --Calibration
//Calcul l'angle:
//              --Angle
void getCMPS12(void){
    iBEARING = Boussole.readBearing() ;
    iANGLE = (iBEARING / 10) ;
    iCALIBRATION = Boussole.readCalibration() ;
}

//---------------- Fonction debugg
//
// Affiche la boussole pointant le nord la direction du capteur:
void printScreen(void){
    SCREEN.ClearScreen() ;
    SCREEN.EmptyCircle(60, 30, 20,BLACK) ;
    SCREEN.DegreeLine(60,30,-(iANGLE),0, 15, BLACK) ;
    SCREEN.EmptyCircle(60, 50, 2,BLACK) ; // indicateur de la direction du capteur
    iDEPLACEMENT = iANGLE ;
}

//---------------- Fonction debugg
//
// Imprime les valeurs:
//              --Bearing
//              --Angle
//              --Calibration
void debugg(void){ 
    printf("\n\n\r\t-   Bearing is = %04i" , iBEARING) ;
                
    if ( !((iANGLE>=0) && (iANGLE < 360)) ){ // borne statique
        printf("\n\r\t-   Angle is IMPOSSIBLE") ;
    }else {
        printf("\n\r\t-   Angle is = %03i" , iANGLE) ;
    }
    printf("\n\n\r\t-   Calibration is = %02i" , iCALIBRATION) ;
}

