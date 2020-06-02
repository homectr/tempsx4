#ifndef __GENERICITEM_H__
#define __GENERICITEM_H__

#include <Timer.h>
#include <Looper.h>


class GenericItem : public Looper {
    protected:
    const char *id;
    Timer aliveTimer;
    bool _changed = false;
    bool _ready = false;  // started & ready for operation

    public:
    GenericItem(const char *id){this->id = strdup(id);}
    ~GenericItem(){free((void*)id);}
    bool changed(){return _changed;}
    void invalidate(){_changed = false;};
    virtual void begin()=0;

};

class GenericSensor: public GenericItem {
    protected:
    uint32_t readingInterval = 1000;
    Timer readingTimer;

    public:
    GenericSensor(const char *id):GenericItem(id){};
    String getValue();
    bool valueReady(){return _changed || readingTimer.timeout();}
    void invalidate() {_changed = false; readingTimer.reset();}
};



#endif