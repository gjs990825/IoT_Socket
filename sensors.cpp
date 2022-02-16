#include "sensors.h"

uint16_t SENSORS::photoresistorGetRaw() {
    return analogRead(ADC_PIN);
}

void SENSORS::initialize() {
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

void SENSORS::updateTemperature() {
    tempearature = bmp280.readTemperature();
}

void SENSORS::updatePressure() {
    pressure = bmp280.readPressure() / 100;
}

void SENSORS::updateLight() {
    uint16_t raw = photoresistorGetRaw();
    // 0xFFF ~ 0 => 0 ~ 100 brightness
    brightness = (int)map(raw, 0xFFF, 0, 0, 100);
}

void SENSORS::updateAll() {
    updateTemperature();
    updatePressure();
    updateLight();
}

float SENSORS::getTemperature() { return tempearature; };
float SENSORS::getPressure() { return pressure; };
int SENSORS::getBrightness() { return brightness; };

SENSORS Sensors;
