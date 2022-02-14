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

    uint16_t photoresistorGetRaw() {
        return analogRead(ADC_PIN);
    }

public:
    void initialize() {
        Wire.begin(BMP280_SDA_PIN, BMP280_SCL_PIN);
        if (!bmp280.begin(BMP280_ADDRESS_ALT)) {
            log_e("BMP280 ERR!");
        } else {
            bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                               Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                               Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                               Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                               Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
        }
    }

    void updateTemperature() {
        tempearature = bmp280.readTemperature();
    }
    void updatePressure() {
        pressure = bmp280.readPressure() / 100;
    }
    void updateLight() {
        uint16_t raw = photoresistorGetRaw();
        // 0xFFF ~ 0 => 0 ~ 100 brightness
        brightness = (int)map(raw, 0xFFF, 0, 0, 100);
    }
    void updateAll() {
        updateTemperature();
        updatePressure();
        updateLight();
    }

    float getTemperature() { return tempearature; };
    float getPressure() { return pressure; };
    int getBrightness() { return brightness; };
};

static SENSORS Sensors;

#endif // _SENSORS_H_
