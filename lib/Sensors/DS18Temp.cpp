#include "DS18Temp.h"

//#define NODEBUG_PRINT
#include <debug_print.h>

DS18Temp::DS18Temp(const char* id, uint8_t pin, uint32_t readingInt, uint8_t resolution) : GenericSensor(id)
{
    
    readings = new DynamicJsonDocument(512);

    this->pin = pin;
    readingInterval = readingInt;

    #ifndef NODEBUG_PRINT
    aliveTimer.start(10000);
    aliveTimer.triggerTimeout();
    #endif

    if (resolution < 9 || resolution > 12) resolution = 9;

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
    DeviceAddress a;
    DEBUG_PRINT("[DS18:begin] start id=%s\n",id);

    oneWire = new OneWire(pin);
    sensors = new DallasTemperature(oneWire);

    if (!oneWire->search(a)){
        CONSOLE("[DS18:begin] ERROR: no DS18 oneWire device found\n");
    } else {
        char hexadr[17];
        int i=0;
        CONSOLE("[DS18:begin] Searching for DS18 sensors\n");
        do {
            byte2hex(hexadr,a,8);
            CONSOLE("%d. addr=%s\n", ++i, hexadr);
            strncpy(adrs[i], hexadr, 17);
        } while (oneWire->search(a));
        CONSOLE("[DS18:begin] Found %d sensors\n",i);
    } 

    sensors->begin();

    readingTimer.start(readingInterval);
    readingTimer.triggerTimeout();

    conversionTimer.start(1000/(1 << (12-resolution)));  // higher resolution -> longer conversion time
    conversionTimer.triggerTimeout();

    _ready = true;
    state = 1;

}


void DS18Temp::loop(){
    #ifndef NODEBUG_PRINT
    if (aliveTimer.timeout()){
        DEBUG_PRINT("[DS18:loop] ms=%lu Alive id=%s state=%d\n",millis(),id, state);
        aliveTimer.reset();
    }
    #endif

    GenericSensor::loop();
    if (state == 0) return;

    if (state == 1) {
        if (!readingTimer.timeout()) return;
        readingTimer.reset();
        state = 2;
        sensors->requestTemperatures(); // Send the command to get temperature readings 
        conversionTimer.reset();
    }

    if (state == 2 && !conversionTimer.timeout()) return;

    uint8_t c = sensors->getDS18Count();
    
    if (c == 0) {
        DEBUG_PRINT("[DS18:loop] No DS18 sensors detected.\n" );
        return;
    }

    DEBUG_PRINT("[DS18:loop] sensors=%d\n",c);

    readings->clear();  // remove previous readings

    for(uint8_t i=0; i<c; i++){
        // read temperature
        
        float t = sensors->getTempCByIndex(i);
        DEBUG_PRINT("[reading] i=%d temp=%f\n",i,t);

        // get sensor address
        uint8_t adr[10];
        char hexadr[25];
        sensors->getAddress(&adr[0],i);

        byte2hex(hexadr,adr,8);

        JsonVariant k = readings->getOrAddMember(hexadr);
        k.set(t);

        DEBUG_PRINT("[DS18:loop] i=%d a=%s t=%.3f\n",i,hexadr,t);
    }

    _changed = true;
    state = 1;

    #ifndef NODEBUG_PRINT
    DEBUG_PRINT("[DS18:loop] readings=\n");
    serializeJsonPretty(*readings, DEBUG_PORT);
    DEBUG_PRINT("\n");
    #endif

}