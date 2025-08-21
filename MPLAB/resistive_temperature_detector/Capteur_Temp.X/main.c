// Colin Boulé et Louis Tran
#include "mcc_generated_files/mcc.h"

#define TABLE_SIZE 151

typedef struct {
    float resistance;
    float temperature;
} rtd_lookup_table;

rtd_lookup_table table[TABLE_SIZE] = {
{80.31 ,-50},
{80.70 ,-49},
{81.10 ,-48},
{81.50 ,-47},
{81.89 ,-46},
{82.29 ,-45},
{82.69 ,-44},
{83.08 ,-43},
{83.48 ,-42},
{83.87 ,-41},
{84.27 ,-40},
{84.67 ,-39},
{85.06 ,-38},
{85.46 ,-37},
{85.85 ,-36},
{86.25 ,-35},
{86.64 ,-34},
{87.04 ,-33},
{87.43 ,-32},
{87.83 ,-31},
{88.22 ,-30},
{88.62 ,-29},
{89.01 ,-28},
{89.40 ,-27},
{89.80 ,-26},
{90.19 ,-25},
{90.59 ,-24},
{90.98 ,-23},
{91.37 ,-22},
{91.77 ,-21},
{92.16 ,-20},
{92.55 ,-19},
{92.95 ,-18},
{93.34 ,-17},
{93.73 ,-16},
{94.12 ,-15},
{94.52 ,-14},
{94.91 ,-13},
{95.30 ,-12},
{95.69 ,-11},
{96.09 ,-10},
{96.48 ,-9 },
{96.87 ,-8 },
{97.26 ,-7 },
{97.65 ,-6 },
{98.04 ,-5 },
{98.44 ,-4 },
{98.83 ,-3 },
{99.22 ,-2 },
{99.61 ,-1 },
{100.00 ,0  },
{100.39 ,1  },
{100.78 ,2  },
{101.17 ,3  },
{101.56 ,4  },
{101.95 ,5  },
{102.34 ,6  },
{102.73 ,7  },
{103.12 ,8  },
{103.51 ,9  },
{103.90 ,10 },
{104.29 ,11 },
{104.68 ,12 },
{105.07 ,13 },
{105.46 ,14 },
{105.85 ,15 },
{106.24 ,16 },
{106.63 ,17 },
{107.02 ,18 },
{107.40 ,19 },
{107.79 ,20 },
{108.18 ,21 },
{108.57 ,22 },
{108.96 ,23 },
{109.35 ,24 },
{109.73 ,25 },
{110.12 ,26 },
{110.51 ,27 },
{110.90 ,28 },
{111.29 ,29 },
{111.67 ,30 },
{112.06 ,31 },
{112.45 ,32 },
{112.83 ,33 },
{113.22 ,34 },
{113.61 ,35 },
{114.00 ,36 },
{114.38 ,37 },
{114.77 ,38 },
{115.15 ,39 },
{115.54 ,40 },
{115.93 ,41 },
{116.31 ,42 },
{116.70 ,43 },
{117.08 ,44 },
{117.47 ,45 },
{117.86 ,46 },
{118.24 ,47 },
{118.63 ,48 },
{119.01 ,49 },
{119.40 ,50 },
{119.78 ,51 },
{120.17 ,52 },
{120.55 ,53 },
{120.94 ,54 },
{121.32 ,55 },
{121.71 ,56 },
{122.09 ,57 },
{122.47 ,58 },
{122.86 ,59 },
{123.24 ,60 },
{123.63 ,61 },
{124.01 ,62 },
{124.39 ,63 },
{124.78 ,64 },
{125.16 ,65 },
{125.54 ,66 },
{125.93 ,67 },
{126.31 ,68 },
{126.69 ,69 },
{127.08 ,70 },
{127.46 ,71 },
{127.84 ,72 },
{128.22 ,73 },
{128.61 ,74 },
{128.99 ,75 },
{129.37 ,76 },
{129.75 ,77 },
{130.13 ,78 },
{130.52 ,79 },
{130.90 ,80 },
{131.28 ,81 },
{131.66 ,82 },
{132.04 ,83 },
{132.42 ,84 },
{132.80 ,85 },
{133.18 ,86 },
{133.57 ,87 },
{133.95 ,88 },
{134.33 ,89 },
{134.71 ,90 },
{135.09 ,91 },
{135.47 ,92 },
{135.85 ,93 },
{136.23 ,94 },
{136.61 ,95 },
{136.99 ,96 },
{137.37 ,97 },
{137.75 ,98 },
{138.13 ,99 },
{138.51 ,100}, 
};
uint16_t adc;

double volt, res, m, tensionADC, temp;

double rtd_lookup(double x, int size);
void read_ADC(void);

void main(void) {
    // Initialize the device
    SYSTEM_Initialize();

    printf("\n\r ---- RESET ---- \n\r");

    while (1) {

        read_ADC();

    }
}

double rtd_lookup(double x, int size) {
    double pente;
    int i = 0;
    while ((i < size)&& (x > table[i].resistance)) {
        i++;
    }
    if (i == size){
        printf("\n\r Iimite de température supérieur atteinte");//borne statique supérieur
        return (table[i - 1].temperature);
    }
        
    if (i == 0){
        return (table[i].temperature);
        printf("\n\r Limite de température inférieur atteinte");//borne statique inférieur
    }

    pente = (table[i].temperature - table[i - 1].temperature) / (table[i].resistance - table[i - 1].resistance);
    
    return ( pente * (x - table[i].resistance)) + table[i].temperature;

}

void read_ADC(void) {

    adc = ADC_GetConversion(channel_AN0);// Recois donnee ADC
    __delay_ms(250);
    tensionADC = (((double) (adc)*2) / 4095); // Calcul ADC -> Tension
    volt = (tensionADC + 3) / 33.78;// Calcul Tension AMP04 -> Tension Resistance
    res = volt / 0.001;// Calcul Tension Resistance -> Resistance

    temp = rtd_lookup((double) res, TABLE_SIZE);

    // printf("\n\r ---- ADC         = %u ---- ", ADC_GetConversion(channel_AN0));
    //printf("\n\r ---- Tension adc = %lf ---- ", tensionADC+2.5);
    printf("\n\r ---- Tension res = %f ---- ", volt);
     printf("\n\r ---- Resistance  = %f ---- ", res);
    printf("\n\r ---- Temperature = %f ---- \n\r", temp);

    __delay_ms(600);
};
/*
 
 
 
 2.34
 
 */
