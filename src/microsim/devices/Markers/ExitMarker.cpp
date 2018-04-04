//
// Created by levente on 2018.03.13..
//

#include "ExitMarker.h"

ExitMarker::ExitMarker(std::string markerID, MSEdge *position):
    BaseMarker(markerID, position)
{}

ExitMarker::~ExitMarker() {}

void* ExitMarker::onEnter(SUMOVehicle *who) {
    return NULL;
}

void* ExitMarker::onExit(SUMOVehicle *who) {
    return this;
}