/**
 * @author Colin & Louis
 *
 * ITG3200 digital GYRO module.
 *
 * Datasheet:
 * https://www.sparkfun.com/datasheets/Sensors/Gyro/PS-ITG-3200-00-01.4.pdf
 */

#ifndef ITG3200_H
#define ITG3200_H

/**
 * Includes
 */
#include "mbed.h"
#include "I2C.h"
#include <cstdint>

/**
 * Defines
 */
#define ITG3200_DEFAULT_I2C_ADDRESS 0b1101001

//-----------
// Registers
//-----------
#define GYRO_XOUT_REG 0x1D
#define GYRO_YOUT_REG 0x1F
#define GYRO_ZOUT_REG 0x21
#define GYRO_TEMP_REG 0x1B

/**
 * CMPS12 digital compass module.
 */
class ITG3200 {

    I2C* i2c;
    int  i2cAddress;

public:

    /**
     * Constructor.
     *
     * @param sda mbed pin to use for I2C SDA
     * @param scl mbed pin to use for I2C SCL
     * @param address I2C address of this device.
     */
    ITG3200(PinName sda, PinName scl, int address);

    /**
     * Reads the current X axis.
     *
     * @return The current X axis of the gyroscope as a value between 0 and 65535,
     *         representing +/-2000°/s
     */
    int16_t readAXE_X(void);

    /**
     * Reads the current Y axis.
     *
     * @return The current Y axis of the gyroscope as a value between 0 and 65535,
     *         representing +/-2000°/s
     */
    int16_t readAXE_Y(void);

    /**
     * Reads the current Z axis.
     *
     * @return The current Z axis of the gyroscope as a value between 0 and 65535,
     *         representing +/-2000°/s
     */
    int16_t readAXE_Z(void);
    
    /**
     * Reads the current temperature.
     *
     * @return The current temperature of the gyroscope as a value between 0 and 65535,
     *         representing -30°C to 85°C
     */
    int16_t readTEMP(void);


};

#endif /* ITG3200_H */
