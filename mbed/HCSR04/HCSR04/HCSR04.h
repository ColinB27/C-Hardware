//-------------------------------------
// Short Example how to use this class:
// [returns Distance in cm(float)]
//-------------------------------------
/*
#include "mbed.h"
#include "HCSR04.h"

Serial pc(USBTX, USBRX, 115200);
HCSR04 sensor(D9, D8);

int main(void)
{    
    pc.printf("\nHCSR04 - Interrupt\n");
    sensor.startSensor(2);
    while(1)
    {
        if(sensor.checkUpdate())
        {
            pc.printf("Distance: %.2fcm \n", sensor.getDistance());
        }
    }
    return 0;
}
*/
#ifndef HCSR04_H
#define HCSR04_H

#include "mbed.h"


class HCSR04
{
private:
    // Trigger PIN - to write LOW and HIGH
    DigitalOut trigger;
    // ECHO PIN - to start Function after rising/falling flank
    InterruptIn echo;
    // Ticker object to periodicly start the measurement
    Ticker execute;
    // Timeout to write LOW to Trigger pin after 10 microseconds
    Timeout go_low;
    // Timer to measure the time between the sonic burst goes out and in again
    Timer get_time;
    // Variables for the update status and the the distance in microseconds 
    volatile float distance_us;
    volatile bool update;
    
    // Called by TICKER object to periodicly write HIGH to TRIGGER PIN
    void sendHigh(void);
    // Called by TIMEOUT Object to set TRIPPER PIN LOW again
    void sendLow(void);
    // Called by interrupt - rising flank on ECHO PIN
    void startTimer(void);
    // Called by interrupt - falling flank on ECHO PIN
    void stopTimer(void);

public:
    // Constructor
    HCSR04(PinName _trigger, PinName _echo):trigger(_trigger), echo(_echo)
    {
        // Attach member functions to InterruptIn for changing state on ECHO PIN [HIGH,LOW]
        echo.rise(callback(this, &HCSR04::startTimer));
        echo.fall(callback(this, &HCSR04::stopTimer));
        
        // Default Values
        trigger.write(0);
        update = false;
        
    }
    
    // Function to return the distance in cm, should be called if "checkUpdate()" returns true
    float getDistance(void);
    // Check for an update status for the measurement
    bool checkUpdate(void);
    // Function to start sending HIGH in periodic intervalls to TRIGGER PIN via a TICKER Object
    void startSensor(int freq);
    // Function to stop sending HIGH to TRIGGER PIN
    void stopSensor(void);
    

};

#endif