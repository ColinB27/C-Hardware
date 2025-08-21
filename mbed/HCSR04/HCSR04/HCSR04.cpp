#include "HCSR04.h"


void HCSR04::startSensor(int freq)
{
    if(freq > 16)   freq = 16;
    if(freq < 0)    freq = 0;
    
    float freq_calc = 1.0f / freq;
    
    execute.attach(callback(this, &HCSR04::sendHigh), freq_calc);
}

void HCSR04::stopSensor(void)
{
    execute.detach();
}


bool HCSR04::checkUpdate(void)
{
    if(update)
    {
        update = false;
        return true;
    }
    return false;
}


float HCSR04::getDistance(void)
{
    float distance_return = distance_us * 0.03432f / 2.0f;
    return distance_return;
}

void HCSR04::startTimer(void)
{
    get_time.reset();
    get_time.start();
}

void HCSR04::stopTimer(void)
{
    get_time.stop();
    distance_us = get_time.read_us();
    update = true;
}

void HCSR04::sendHigh(void)
{
    trigger.write(1);
    go_low.attach_us(callback(this, &HCSR04::sendLow), 10);
}

void HCSR04::sendLow(void)
{
    trigger.write(0);
}