//
// Created by levente on 2018.02.28..
//

//#include <libsumo/Vehicle.h>
#include "GroupMessages.h"
#include <utils/common/TplConvert.h>
#include <utils/options/OptionsCont.h>
//#include <utils/iodevices/OutputDevice.h>
#include <utils/vehicle/SUMOVehicle.h>
//#include <microsim/MSNet.h>
//#include <microsim/MSLane.h>
//#include <microsim/MSEdge.h>
//#include <microsim/MSVehicle.h>
#include <microsim/devices/MSDevice_Messenger.h>
//#include <microsim/MSVehicleControl.h>
#include <guisim/GUIBaseVehicle.h>
#include <FXColorList.h>
#include <microsim/lcmodels/MSLCM_Smart.h>
#include "libsumo/Vehicle.h"
#include "helper.h"



JoinGroupMessage::JoinGroupMessage(SUMOVehicle *sender, SUMOVehicle *receiver, SUMOVehicle* who):
    Message(sender, receiver, NULL)
{
    this->content =who;
}

JoinGroupMessage::~JoinGroupMessage() {}

SUMOVehicle* JoinGroupMessage::getContent() {
    return content;
}

void JoinGroupMessage::setContent(SUMOVehicle *who) {
    content = who;
}

void JoinGroupMessage::processMessage() {
    /*libsumo::TraCIColor* color = new libsumo::TraCIColor(libsumo::Vehicle::getColor(receiver->getID()));
    WelcomeMessageContent* content = new WelcomeMessageContent();
    MSDevice_Messenger* otherDevice = getMessengerDeviceFromVehicle(receiver);

    if (otherDevice -> isIsLeader()) {
        //int a = (int) color->a;
        //std::cout << a <<" " << color->r << " " << color -> g << " " << color -> b << std::endl;

        color->a = 255;
        color->r = rand() % 256;
        color->g = rand() % 256;
        color->b = rand() % 256;
        content -> color = color;
    } else content->color = color;
    content->leader = otherDevice -> getLeader();
    GroupWelcomeMessage* welcomeMessage = new GroupWelcomeMessage(receiver, sender, content);
    welcomeMessage -> processMessage();
    delete color;
    delete content;
    delete welcomeMessage;*/
}

LanechangeMessage::LanechangeMessage(SUMOVehicle *sender, SUMOVehicle *receiver, LanechangeContent *content):
    Message(sender, receiver, NULL){
    this->content = content;
}

LanechangeMessage::~LanechangeMessage() {}

void LanechangeMessage::processMessage() {
    MSLCM_Smart& smartLaneCh = (MSLCM_Smart&)((MSVehicle*)receiver)->getLaneChangeModel();
    smartLaneCh.setLCAction(content->laneOffset, content->result);
}


GroupWelcomeMessage::GroupWelcomeMessage(SUMOVehicle *sender, SUMOVehicle *receiver, WelcomeMessageContent *content):
    Message(sender, receiver, NULL)
{
    this->content = content;
}

void GroupWelcomeMessage::processMessage(){
    MSDevice_Messenger* otherDevice = getMessengerDeviceFromVehicle(receiver);
    //MSDevice_Messenger* leaderDevice = getMessengerDeviceFromVehicle(content->leader);
    otherDevice -> setGroupMemberData(*content->color, content->leader);
}

WelcomeMessageContent* GroupWelcomeMessage::getContent() {
    return static_cast<WelcomeMessageContent*>(NULL);
}



GroupWelcomeMessage::~GroupWelcomeMessage() {}

LeaveGroupMessage::LeaveGroupMessage(SUMOVehicle *sender, SUMOVehicle *receiver, void *content):
        Message(sender, receiver, NULL)
{}

void LeaveGroupMessage::processMessage() {
    /*MSDevice_Messenger* senderDevice = getMessengerDeviceFromVehicle(sender);
    //MSDevice_Messenger* leaderDevice = getMessengerDeviceFromVehicle(sender->getL);
    if (senderDevice->isIsLeader()){
        SUMOVehicle* newLeader = senderDevice->getVehicleOfGroup(0);
        if (newLeader == NULL) return;

        senderDevice->removeFirstVehicleFromGroup();
        RedefineGroupLeaderContent* leaderContent = new RedefineGroupLeaderContent();
        leaderContent->leader = newLeader;
        std::vector<SUMOVehicle*>* newGroup = new std::vector<SUMOVehicle*>(senderDevice->getGroup());
        leaderContent->groupMembers = newGroup;

        //Sending to the new leader:
        NewLeaderMessage* newLeaderMessage = new NewLeaderMessage(sender, newLeader, leaderContent);
        newLeaderMessage->processMessage();

        //Sending to others:
        leaderContent->groupMembers = NULL;
        senderDevice->sendGroupcastMessage(newLeaderMessage);
        delete newLeaderMessage;
        delete newGroup;
        delete leaderContent;

    } else {
        MSDevice_Messenger* leaderDevice = getMessengerDeviceFromVehicle(senderDevice->getLeader());
        leaderDevice->removeVehicleFromGroup(sender);
    }*/
}

LeaveGroupMessage::~LeaveGroupMessage() {}

NewLeaderMessage::NewLeaderMessage(SUMOVehicle *sender, SUMOVehicle *receiver, RedefineGroupLeaderContent *content):
        Message(sender, receiver, NULL) {
    this->content = content;
}

NewLeaderMessage::~NewLeaderMessage() {}

void NewLeaderMessage::processMessage() {
    //Will it be a Leader?
    /*MSDevice_Messenger* receiverDevice = getMessengerDeviceFromVehicle(receiver);
    if (content->groupMembers!=NULL){
        receiverDevice -> setIsLeader(true);
        receiverDevice -> newGroup(content->groupMembers);
        receiverDevice -> setLeader(receiver);
    } else {
        receiverDevice -> setLeader(content->leader);
    }*/
}
