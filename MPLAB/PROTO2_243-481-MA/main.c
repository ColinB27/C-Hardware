
#include "mcc_generated_files/mcc.h"
//----------Enumerations---------------------------------------------------------

typedef enum {//machine a etat pour les raccourcis EUSART
    clear,
    curseur,
    home,
    brightness,
    on,
    off,
    blinkingCursorOn,
    blinkingCursorOff,
    Initialise
} Function;

typedef enum {
    Reception,
    Eteint,
    Armed,
    Perdu,
    Alarme,
    null
} Normal;
Normal GoBack = null; //copie du mode normale pour emmagasiner la position du retour apres la deconnexion
Normal EtatN = null; //mode normale

typedef enum {//etats du mode ecran
    ReceptionE,
    EteintE,
    Plat,
    rien
} Ecran;

Ecran goback = rien; //copie du mode ecran pour emmagasiner la position du retour apres la deconnexion
Ecran EtatE = rien; //mode ecran

typedef enum {//machines d'etat d'actions du programme
    Oui,
    Non,
    idle
} action;

action sec5 = idle; //machine d'etat pour le 5 seconde allumerde la led SET
action minute = idle; //machine d'etat pour le clignotement a la minute de la led SET
action plat = idle; //machine d'etat pour la detection du mode plat et la gestion des ses lumieres
action saving = Non; //machine d'etat pour l'emmagsinage de la position Armée
action perte = idle;
action connexion = idle;


//-----------Variables-----------------------------------------------------------

//--BOOL-----------------
bool LCD_Mode = false; //décision du mode
bool MINUTE = false; //machine d'etat pour le clignotement a la minute de la led SET

//--INT-----------------
int i, Hx, m;
unsigned int t1, t2, t3; //espace intermédiaire pour la trame
unsigned int PX, PY, PZ; //lecture en temps réelle de la trame
unsigned int SPX, SPY, SPZ; //données pour le mode armée
int c = 0;

uint8_t trame[19] = {}; //espace pour la trame


//------------Fonctions----------------------------------------------------------

//-TEST
void lamp_test(void); //test des lumiere et du buzzer

//--TRAITEMENT_LUMIERES
void traitementNormale(void); //mode normale
void traitementEcran(void); //mode ecran

//--FONCTIONNEMENT
void reception_affichage(void); //recoit et affiche sur l'ecran
void reception_(void); //recoit
uint8_t lecture(void); //lit le port de communication

//--INTERUPTIONS
void Blink_(void); //gere les lumieres
void Minute(void); //gere la lumiere du mode armé qui clignote a la minute
void Secondes_5(void); //gere la lumiere de mode armee qui s'ilumine 5sec a l'armement
void connect(void); //gere la connection du module (reception ou eteint))
//--EUSART/ECRAN
void affiche(void); //affichage du mode ecran
void EUSART_function(char hex); //racourci pour les fonction du eusart
void LCD_Function(Function ordre, uint8_t x, uint8_t z); //racourci pour les fonctions eusart

//-----------------------------------------------------------------------------------------------------------------------------

void main(void) {
    // Initialize the device
    SYSTEM_Initialize();
    TMR3_Initialize();
    TMR4_SetInterruptHandler(connect);
    TMR0_SetInterruptHandler(Minute);
    TMR1_SetInterruptHandler(Blink_);
    TMR3_SetInterruptHandler(Secondes_5);
    // Enable the Global Interrupts
    
    LCD_Mode = IO_RB3_GetValue() ? false : true; //cahoisir le mode dépendament du bouton SET
    lamp_test();
    
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    
    while(1){
        if (LCD_Mode) {//si le mode est écran
            EtatE = ReceptionE; //etat de l'ecran est eteint
            goback = ReceptionE; //etat de retour est reception
            while (1) {
                traitementEcran(); //mode ecran
                if(LCD_Mode == false)break;
            }
        } else {//sinon
            EtatN = Reception; //etat du mode normal est eteint
            GoBack = Reception; //mode de retour est reception
            while (1) {
                traitementNormale(); //mode ecran
            }
        }
    }
}

//-----------------------------------------------------------------------------------------------------------------------------

//------------------- TESTS / LUMIERES ----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------

