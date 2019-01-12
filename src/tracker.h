#ifndef _tracker_h
#define _tracker_h

#include "api/materialstorage.h"


class Tracker 
{
public:
    Tracker();
    ~Tracker();
    Tracker(const Tracker& other) = delete;
    Tracker(Tracker&& other) = delete;

    void tick();

    void show();
    void start();
    void stop();

    bool visible = false;

private:
    MaterialStorageData materials;
};

#endif //_tracker_h