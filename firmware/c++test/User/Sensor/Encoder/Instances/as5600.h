
#ifndef __AS5600_H
#define __AS5600_H

#include "encoder_base.h"

class EncoderAS5600Base : public EncoderBase
{
public:
    const static uint8_t RESOLUTION_BITS = 12;
    const static uint8_t AS5600_ADDR = 0x36;
    const static uint8_t AS5600_RAW_ANGLE_REGISTER = 0x0C;

    EncoderAS5600Base()
    {
        VarInit();
    }
    // virtual void Init();

private:
    /***** Platform Specified Implements *****/
    virtual void i2cInit();
    virtual int i2cWrite(uint8_t dev_addr, uint8_t *pData, uint32_t count);
    virtual int i2cRead(uint8_t dev_addr, uint8_t *pData, uint32_t count);

    virtual float GetRawAngle();
};

#endif
