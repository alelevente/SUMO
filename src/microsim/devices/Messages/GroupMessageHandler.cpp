//
// Created by levente on 2018.03.10..
//

#include <microsim/MSVehicle.h>
#include <microsim/devices/MSDevice_Messenger.h>
#include "GroupMessageHandler.h"
#include "helper.h"

SUMOVehicle* GroupMessageHandler::Join(SUMOVehicle *who) {
    MSVehicle* myVech = static_cast<MSVehicle*>(who);
    std::pair< const MSVehicle *const, double > vech = myVech->getLeader(MAX_DISTANCE);

    MSVehicle* other = const_cast<MSVehicle*>(vech.first);
    //Going in the direction of the same ExitMarker?
    MSDevice_Messenger* follower = getMessengerDeviceFromVehicle(myVech);
    MSDevice_Messenger* followed = getMessengerDeviceFromVehicle(other);
    if (followed!=NULL && followed->getExitMarker() == follower->getExitMarker()){
        MSDevice_Messenger* leader = getMessengerDeviceFromVehicle(followed->getLeader());
        //Good group?
        if (leader != NULL && leader->isAbleToJoin(who)) {
            leader->joinNewMember(who);
            follower->resetFlag();
        }
        else {
            follower->newGroup();
            follower->resetFlag();
        }
    } else {
        follower->newGroup();
        follower->resetFlag();
    }
}

void GroupMessageHandler::tryToLeave(SUMOVehicle *leader, SUMOVehicle *who) {
    getMessengerDeviceFromVehicle(leader)->notifyLeaved(who);
}