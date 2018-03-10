//
// Created by levente on 2018.03.10..
//

#ifndef SUMO_ENTRYMARKER_H
#define SUMO_ENTRYMARKER_H

#include "BaseMarker.h"

class EntryMarker: public BaseMarker{
public:
    EntryMarker(std::string markerID, const MSEdge* position const);
    ~EntryMarker();

    void onPassing(SUMOVehicle* who);
};

#endif //SUMO_ENTRYMARKER_H
