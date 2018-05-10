//
// Created by levente on 2018.02.27..
//

#include "helper.h"
#include <microsim/devices/MSDevice_Messenger.h>
#include <utils/vehicle/SUMOVehicle.h>

#ifdef debug
MessagesHelper::MessagesHelper() {}
MessagesHelper::~MessagesHelper() {}
#endif

MSDevice_Messenger* getMessengerDeviceFromVehicle(SUMOVehicle *vehicle){
    if (vehicle == NULL) return NULL;
    SUMOVehicle& veh = *(vehicle);
    return static_cast<MSDevice_Messenger*>(veh.getDevice(typeid(MSDevice_Messenger)));
}

std::string getJunctionName(const std::string& markerId){
    int i;
    std::string str;
    for (i = 0; markerId[i]!='_'; ++i);
    ++i;
    while (markerId[i]!='_') {
        str += markerId[i];
        ++i;
    }
    return str;
}

bool compareNames(const std::string& junctionName, const std::string& posName){
    for (int i=0; i<junctionName.length(); ++i) {
        if (junctionName[i]!=posName[i]) return false;
    }
    return true;
}