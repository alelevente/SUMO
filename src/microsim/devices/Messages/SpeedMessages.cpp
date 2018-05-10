//
// Created by levente on 2018.04.02..
//

#include <utils/common/StdDefs.h>
#include <microsim/devices/MSDevice_Messenger.h>
#include "SpeedMessages.h"
#include "helper.h"

LetUsIn::LetUsIn(SUMOVehicle *sender, SUMOVehicle *receiver, SUMOVehicle *who):
        Message(sender, receiver, NULL){
    UNUSED_PARAMETER(who);
}

LetUsIn::~LetUsIn() {}

void* LetUsIn::getContent() {
    return &groupLength;
}

void LetUsIn::setContent(int groupLength) {
    this->groupLength = groupLength;
}

void LetUsIn::processMessage() {
    MSDevice_Messenger* messenger = getMessengerDeviceFromVehicle(receiver);
    double vMax = messenger->getMaxSpeed();
    //double v = 0.5*vMax - (groupLength+SAFETY_DISTANCE)/ESTIMATED_LC_TIME;
    double v = 0.125*vMax;
    v = v>0? v: 0;
    messenger->addLetIn(sender, v);
}

HadToStop::HadToStop(SUMOVehicle *sender, SUMOVehicle *receiver, SUMOVehicle *who):
Message(sender, receiver, NULL)
{}

HadToStop::~HadToStop() {}

void* HadToStop::getContent() { return NULL;}
void HadToStop::setContent(void *p) {}
void HadToStop::processMessage() {
    MSDevice_Messenger* messenger = getMessengerDeviceFromVehicle(receiver);
    messenger->setVehicleSpeed(0);
}

CameIn::CameIn(SUMOVehicle *sender, SUMOVehicle *receiver, SUMOVehicle *who): Message(sender, receiver, NULL) {}
CameIn::~CameIn() {}
void CameIn::processMessage() {
    MSDevice_Messenger* messenger = getMessengerDeviceFromVehicle(receiver);
    messenger->letInMade(sender);
}