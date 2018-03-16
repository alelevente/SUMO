//
// Created by levente on 2018.03.12..
//

#include "BaseMarker.h"

MSEdge* BaseMarker::getPosition() {
    return position;
}

std::string BaseMarker::getMarkerID() {
    return markerID;
}

void BaseMarker::setPosition(MSEdge *posi) {
    position = posi;
}

BaseMarker::BaseMarker(std::string markerID, MSEdge *position):
    markerID(markerID),
    position(position)
{}

BaseMarker::~BaseMarker() {}