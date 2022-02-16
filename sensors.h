#if !defined(_SENSORS_H_)
#define _SENSORS_H_

#include <Adafruit_BMP280.h>
#include "bsp.h"

class SENSORS
{
private:
    float tempearature;
    float pressure;
    int brightness;
    Adafruit_BMP280 bmp280;

    uint16_t photoresistorGetRaw();

public:
    void initialize();

    void updateTemperature();
    void updatePressure();
    void updateLight();
    void updateAll();

    float getTemperature();
    float getPressure();
    int getBrightness();
};

extern SENSORS Sensors;

#endif // _SENSORS_H_
