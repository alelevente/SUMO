//
// Created by levente on 2018.05.13..
//

#ifndef SUMO_TEACHER_H
#define SUMO_TEACHER_H

#include <libsumo/TraCIDefs.h>
#include <microsim/devices/MSDevice_Messenger.h>

class Teacher;

class Teacher{
public:


    void newCar();
    void oldCar();
    static Teacher& getInstance();
    int MAX_GROUP_MEMBERS = 15;

private:
    long carsIn, carsOut;
    SUMOTime first = 0, last = 0;
    double results[20];
    bool teaching;
    Teacher();
    ~Teacher();
};

#endif //SUMO_TEACHER_H
