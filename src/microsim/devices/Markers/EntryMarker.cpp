//
// Created by levente on 2018.03.11..
//

#include <utils/vehicle/SUMOVehicle.h>
#include "EntryMarker.h"

void* EntryMarker::onEnter(SUMOVehicle *who) {
    //std::vector<ExitMarker*> *answer = new std::vector<ExitMarker*> (exitMarkers);
    return &exitMarkers;
}

void* EntryMarker::onExit(SUMOVehicle *who) {
    return NULL;
}

EntryMarker::EntryMarker(std::string markerID, MSEdge *position):
    BaseMarker(markerID, position)
{
}

EntryMarker::~EntryMarker() {}