#if !defined(_SENSORS_H_)
#define _SENSORS_H_

#include "bsp.h"

class Sensors {
private:
    static float tempearature;
    static float pressure;
    static int brightness;

public:
    Sensors();
    ~Sensors();
    static void updateTemperature();
    static void updatePressure();
    static void updateLight();
    static void updateAll();

    static float getTemperature();
    static float getPressure();
    static float getBrightness();
};

#endif // _SENSORS_H_
