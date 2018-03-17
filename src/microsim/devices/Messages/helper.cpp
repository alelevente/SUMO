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