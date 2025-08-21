
#include <chrono>
#include <cstdint>
#include <mbed.h>

//#define stop(x) ThisThread::sleep_for(x)
#define BLINKING_RATE     500ms
#define wait(x) ThisThread::sleep_for(x)

#define M1_STEP p29
#define M2_STEP p23
#define M1_DIR p30
#define EN p28

#define Ms1 p27
#define Ms2 p26
#define Ms3 p25

#define POT p15

const int VITESSE_ANGULAIRE_MAX_DEG_SEC = 720;

// =========================================================================
// -- Fonctions
void manualspin(void);
float rotate(int degrees_persecond);
void stepper1(void);
float tension_degree(uint16_t V_in);
// =========================================================================


// =========================================================================
// -- Objets
DigitalOut m1_step(M1_STEP);
DigitalOut m2_step(M2_STEP);
DigitalOut m1_dir(M1_DIR);
DigitalOut ENABLED(EN);
DigitalOut led(LED1);

AnalogIn speed(POT);

Timeout time1;
// =========================================================================


// =========================================================================
// -- Variables
int steps = 201;
int initialTime = 1000;
float degree_tension;
// =========================================================================


//================================================ THIS IS MAIN
int main() {
    printf("------------------Reset------------------");
    m1_step = 0; // position initiale
    degree_tension = tension_degree(speed.read_u16());   // Convertie la tension lue en degré
    time1.attach(&stepper1, rotate(degree_tension));     // sert a attacher l'interruption sur la fonction stepper1

    while(1) {
        ENABLED = 0; 
        printf("\n\rDegre par seconde: %i",int(tension_degree(speed.read_u16())));     
        degree_tension = tension_degree(speed.read_u16());
        ThisThread::sleep_for(250ms);
    }
}

// =========================================================================
// -- TP 2
// =========================================================================

float rotate(int degrees_persecond){
    return(1/((degrees_persecond/1.8)*2)); // transforme les degrees par secondes en temps haut et bas pour la bonne fréquence de rotation
}


void stepper1(void){
    m1_step = !m1_step;
    time1.attach(&stepper1, rotate(degree_tension)); 
}

float tension_degree(uint16_t V_in){
    return (V_in/91);
}


// =========================================================================
// -- TP 1
// =========================================================================
void manualspin(void){    
    ENABLED = 0;

    for(int i = 0; i< steps; i++){
        m1_step = 1;
        wait_us(initialTime);
        m1_step = 0;
        wait_us(initialTime);
    }
    if(initialTime>200){
        initialTime-=10;
    }else if(initialTime>150){
        initialTime-=5;
    }else if(initialTime>90){
        initialTime-=1;
    }

    ENABLED = 1;


}

