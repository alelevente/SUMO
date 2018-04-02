//
// Created by levente on 2018.04.02..
//

#ifndef SUMO_SPEEDMESSAGES_H
#define SUMO_SPEEDMESSAGES_H

static int SAFETY_DISTANCE = 10;
static double ESTIMATED_LC_TIME = 15;

#include "Message.h"

class LetUsIn: public Message{
public:
    LetUsIn(SUMOVehicle* sender, SUMOVehicle* receiver, SUMOVehicle* who);
    ~LetUsIn();
    void* getContent();
    void setContent (int groupLength);
    inline void processMessage();

private:
    int groupLength;
};

class HadToStop: public Message{
public:
    HadToStop(SUMOVehicle* sender, SUMOVehicle* receiver, SUMOVehicle* who);
    ~HadToStop();
    void* getContent();
    void setContent (void* p);
    inline void processMessage();
};

class CameIn: public Message{
public:
    CameIn(SUMOVehicle* sender, SUMOVehicle* receiver, SUMOVehicle* who);
    ~CameIn();
    void* getContent();
    void setContent (void* p);
    inline void processMessage();
};

#endif //SUMO_SPEEDMESSAGES_H
