/**
 * @author Colin & Louis
 *
 * ITG3200 digital GYRO module.
 *
 * Datasheet:
 * https://www.sparkfun.com/datasheets/Sensors/Gyro/PS-ITG-3200-00-01.4.pdf
 */

/**
 * Includes
 */
#include "ITG3200.h"

ITG3200::ITG3200(PinName sda, PinName scl, int address) {

    i2c = new I2C(sda, scl);
    //Compass designed to work at 100KHz. See datasheet for details.
    i2c->frequency(100000);
    i2cAddress = address;

}

int16_t ITG3200::readAXE_X(void){

    char registerNumber = GYRO_XOUT_REG;
    char registerContents[2] = {0x00, 0x00};
   

    i2c->write(i2cAddress, &registerNumber, 1);
    
    i2c->read(i2cAddress, registerContents, 2);
 
    int16_t GYRO_X = ((int16_t)registerContents[0] << 8) | ((int16_t)registerContents[1]);
    
    return GYRO_X;
    
}

int16_t ITG3200::readAXE_Y(void){

    char registerNumber = GYRO_YOUT_REG;
    char registerContents[2] = {0x00, 0x00};
   

    i2c->write(i2cAddress, &registerNumber, 1);
    
    i2c->read(i2cAddress, registerContents, 2);
 
    int16_t GYRO_Y = ((int16_t)registerContents[0] << 8) | ((int16_t)registerContents[1]);
    
    return GYRO_Y;
    
}

int16_t ITG3200::readAXE_Z(void){

    char registerNumber = GYRO_ZOUT_REG;
    char registerContents[2] = {0x00, 0x00};
   

    i2c->write(i2cAddress, &registerNumber, 1);
    
    i2c->read(i2cAddress, registerContents, 2);
 
    int16_t GYRO_Z = ((int16_t)registerContents[0] << 8) | ((int16_t)registerContents[1]);
    
    return GYRO_Z;
    
}

int16_t ITG3200::readTEMP(void){

    char registerNumber = GYRO_TEMP_REG;
    char registerContents[2] = {0x00, 0x00};
   

    i2c->write(i2cAddress, &registerNumber, 1);
    
    i2c->read(i2cAddress, registerContents, 2);
 
    int16_t TEMP = ((int16_t)registerContents[0] << 8) | ((int16_t)registerContents[1]);
    
    return TEMP;
    
}