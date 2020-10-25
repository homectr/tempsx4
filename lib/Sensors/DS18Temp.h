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
        uint8_t resolution = 9; // 9-12 bits, corresponding to 0.5°C, 0.25°C, 0.125°C, and 0.0625°C. HW default is 12-bit.
        Timer conversionTimer;

        OneWire *oneWire;
        DallasTemperature *sensors;
        uint8_t sensorCount = 0; // count of sensors which returned value
        uint8_t detectedSensors = 0; // count of sensors detected at startup
        uint8_t state = 0;
        DynamicJsonDocument *doc;
        JsonArray ds18array;
        
    public:
        JsonObject readings;
        char adrs[DS18_MAX_SENSORS][17]; // list of sensor addresses

    public:
        DS18Temp(const char* id, uint8_t pin, uint32_t readingInt=5000, uint8_t resolution=10);
        void begin() override;
        void loop();
        bool isConnected() override {return sensorCount>0;}

};

#endif