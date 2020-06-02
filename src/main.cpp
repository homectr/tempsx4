#include <Homie.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <Timer.h>

#include <DS18Temp.h>

//#define NODEBUG_PRINT
#include <debug_print.h>

DS18Temp *ds18;

HomieNode node("temperature", "Temperature", "temperature");

HomieSetting<const char*> temp1adr("temp1adr","Address sensor 1");
HomieSetting<const char*> temp2adr("temp2adr","Address sensor 2");
HomieSetting<const char*> temp3adr("temp3adr","Address sensor 3");
HomieSetting<const char*> temp4adr("temp4adr","Address sensor 4");
HomieSetting<const char*> temp5adr("temp5adr","Address sensor 5");

String uAdrs = "";

void loopHandler() {
    ds18->loop();
    if (ds18->valueReady()) {
        float t;
        if (temp1adr.wasProvided()){
            t = ds18->readings->getMember(temp1adr.get()).as<float>();
            DEBUG_PRINT("[main:loopH] temp1=%f\n", t);
            node.setProperty("temp1").send(String(t));
        }
        if (temp2adr.wasProvided()){
            t = ds18->readings->getMember(temp2adr.get()).as<float>();
            DEBUG_PRINT("[main:loopH] temp2=%f\n", t);
            node.setProperty("temp2").send(String(t));
        }
        if (temp3adr.wasProvided()){
            t = ds18->readings->getMember(temp3adr.get()).as<float>();
            DEBUG_PRINT("[main:loopH] temp3=%f\n", t);
            node.setProperty("temp3").send(String(t));
        }
        if (temp4adr.wasProvided()){
            t = ds18->readings->getMember(temp4adr.get()).as<float>();
            DEBUG_PRINT("[main:loopH] temp4=%f\n", t);
            node.setProperty("temp4").send(String(t));
        }
        if (temp5adr.wasProvided()){
            t = ds18->readings->getMember(temp5adr.get()).as<float>();
            DEBUG_PRINT("[main:loopH] temp5=%f\n", t);
            node.setProperty("temp5").send(String(t));
        }
        ds18->invalidate();
        node.setProperty("unassigned").send(uAdrs);
    }
}

void setup() {
    Serial.begin(9600);
    Serial << endl << endl;
    Homie_setFirmware("homectr-tempsx4", "0.0.1");
    ds18 = new DS18Temp("ds18", 4, 5000);

    temp1adr.setDefaultValue("0");
    temp2adr.setDefaultValue("0");
    temp3adr.setDefaultValue("0");
    temp4adr.setDefaultValue("0");
    temp5adr.setDefaultValue("0");

    Homie.setLoopFunction(loopHandler);
    Homie.setup();

    if (temp1adr.wasProvided())
        node.advertise("temp1").setName("Temperature 1").setDatatype("float").setUnit("ºC");
    if (temp2adr.wasProvided())
        node.advertise("temp2").setName("Temperature 2").setDatatype("float").setUnit("ºC");
    if (temp3adr.wasProvided())    
        node.advertise("temp3").setName("Temperature 3").setDatatype("float").setUnit("ºC");
    if (temp4adr.wasProvided())
        node.advertise("temp4").setName("Temperature 4").setDatatype("float").setUnit("ºC");
    if (temp5adr.wasProvided())        
        node.advertise("temp5").setName("Temperature 5").setDatatype("float").setUnit("ºC");
    
    node.advertise("unassigned").setName("Unassigned sensors").setDatatype("string"); // special property for propagating unknown devices

    ds18->begin();
    
    for(int i=0;i<DS18_MAX_SENSORS;i++){
        if (strcmp(ds18->adrs[i], temp1adr.get()) != 0
            && strcmp(ds18->adrs[i], temp2adr.get()) != 0
            && strcmp(ds18->adrs[i], temp3adr.get()) != 0
            && strcmp(ds18->adrs[i], temp4adr.get()) != 0
            && strcmp(ds18->adrs[i], temp5adr.get()) != 0)
            uAdrs = uAdrs + ds18->adrs[i] + ", ";
    }

}

void loop() {
    Homie.loop();
}