void lamp_test(void) { //test de départ

    for (int j = 0; j < 2; j++) {//2 changements d'états   Vert
        IO_RB1_Toggle(); //LED Verte on/off
        __delay_ms(125); //délais
    }
    __delay_ms(250); //attendre 250ms
    for (int j = 0; j < 2; j++) {//2 changements d'états Rouge
        IO_RB2_Toggle(); //LED Rouge on/off
        __delay_ms(125); //délais
    }
    __delay_ms(250); //attendre 250ms
    int k = LCD_Mode ? 12 : 6; //clignotements différents dépendamment du mode de départ

    for (int j = 0; j < k; j++) {//clignotements
        IO_RB5_Toggle(); //buzzer on/off
        __delay_ms(125); //délais
    }
}

//-----------------------------------------------------------------------------------------------------------------------------

//------------------- INTERUPTIONS --------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------

void Blink_(void) {
    if (EtatN != null) {//si le mode est non null
        switch (EtatN) {
            case Reception://pour le mode reception
                IO_RB1_Toggle(); //clignote la lumiere verte
                IO_RB2_SetHigh(); //eteint la lumiere alarm
                break;
            case Eteint://pour le mode reception
                IO_RB2_Toggle(); //clignote la lumiere Alarm
                IO_RB1_SetHigh(); //eteint la lumiere
                break;
            case Armed://pour le mode reception
                if (MINUTE) {//si la minute a passer
                    IO_RB1_Toggle(); //fais clignoter la lumiere
                    m++; //incrémente m
                }
                if (m == 2) {//apres un clignotement
                    MINUTE = false; //attend une autre minute
                    minute = Oui; //attend une autre minute
                    m = 0; //reinitialise m
                }
                break;
            case Alarme://pour le mode reception
                IO_RB1_SetHigh(); //eteint la lumiere SET
                IO_RB2_SetLow(); //allume la lumiere ALarm
                break;
            case Perdu://pour le mode Perdu
                if(perte == Oui)IO_RB5_SetLow();
                IO_RB1_SetLow(); //allume les deux lumieres
                IO_RB2_SetLow();
                break;
            case null:
                break;
        }
    }
    if (EtatE != rien) {//lorsque le mode est non null
        switch (EtatE) {//pour le mode de l'EtatE
            case Plat://pour le mod Plat
                if (plat == Oui) {//la premiere fois
                    IO_RB1_SetHigh(); //lumiere Set eteinte
                    IO_RB2_SetLow(); //lumiere Alarm allumer
                    plat = Non; //plus la premiere fois
                }
                IO_RB1_Toggle(); //clignote les deux lumieres en alternance
                IO_RB2_Toggle();
                break;
            case ReceptionE://pour le mod Plat
                IO_RB1_Toggle(); //clignote la lumiere SET
                IO_RB2_SetHigh(); //eteint la lumiere alarm
                break;
            case EteintE://pour le mod Plat
                IO_RB1_SetHigh(); //eteint la lumiere set
                IO_RB2_Toggle(); //clignote la lumiere ALARM
                break;
            case rien:
                break;
        }

    }
}

//-----------------------------------------------------------------------------------------------------------------------------

void Minute(void) {//faire clignoter a la minute la lumiere SET
    if (minute == Oui) {
        MINUTE = true;
        minute = Non;
    }
}

//-----------------------------------------------------------------------------------------------------------------------------

void Secondes_5(void) {//eteindre la lumiere apres 5 sec lorsque le module est armée
    if (sec5 == Oui) {
        IO_RB1_SetHigh();
        sec5 = Non;
    }
}

//-----------------------------------------------------------------------------------------------------------------------------

//void connect(void) {
//   c++;  
//    if (c >= 2 && EtatN != null && EtatN != Alarme && EtatN != Eteint) {//quand la communication se perd et que l mode est non null et non alarme
//        if(EtatN == Armed ){
//            EtatN = Perdu;
//            perte = Oui;
//            TMR1_WriteTimer(0);
//            IO_RB5_SetHigh();
//        }else if(EtatN != Perdu){
//            EtatN = Eteint;
//        }
//        c = 0;
//    }else if(c == 0 && EtatN == Eteint){
//        EtatN = Reception;
//    }
//   
////    if(c >= 1 && EtatE != rien && EtatE != EteintE){
////        EtatE = EteintE;
////        c = 0;
////    }
////    if (lecture() ==1 && EtatN == null )EtatE = ReceptionE;
//   
//
//}


