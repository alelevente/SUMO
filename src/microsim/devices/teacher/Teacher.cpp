//
// Created by levente on 2018.05.13..
//

#include <fstream>
#include <libsumo/Simulation.h>
#include "Teacher.h"
#include "microsim/devices/MSDevice_Messenger.h"

Teacher::Teacher() {
    std::ifstream confFile("/home/levente/Egyetem/6_felev/onlab/judges/teach.conf");

    confFile >> teaching;
    int db = 0;
    for (int i=0; i<20; ++i) {
        confFile >> results[i];
        if (results[i] < 0.0001 && db == 0) db = i;
    }
    confFile.close();

    if (teaching) MAX_GROUP_MEMBERS = db;
    else {
        db = 0;
        for (int i=0; i<20; ++i) if (results[i]>results[db]) db = i;
        MAX_GROUP_MEMBERS = db;
    }
}

void Teacher::newCar() {
    if (first == 0) first = libsumo::Simulation::getCurrentTime() / 1000;
    ++carsIn;
}

void Teacher::oldCar() {
    last = libsumo::Simulation::getCurrentTime() / 1000;
    ++carsOut;
}

Teacher::~Teacher(){
    std::cout << "most dtor: " << carsOut << " " << first << " " << last <<std::endl;
    double value = (double) carsOut / (last - first);
    if (teaching) {
        std::ofstream confFile("/home/levente/Egyetem/6_felev/onlab/judges/teach.conf");
        confFile << teaching << std::endl;
        for (int i = 0; i < 20; ++i) {
            if (i != MAX_GROUP_MEMBERS) confFile << results[i] << std::endl;
            else confFile << value << std::endl;
        }
        confFile.close();
    }
}

Teacher& Teacher::getInstance() {
    static Teacher teacher;
    return teacher;
}