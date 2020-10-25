#include "DS18Temp.h"

#define NODEBUG_PRINT
#include <debug_print.h>

#define DS18_STATE_REQUEST_VALUES       1
#define DS18_STATE_READ_VALUES          2

DS18Temp::DS18Temp(const char* id, uint8_t pin, uint32_t readingInt, uint8_t resolution) : GenericSensor(id, readingInt)
{
    
    doc = new DynamicJsonDocument(512);
    readings = doc->to<JsonObject>();
    ds18array = readings.createNestedArray("ds18");

    this->pin = pin;

    #ifndef NODEBUG_PRINT
    aliveTimer.start(10000);
    aliveTimer.triggerTimeout();
    #endif

    if (resolution < 9 || resolution > 12) resolution = 9;
    this->resolution = resolution;

}

void byte2hex(char* dst, uint8_t* src, uint8_t length){
    char hex[] = "0123456789ABCDEF";
    for(uint8_t i=0;i<length;i++){
        *(dst+(i*2)) = hex[*(src+i)>>4];
        *(dst+(i*2)+1) = hex[*(src+i)&0x0F];
    }
    *(dst+(length*2)) = 0;
}


void DS18Temp::begin(){
    DEBUG_PRINT("[DS18-%s:begin] start id=%s\n",id);

    oneWire = new OneWire(pin);
    sensors = new DallasTemperature(oneWire);

    sensors->begin();

    detectedSensors = sensors->getDeviceCount();
    sensorCount = detectedSensors;

    if (detectedSensors == 0){
        CONSOLE("[DS18-%s:begin] ERROR: no DS18 oneWire device found\n",id);
    } else {
        CONSOLE("[DS18-%s:begin] Found %d DS18 sensors\n", id, detectedSensors);
        char hexadr[17];
        for (uint8_t s=0; s < detectedSensors; s++){
            DeviceAddress a;
            if (!sensors->getAddress(a, s)) {
                CONSOLE("[DS18-%s:begin] Error reading address for device #%d\n",id,s);
            } else {
                byte2hex(hexadr,a,8);
                CONSOLE("  %d. addr=%s\n", s+1, hexadr);
                strncpy(adrs[s], hexadr, 17);
            }
        }
    }

    sensors->setResolution(resolution);

    readingTimer.start();
    readingTimer.triggerTimeout();

    conversionTimer.start(1000/(1 << (12-resolution)));  // higher resolution -> longer conversion time
    conversionTimer.triggerTimeout();

    _ready = true;
    state = DS18_STATE_REQUEST_VALUES;

}


void DS18Temp::loop(){
    #ifndef NODEBUG_PRINT
    if (aliveTimer.timeout()){
        DEBUG_PRINT("[DS18-%s:loop] ms=%lu Alive id=%s state=%d\n", id, millis(), id, state);
        aliveTimer.reset();
    }
    #endif

    GenericSensor::loop();
    if (state == 0) return;

    if (state == DS18_STATE_REQUEST_VALUES) {
        if (!readingTimer.timeout()) return;
        readingTimer.reset();
        state = DS18_STATE_READ_VALUES;
        sensors->requestTemperatures(); // Send the command to get temperature readings 
        conversionTimer.reset();
    }

    if (state == DS18_STATE_READ_VALUES && !conversionTimer.timeout()) return;

    doc->clear();  // remove previous readings
    readings = doc->to<JsonObject>();
    ds18array = readings.createNestedArray("ds18");

    sensorCount = 0;
    for(uint8_t i=0; i < detectedSensors; i++){
        // read temperature
        float t = sensors->getTempCByIndex(i);

        if (t>DEVICE_DISCONNECTED_C){
            // get sensor address
            uint8_t adr[10];
            char hexadr[25];
            sensors->getAddress(&adr[0],i);

            byte2hex(hexadr,adr,8);

            JsonObject k = ds18array.createNestedObject();
            k["a"] = String(hexadr);    // address
            k["v"] = t;                 // value
            k["res"] = resolution;      // resolution

            DEBUG_PRINT("[DS18-%s:loop] i=%d a=%s t=%.3f\n", id, i, hexadr, t);

            sensorCount++;  // sensor returned value, so lets increase available sensor count
        } else {
            DEBUG_PRINT("[DS18-%s:loop] i=%d Error reading value\n", id, i);
        }

        delay(0);
    } 

    _changed = true;
    state = DS18_STATE_REQUEST_VALUES;

    #ifndef NODEBUG_PRINT
    DEBUG_PRINT("[DS18-%s:loop] readings=\n", id);
    serializeJsonPretty(*doc, DEBUG_PORT);
    DEBUG_PRINT("\n");
    #endif

}