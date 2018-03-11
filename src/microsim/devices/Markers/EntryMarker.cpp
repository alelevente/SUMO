//
// Created by levente on 2018.03.11..
//

#include <utils/vehicle/SUMOVehicle.h>
#include "EntryMarker.h"

void* EntryMarker::onEnter(SUMOVehicle *who) {
    std::vector<ExitMarker*> *answer = new std::vector<ExitMarker*> (exitMarkers);
    return answer;
}