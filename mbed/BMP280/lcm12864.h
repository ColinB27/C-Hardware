#include "mbed.h"
// Global includes
//#include <htc.h>

// This define states the delay when strobing the EN
// line in uS
#define ENSTROBEDELAY 10

// gLCD definitions
#define GLCD_READ         0b11111111
#define GLCD_WRITE        0b00000000
#define GLCD_CMD          0
#define GLCD_DATA         1

#define READ						1
#define WRITE						0

#define SWITCH0					0
//#define SWITCH1					1

// Hardware mapping definitions
// gLCD character display hardware


/*#define   GLCD_DB0         RD0
#define GLCD_DB1         RD1
#define GLCD_DB2         RD2
#define GLCD_DB3         RD3
#define   GLCD_DB4         RD4
#define GLCD_DB5         RD5
#define GLCD_DB6         RD6
#define GLCD_DB7         RD7
*/
BusInOut GLCD_DATABUS(p30,p29,p28,p27,p26,p25,p24,p23);
//#define GLCD_DATABUS      PORTD
//#define GLCD_DATADIRECTION   TRISD

DigitalOut GLCD_RS(p18);
DigitalOut GLCD_RW(p19);
DigitalOut GLCD_EN(p17);
//#define GLCD_RS            RB0
//#define GLCD_RW            RB1
//#define GLCD_EN            RB2

DigitalOut GLCD_CS1(p21);
DigitalOut GLCD_CS2(p22);
//#define GLCD_CS1         RB3
//#define GLCD_CS2         RB4

// Function prototypes
void drawFftGraph(short inputData[]);
void drawbarGraph(short inputData);  //nouvelle fonction pour faire une seule grosse barre avec une valeur
void gLcdInit(void);
void gLcdClear(void);


// Write a byte directly to the screen hardware (quick version)
void fLcdWrite(unsigned char cmdType, unsigned char bank, unsigned char byte)
{
   // Wait for the busy flag to clear
   //GLCD_DATADIRECTION = GLCD_READ;
	 GLCD_DATABUS.input();
   GLCD_RW.write(READ);
   GLCD_RS.write(GLCD_CMD);
   
   do
   {
      // Strobe the EN line
      GLCD_EN.write(1);
      wait_us(ENSTROBEDELAY);
      GLCD_EN.write(0);
  // } while (GLCD_DB7 == 1);
			} while (GLCD_DATABUS[7] == 1);

   // Select the command type
   GLCD_RS.write(cmdType);

   // Select the screen bank
   if (bank == 0)
   {
      GLCD_CS1.write(0); //je l'ai invers� pour notre LCD
      GLCD_CS2.write(1); //
   }
   else
   {
      GLCD_CS1.write(1);
      GLCD_CS2.write(0);
   }

   // Place the byte on the databus
   //GLCD_DATADIRECTION = GLCD_WRITE;
	 GLCD_DATABUS.output();
   GLCD_RW.write(WRITE);
   GLCD_DATABUS = byte;

   // Strobe the EN line
   GLCD_EN.write(1);
   wait_us(ENSTROBEDELAY);
   GLCD_EN.write(0);
}

// Global to hold the display data (required for output damping)
short displayData[32];

// Plot the output graph for FFT
void drawbarGraph(short inputData)
{
   short inputValue;
   
   // Scale the input data to 0-63 and perform the dampening of the display
   for (unsigned char counter = 1; counter < 32; counter++)
   {
      // Scale the input data for the display (linear) x1 or x8
      if (SWITCH0 == 0) inputValue = inputData >> 1;
      else inputValue = inputData;
    //  if (inputValue > 181) inputValue = 181;
      
      // Apply a linear or logarithmic conversion on the data
      //if (SWITCH1 == 0) inputValue = (short)logTable[inputValue];
      //else inputValue = (short)linTable[inputValue];
      
      // Perform damping on the displayed output
     // if (inputValue > displayData[counter]) displayData[counter] = inputValue;
     // else displayData[counter] -= 10;
     // if (displayData[counter] < 0) displayData[counter] = 0;
		 displayData[counter] = inputValue;
		 
   }
   
   // Our FFT animation speed is dependent on how fast the LCD can
   // be updated, so here we use a bargraph drawing routine which
   // is highly optimised to the manner in which the LCD is updated.
   unsigned char xByte, requiredY, y, pointer;
   for (y = 0; y < 8; y++)
   {
      // Move to the correct screen page
      
      // Left bank
     // fLcdWrite(GLCD_CMD, 0, y | 0b10111000);
	  fLcdWrite(GLCD_CMD, 0, y | 0xb8);
     // fLcdWrite(GLCD_CMD, 0, 0b01000000);
		 fLcdWrite(GLCD_CMD, 0, 0x40);
      
      // Right bank
     // fLcdWrite(GLCD_CMD, 1, y | 0b10111000);		//les trois derniers bits sont pour la page
		 fLcdWrite(GLCD_CMD, 1, y | 0xb8);			//si y=0, on acc�de la page 0
      //fLcdWrite(GLCD_CMD, 1, 0b01000000);			//ici on r�gle pour la premi�re colonne
		 fLcdWrite(GLCD_CMD, 1, 0x40);
      
      unsigned char xPos = 0;
      
      // We only draw buckets 1 to 31 (bucket 0 is invalid)
      for (pointer = 0; pointer < 32; pointer++)
      {
         xByte = 0;
         requiredY = 63 - displayData[pointer];		//parceque la page 0 est en haut!!!
			//	requiredY = displayData[pointer];
         
         // Either fill the whole byte or
         // left shift according to the remainder of 
         // the division to get the right number of pixels
         if (requiredY <= y * 8) xByte = 0xff;		//0b11111111
         else if (requiredY / 8 <= y) xByte = 0xff << (requiredY % 8);   //0b11111111
         
         if (xPos < 64) fLcdWrite(GLCD_DATA, 0, xByte); // 1/3 of bar
         else fLcdWrite(GLCD_DATA, 1, xByte);
         xPos++;
         
         if (xPos < 64) fLcdWrite(GLCD_DATA, 0, xByte); // 1/3 of bar
         else fLcdWrite(GLCD_DATA, 1, xByte);
         xPos++;
         
         if (xPos < 64) fLcdWrite(GLCD_DATA, 0, xByte); // 1/3 of bar
         else fLcdWrite(GLCD_DATA, 1, xByte);
         xPos++;
         
         if (xPos < 64) fLcdWrite(GLCD_DATA, 0, 0x00); // gap
         else fLcdWrite(GLCD_DATA, 1, 0x00);
         xPos++;
      }
   }
}