void connect(void) {
    c++;
    if (c >= 2 && EtatN != null && EtatN != Alarme && EtatN != Eteint && EtatN != Perdu) {//quand la communication se perd et que l mode est non null et non alarme
        if(EtatN == Armed ){
            EtatN = Perdu;
            perte = Oui;
            TMR1_WriteTimer(0);
            IO_RB5_SetHigh();
        }else{
            EtatN = Eteint;
        }   
        
        c = 0;
    } else if (EtatN == Eteint && connexion == 1) {
        EtatN = Reception; //retourne au mode précédant
    }else if (EtatN == Perdu && connexion == 1){
        EtatN = Armed;
        IO_RB1_SetHigh();
        IO_RB2_SetHigh();
    } else if (EtatN == Eteint && connexion == 0) c = 0; //réinitialise c si la fonction est appelé mais ne fais aucun changement

    if (c >= 2 && EtatE != rien && EtatE != EteintE) {
        EtatE = EteintE; //mode eteint
        c = 0; //réinitialise c
    } else if (EtatE == EteintE && connexion == 1) {
        EtatE = ReceptionE; //retourne au mode précédant
    } else if (connexion == Non)c = 0; //réinitialise c si la fonction est appelé mais ne fais aucun changement
}


//-----------------------------------------------------------------------------------------------------------------------------

//------------------ TRAITEMENT_LUMIERES --------------------------------------------------------------------------------------

//----------------------------------------------------------- ------------------------------------------------------------------

void traitementNormale(void) {
    switch (EtatN) {
        case Reception:
            connexion = Oui;
            reception_();
            if (saving == Non && !IO_RB3_GetValue())
                saving = Oui;
            if (saving == idle) {
                EtatN = Armed;
                saving = Non;
                sec5 = Oui;
                minute = Oui;
                TMR3_WriteTimer(0);
                IO_RB1_SetLow();
            }
            break;
        case Eteint://cas vide pour enlever les warnings
            if (lecture() == 1){
                connexion = Oui;
            }else{
                connexion = Non;
            }
            break;
        case Armed://lorsque le module est armée
            
            if (!IO_RB4_GetValue()) {//si on appuie si le bouton réinitialiser l'armement pour un nouvel armement
                SPZ = 0;
                SPX = 0;
                SPY = 0;
                EtatN = Reception; //retourner au mode reception
            }
                reception_affichage(); //écoute la réception de trame
                //di le module bouge
                if (!((PZ < (SPZ + 0x1f))&&(PZ > (SPZ - 0x1f))&&(PX < (SPX + 0x1f))&&(PX > (SPX - 0x1f))&&(PY < (SPY + 0x1f))&&(PY > (SPY - 0x1f)))) {
                    EtatN = Alarme; //partir l'alarme
                   IO_RB5_SetHigh(); //alumer le buzzer
                }
            
            
            break;
        case Perdu://cas vide pour enlever les warnings
            if (lecture() == 1){
                connexion = Oui;
            }else{
                connexion = Non;
            }
            break;
        case Alarme://cas vide pour enlever les warnings
            if(!IO_RB4_GetValue()){
                EtatN = Reception;
                IO_RB5_SetLow();
            }
            break;
        case null://cas vide pour enlever les warnings
            break;
    }

};

//-----------------------------------------------------------------------------------------------------------------------------

void traitementEcran(void) {
    if(!IO_RB4_GetValue()){
        EtatE = rien;
        EtatN = Reception;
        LCD_Mode = false;
        LCD_Function(clear,0,0);
        LCD_Function(off,0,0);
    }
    switch (EtatE) { //gestion du mode ecran
        case ReceptionE: //gestion de la reception
            plat = idle;
            reception_affichage();
            if ((PZ == 0x03ff) && (PX >= 0x0200) && (PX <= 0x0210) && (PY >= 0x0200) && (PY <= 0x0210)) {
                EtatE = Plat;
            }
            break;
        case EteintE://cas vide pour enlever les warnings
            if (lecture() == 1){
                connexion = Oui;
            }else{
                connexion = Non;
            }
            break;
        case Plat: //gestion de la reception a plat
            if (plat == idle) {
                plat = Oui;
            }           
            reception_affichage();
            if (!((PZ == 0x03ff) && (PX >= 0x0200) && (PX <= 0x0210) && (PY >= 0x0200) && (PY <= 0x0210))) {
                EtatE = ReceptionE;
            }
            break;
        case rien://cas vide pour enlever les warnings
            break;
    }

}

//-----------------------------------------------------------------------------------------------------------------------------

//------------------ FONCTIONNEMENT -------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------

void reception_affichage(void) {
    RCSTA1bits.CREN = 1;
    while (lecture() == 0); //lire les capteurs
    RCSTA1bits.CREN = 0;
    affiche();
}

