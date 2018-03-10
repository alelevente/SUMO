//
// Created by levente on 2018.03.10..
//

#ifndef SUMO_BASEMARKER_H
#define SUMO_BASEMARKER_H


#include <microsim/MSEdge.h>

class BaseMarker{
    MSEdge *position;
    std::string markerID;

    bool operator= (const BaseMarker& rhs);
    BaseMarker(const BaseMarker& rhs);

public:
    MSEdge* getPosition();
    void setPosition(const MSEdge* const);
    std::string getMarkerID();

    BaseMarker(std::string markerID, const MSEdge* position const);
    ~BaseMarker();

    virtual void onPassing(SUMOVehicle* who) = 0;
};
#endif //SUMO_BASEMARKER_H
