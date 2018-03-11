//
// Created by levente on 2018.03.10..
//

#include <microsim/MSVehicle.h>
#include "GroupMessageHandler.h"

SUMOVehicle* GroupMessageHandler::canJoin(SUMOVehicle *who) {
    MSVehicle* myVech = static_cast<MSVehicle*>(&who);
    std::pair< const MSVehicle *const, double > vech = myVech->getLeader(50);
    double tavolsag = (double)vech.second;

    MSVehicle* other = const_cast<MSVehicle*>(vech.first);
}