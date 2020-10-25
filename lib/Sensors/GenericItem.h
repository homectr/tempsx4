#ifndef __GENERICITEM_H__
#define __GENERICITEM_H__

#include <Timer.h>
#include <Looper.h>


class GenericItem : public Looper {
    protected:
    const char *id;
    Timer aliveTimer;

    bool _ready = false;  // started & ready for operation

    public:
    GenericItem(const char *id){this->id = strdup(id);}
    ~GenericItem(){free((void*)id);}

    virtual void begin()=0;

    // itam/sensor/actor has been initialized and is ready for operation
    virtual bool isReady(){return _ready;}

};

class GenericSensor: public GenericItem {
    protected:
    Timer readingTimer;
    bool _changed = false;

    public:
    GenericSensor(const char *id, uint32_t interval=1000):GenericItem(id){
        readingTimer.set(interval); 
        readingTimer.start();
    };
    
    // is sensor connected & operational?
    virtual bool isConnected()=0; 

    // value has chan
    virtual bool changed() {return _changed || readingTimer.timeout();}

    // invalidate sensor value - make sensor read new value
    virtual void invalidate() {_changed = false; readingTimer.reset();}

    void setReadingInterval(uint32_t interval){
        readingTimer.set(interval);
    }

};



#endif