//
// Created by levente on 2018.03.10..
//

#ifndef SUMO_ENTRYMARKER_H
#define SUMO_ENTRYMARKER_H

#include "BaseMarker.h"
#include "ExitMarker.h"

class EntryMarker: public BaseMarker{
public:
    EntryMarker(std::string markerID, MSEdge* position);
    ~EntryMarker();

    void* onEnter(SUMOVehicle* who);
    void* onExit(SUMOVehicle* who);
    std::vector<ExitMarker*> exitMarkers;
};

#endif //SUMO_ENTRYMARKER_H
