#include <Homie.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <Timer.h>

#include <DS18Temp.h>

#define NODEBUG_PRINT
#include <debug_print.h>

#define TEMP_INTERVAL       15000  // temperature measuring interval in milliseconds
#define TEMP_PRECISION      10     // temperature measuring precision 9-12 bits

#define DS18_PORT_COUNT     5
int ports[DS18_PORT_COUNT] = {4,5,13,14,12};  // gpios used with sensors
DS18Temp *ds18[DS18_PORT_COUNT];

HomieNode node("temperature", "Temperature", "temperature");

void loopHandler() {
    for(uint8_t pc = 0; pc < DS18_PORT_COUNT; pc++) {
        delay(10);
        ds18[pc]->loop();
        if (ds18[pc]->isConnected() && ds18[pc]->changed()) {
            float t;
            // read the first value - ignore rest -assuming single sensor connected to a single port
            t = ds18[pc]->readings["ds18"][0]["v"].as<float>(); 
            CONSOLE("[main:loopH] mil=%lu port=%d temp=%f\n", millis(), pc+1, t);
            node.setProperty("temp"+String(pc+1)).send(String(t));
            ds18[pc]->invalidate();
        }
    }
}

void setup() {
    Serial.begin(9600);
    Serial << endl << endl;

    // create and intialize sensors
    for (uint8_t i=0; i < DS18_PORT_COUNT; i++){
        ds18[i] = new DS18Temp(("ds18p"+String(i+1)).c_str(), ports[i], TEMP_INTERVAL, TEMP_PRECISION);
        ds18[i]->begin();
    }

    // configure Homie instance
    Homie_setFirmware("tempsx4", "1.0.1");
    Homie.setLoopFunction(loopHandler);
    Homie.setup();

    // configure Homie node properties
    for (uint8_t i=0; i< DS18_PORT_COUNT; i++){
        node.advertise(("temp"+String(i)).c_str()).setName(("Temperature "+String(i)).c_str()).setDatatype("float").setUnit("ºC");
    }

}

void loop() {
    Homie.loop();
}