//-----------------------------------------------------------------------------------------------------------------------------

void reception_(void) {
    RCSTA1bits.CREN = 1;
    while (lecture() == 0); //lire les capteurs
    RCSTA1bits.CREN = 0;
}

//-----------------------------------------------------------------------------------------------------------------------------

uint8_t lecture(void) {

    if (PIR1bits.RC1IF == 1) //si le port série est prêt
        Hx = EUSART1_Read(); //RCREG1; on prend la valeur de la donner recue
    if (Hx == 0x7E) { //si la donnée recue est 0xFE (debut de trame)
        for (i = 0; i < 19; i++) { //Enregistrement de la donnee dans la Table [i](19 position)
            trame[i] = EUSART1_Read();
        }
        t1 = trame[12] << 8; //X partie 1
        t2 = trame[14] << 8; //Y partie 1
        t3 = trame[16] << 8; //Z partie 1

        if (saving == Non) { // si le programme n'est pas en train d'emmagasiner la position Armée
            PX = t1 + trame[13]; //enregistre la donné X
            PY = t2 + trame[15]; //enregistre la donné Y     CECI A ÉTÉ UTILISÉ MAJORITAIREMENT POUR DES TESTS
            PZ = t3 + trame[17]; //enregistre la donné Z     AINSI QUE DES RACCOURCIS
        } else if (saving == Oui) {// si le programme est en train d'emmagasiner la position ARMÉE
            SPX = t1 + trame[13]; //enregistre la donné X dans l'espace pour la position ARMÉE
            SPY = t2 + trame[15]; //enregistre la donné Y dans l'espace pour la position ARMÉE
            SPZ = t3 + trame[17]; //enregistre la donné Z dans l'espace pour la position ARMÉE
            saving = idle;
        }
        c = 0; //réinitialisation de lavariable de déconnexion
        Hx = 0; //réinitialisation de la recherche de trame
        
        return 1;
    } else {
        
        return 0;
    }

}

//-----------------------------------------------------------------------------------------------------------------------------

//------------------ EUSART/ECRAN --------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------------

void EUSART_function(char hex) {
    EUSART1_Write(0xFE); //envoie une fonction EUSART
    EUSART1_Write(hex); //Fonction demander
}

//-----------------------------------------------------------------------------------------------------------------------------

void affiche(void) {
    LCD_Function(Initialise, 0, 0); // home
    printf("      POSITIONS     ");

    LCD_Function(curseur, 1, 0); //ligne 2
    printf("X = %02X %02X", trame[12], trame[13]); //affiche les valeur de l'axe X

    LCD_Function(curseur, 2, 0); //ligne 3
    printf("Y = %02X %02X", trame[14], trame[15]); //affiche les valeur de l'axe Y
    if (EtatE == Plat) printf("  _-Plat-_");
    

    LCD_Function(curseur, 3, 0); //ligne 4
    printf("Z = %02X %02X", trame[16], trame[17]); //affiche les valeur de l'axe Z
}

//-----------------------------------------------------------------------------------------------------------------------------

void LCD_Function(Function ordre, uint8_t x, uint8_t z) {
    switch (ordre) {
        case clear://fonction clear
            EUSART_function(0x51);
            break;

        case brightness://fonction brightess
            EUSART_function(0x53);
            EUSART1_Write(x);
            break;

        case on: //fonction allumer le LCD
            EUSART_function(0x41);
            break;

        case off://fonction eteindre le LCD
            EUSART_function(0x42);
            break;

        case curseur:// fonction choisir la position du curseur
            EUSART_function(0x45);
            ;
            switch (x) {//choisir la ligne
                case 0:
                    EUSART1_Write(0x00 + z); //ligne 1 position z
                    break;
                case 1:
                    EUSART1_Write(0x40 + z); //ligne 2 position z
                    break;
                case 2:
                    EUSART1_Write(0x14 + z); //ligne 3 position z
                    break;
                case 3:
                    EUSART1_Write(0x54 + z); //ligne 4 position z
                    break;
            }
            break;
        case home://fonction home
            EUSART_function(0x46);
            break;
        case blinkingCursorOn://fonction activer le curseur clignotant
            EUSART_function(0x4B);
            break;
        case blinkingCursorOff://fonction desactiver le curseur clignotant
            EUSART_function(0x4C);
            break;
        case Initialise: // fonctions qui initalise l'utilisation du LCD
            EUSART_function(0x41);
            EUSART_function(0x51);
            EUSART_function(0x46);
            break;
    }
}