// Dessine le graphique pour un tableau de 32 éléments (range 0 à 63)
void drawFftGraph(short inputData[])
{
   char inputValue;
   unsigned char xByte, requiredY, y, pointer;

   for (unsigned char counter = 1; counter < 32; counter++)
   {
      // Scale the input data for the display (linear) x1 or x8
      
         inputValue = inputData[counter];
         displayData[counter] = inputValue;
		 
   }
   
   for (y = 0; y < 8; y++)
   {
      // Move to the correct screen page
      
      // Left bank
     // fLcdWrite(GLCD_CMD, 0, y | 0b10111000);
		  fLcdWrite(GLCD_CMD, 0, y | 0xb8);
     // fLcdWrite(GLCD_CMD, 0, 0b01000000);
		 fLcdWrite(GLCD_CMD, 0, 0x40);
      
      // Right bank
     // fLcdWrite(GLCD_CMD, 1, y | 0b10111000);
		 fLcdWrite(GLCD_CMD, 1, y | 0xb8);
      //fLcdWrite(GLCD_CMD, 1, 0b01000000);
		 fLcdWrite(GLCD_CMD, 1, 0x40);
      
      unsigned char xPos = 0;
      
      // We only draw buckets 1 to 31 (bucket 0 is invalid)
      for (pointer = 0; pointer < 32; pointer++)
      {
         xByte = 0;
         requiredY = 63 - displayData[pointer];
         
         // Either fill the whole byte or
         // left shift according to the remainder of 
         // the division to get the right number of pixels
         if (requiredY <= y * 8) xByte = 0xff;		//0b11111111
         else if (requiredY / 8 <= y) xByte = 0xff << (requiredY % 8);   //0b11111111
         
         if (xPos < 64) fLcdWrite(GLCD_DATA, 0, xByte); // 1/3 of bar
         else fLcdWrite(GLCD_DATA, 1, xByte);
         xPos++;
         
         if (xPos < 64) fLcdWrite(GLCD_DATA, 0, xByte); // 1/3 of bar
         else fLcdWrite(GLCD_DATA, 1, xByte);
         xPos++;
         
         if (xPos < 64) fLcdWrite(GLCD_DATA, 0, xByte); // 1/3 of bar
         else fLcdWrite(GLCD_DATA, 1, xByte);
         xPos++;
         
         if (xPos < 64) fLcdWrite(GLCD_DATA, 0, 0x00); // gap
         else fLcdWrite(GLCD_DATA, 1, 0x00);
         xPos++;
      }
   }
}

// Initialise the gLCD
void gLcdInit(void)
{
   // Bring the display out of reset
   //GLCD_RES = 0; // Screen in reset
  // __delay_us(250);
		wait_us(250);
   //GLCD_RES = 1; // Screen out of reset
   //__delay_us(250);
	  wait_us(250);

   // Set Y Address = 0
   fLcdWrite(GLCD_CMD, 0, 0x40);   //0b01000000
   fLcdWrite(GLCD_CMD, 1, 0x40); //0b01000000

   // Set X Address = 0
   fLcdWrite(GLCD_CMD, 0, 0xb8);  //0b10111000
   fLcdWrite(GLCD_CMD, 1, 0xb8);  //0b10111000

   // Set Display start line = 0
   fLcdWrite(GLCD_CMD, 0, 0xc0);		//0b11000000
   fLcdWrite(GLCD_CMD, 1, 0xc0);			//0b11000000

   // Turn the display ON
   fLcdWrite(GLCD_CMD, 0, 0x3f);		//0b00111111
   fLcdWrite(GLCD_CMD, 1, 0x3f);	//0b00111111
}

// Clear the gLCD to black (zero)
void gLcdClear(void)
{
   unsigned char x, y;

   for (y = 0; y < 8; y++)
   {
      // Move to the correct screen page
      fLcdWrite(GLCD_CMD, 0, y | 0xb8);		//0b10111000
      fLcdWrite(GLCD_CMD, 0, 0x40);  //0b01000000

      fLcdWrite(GLCD_CMD, 1, y | 0xb8);  //0b10111000
      fLcdWrite(GLCD_CMD, 1, 0x40);				//0b01000000
      
      for (x = 0; x < 64; x++)
      {
         fLcdWrite(GLCD_DATA, 0, 0x00);
         fLcdWrite(GLCD_DATA, 1, 0x00);
      }
   }
}
