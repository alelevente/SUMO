//
// Created by levente on 2018.02.27..
//

#ifndef SUMO_HELPER_H
#define SUMO_HELPER_H

#include <microsim/devices/MSDevice_Messenger.h>

#ifndef debug

#endif

class SUMOVehicle;

class MessagesHelper{
#ifdef debug
public:
    MessagesHelper();
    ~MessagesHelper();

    SUMOVehicle *partner;


#endif
};

MSDevice_Messenger* getMessengerDeviceFromVehicle(SUMOVehicle *vehicle);
std::string getJunctionName(const std::string& markerId);
bool compareNames(const std::string& junctionName, const std::string& posName);

#endif //SUMO_HELPER_H
