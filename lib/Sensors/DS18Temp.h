#ifndef __DS18TEMP_H__
#define __DS18TEMP_H__

#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <Timer.h>

#include <GenericItem.h>

#define DS18_MAX_SENSORS    5

class DS18Temp: public GenericSensor {
    protected:
        uint8_t pin;
        uint8_t resolution; // 9-12 bits, corresponding to 0.5°C, 0.25°C, 0.125°C, and 0.0625°C. HW default is 12-bit.
        Timer conversionTimer;

        OneWire *oneWire;
        DallasTemperature *sensors;
        uint8_t state = 0;
        
    public:
        DynamicJsonDocument *readings;
        char adrs[DS18_MAX_SENSORS][17]; // list of sensor addresses

    public:
        DS18Temp(const char* id, uint8_t pin, uint32_t readingInt, uint8_t resolution=9);
        void begin() override;
        void loop();

};

#endif