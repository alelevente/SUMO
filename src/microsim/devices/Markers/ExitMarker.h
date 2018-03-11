//
// Created by levente on 2018.03.10..
//

#ifndef SUMO_EXITMARKER_H
#define SUMO_EXITMARKER_H

#include "BaseMarker.h"

class ExitMarker: public BaseMarker{
public:
    ExitMarker(std::string markerID, const MSEdge* position const);
    ~ExitMarker();

    void* onEnter(SUMOVehicle* who);
    void* onExit(SUMOVehicle* who);
};


#endif //SUMO_EXITMARKER_H
