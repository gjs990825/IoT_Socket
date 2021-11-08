#include "Sensors.h"

float Sensors::tempearature;
float Sensors::pressure;
float Sensors::light;

Sensors::Sensors(){};
Sensors::~Sensors(){};
void Sensors::updateTemperature() { tempearature = BMP280.readTemperature(); };
void Sensors::updatePressure() { pressure = BMP280.readPressure() / 100; };
void Sensors::updateLight() { light = Photoresistor_GetVoltage(); };
void Sensors::updateAll() {
    updateTemperature();
    updatePressure();
    updateLight();
}

float Sensors::getTemperature() { return tempearature; };
float Sensors::getPressure() { return pressure; };
float Sensors::getLight() { return light; };
