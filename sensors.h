#if !defined(_SENSORS_H_)
#define _SENSORS_H_

#include "bsp.h"

class Sensors {
private:
public:
    Sensors();
    ~Sensors();
    static void updateTemperature();
    static void updatePressure();
    static void updateLight();
    static void updateAll();

    static float getTemperature();
    static float getPressure();
    static float getLight();

    static float tempearature;
    static float pressure;
    static float light;
};

#endif // _SENSORS_H_
