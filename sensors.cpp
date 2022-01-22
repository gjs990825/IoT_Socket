#include "Sensors.h"

float Sensors::tempearature;
float Sensors::pressure;
int Sensors::brightness;

Sensors::Sensors(){};
Sensors::~Sensors(){};

void Sensors::updateTemperature() {
    tempearature = BMP280.readTemperature();
}

void Sensors::updatePressure() {
    pressure = BMP280.readPressure() / 100;
}

void Sensors::updateLight() { 
    float raw = Photoresistor_GetVoltage() * 1000;
    // 3.3 ~ 0.0 Volt => 0 ~ 100 brightness 
    brightness = (int)map(raw, 3.3 * 1000, 0, 0, 100); 
}

void Sensors::updateAll() {
    updateTemperature();
    updatePressure();
    updateLight();
}

float Sensors::getTemperature() { return tempearature; };
float Sensors::getPressure() { return pressure; };
float Sensors::getBrightness() { return (float)brightness; };
