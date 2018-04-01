/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2018 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    MSDevice_Example.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    11.06.2013
/// @version $Id$
///
// A device which stands as an implementation example and which outputs movereminder calls
/****************************************************************************/

// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <utils/common/TplConvert.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSVehicle.h>
#include "MSDevice_Tripinfo.h"
#include "MSDevice_Messenger.h"
#include "Messages/helper.h"
#include <microsim/MSVehicleControl.h>
#include <guisim/GUIBaseVehicle.h>
#include "Messages/GroupMessages.h"
#include "libsumo/Vehicle.h"
#include <algorithm>
#include <microsim/devices/Messages/GroupMessageHandler.h>
#include <libsumo/VehicleType.h>
#include <microsim/lcmodels/MSLCM_Smart.h>
#include <microsim/cfmodels/MSCFModel_Smart.h>
#include "Messages/helper.h"
#include "Markers/MarkerSystem.h"

#ifndef debug

#endif

#ifdef debug
MessagesHelper* helper= NULL;
bool done = false;
#endif

#ifndef GROUP_DEBUG

#endif

// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static initialisation methods
// ---------------------------------------------------------------------------
void
MSDevice_Messenger::insertOptions(OptionsCont& oc) {
    oc.addOptionSubTopic("Messenger Device");
    insertDefaultAssignmentOptions("messenger", "Messenger Device", oc);

    oc.doRegister("device.messenger.parameter", new Option_Float(0.0));
    oc.addDescription("device.messenger.parameter", "Messenger Device", "An exemplary parameter which can be used by all instances of the example device");
}


void
MSDevice_Messenger::buildVehicleDevices(SUMOVehicle& v, std::vector<MSDevice*>& into) {
    OptionsCont& oc = OptionsCont::getOptions();
    if (equippedByDefaultAssignmentOptions(oc, "messenger", v)) {
        // build the device
        // get custom vehicle parameter
        double customParameter2 = -1;
        if (v.getParameter().knowsParameter("messenger")) {
            try {
                customParameter2 = TplConvert::_2double(v.getParameter().getParameter("messenger", "-1").c_str());
            } catch (...) {
                WRITE_WARNING("Invalid value '" + v.getParameter().getParameter("messenger", "-1") + "'for vehicle parameter 'messenger'");
            }

        } else {
            //std::cout << "vehicle '" << v.getID() << "' does not supply vehicle parameter 'messenger'. Using default of " << customParameter2 << "\n";
        }
        // get custom vType parameter
        double customParameter3 = -1;
        if (v.getVehicleType().getParameter().knowsParameter("messenger")) {
            try {
                customParameter3 = TplConvert::_2double(v.getVehicleType().getParameter().getParameter("messenger", "-1").c_str());
            } catch (...) {
                WRITE_WARNING("Invalid value '" + v.getVehicleType().getParameter().getParameter("messenger", "-1") + "'for vType parameter 'example'");
            }

        } else {
           // std::cout << "vehicle '" << v.getID() << "' does not supply vType parameter 'messenger'. Using default of " << customParameter3 << "\n";
        }
        MSDevice_Messenger* device = new MSDevice_Messenger(v, "messenger_" + v.getID(),
                oc.getFloat("device.messenger.parameter"),
                customParameter2,
                customParameter3);
        into.push_back(device);
    }
}


// ---------------------------------------------------------------------------
// MSDevice_Example-methods
// ---------------------------------------------------------------------------
MSDevice_Messenger::MSDevice_Messenger(SUMOVehicle& holder, const std::string& id,
                                   double customValue1, double customValue2, double customValue3) :
    MSDevice(holder, id),
    myCustomValue1(customValue1),
    myCustomValue2(customValue2),
    myCustomValue3(customValue3),
    originalColor(NULL)
{
    //GUIBaseVehicle baseVehicle;
    //originalColor = libsumo::VehicleType::getColor(holder.getID());
    groupData.groupLeader = NULL;
    flag = 0;
    //std::cout << "initialized device '" << id << "' with myCustomValue1=" << myCustomValue1 << ", myCustomValue2=" << myCustomValue2 << ", myCustomValue3=" << myCustomValue3 << "\n";
    //setIsLeader(true);
}


MSDevice_Messenger::~MSDevice_Messenger() {
}


bool
MSDevice_Messenger::notifyMove(SUMOVehicle& veh, double /* oldPos */,
                             double /* newPos */, double newSpeed) {

    /*MSVehicle* myVech = static_cast<MSVehicle*>(&veh);
    std::pair< const MSVehicle *const, double > vech = myVech->getLeader(50);
    double tavolsag = (double)vech.second;

    MSVehicle* other = const_cast<MSVehicle*>(vech.first);
    if (isLeader && tavolsag>0 && tavolsag<35) {
        JoinGroupMessage* joinMessage = new JoinGroupMessage(&veh, other, &veh);
        joinMessage -> processMessage();
        delete joinMessage;
    } else*/


    //std::cout << "device '" << getID() << "' notifyMove: newSpeed=" << newSpeed << "\n";
    // check whether another device is present on the vehicle:
    /*MSDevice_Tripinfo* otherDevice = static_cast<MSDevice_Tripinfo*>(veh.getDevice(typeid(MSDevice_Tripinfo)));
    if (otherDevice != 0) {
        std::cout << "  veh '" << veh.getID() << " has device '" << otherDevice->getID() << "'\n";
    }*/

    if (originalColor==NULL) {
        originalColor = new libsumo::TraCIColor(libsumo::VehicleType::getColor(veh.getVehicleType().getID()));
    }

    //if (flag>0){
#ifdef debug
        /*std::cout << veh.getID()<< " auto: " << veh.getEdge()->getID() << " helyen vagyok."<<std::endl;
        MSVehicle* myVech = static_cast<MSVehicle*>(&veh);
        std::pair< const MSVehicle *const, double > vech = myVech->getLeader(50);
        if (vech.first!=NULL) {
            std::cout << vech.first->getID() << " van elottem." <<std::endl;
            flag=0;
        }*/
        if (veh.getID()=="carflow9.0") {
            MSVehicle* myVech = static_cast<MSVehicle*>(&veh);
            std::pair< const MSVehicle *const, double > vech = myVech->getLeader(50);
            if (vech.first!=NULL) {
                std::cout << vech.first->getID() << " van elottem." << std::endl;
            } else
                std::cout << "Nincs elottem senki." << std::endl;
        }
#endif
    if (flag==1){
        GroupMessageHandler::Join(&veh);
    }
    if (flag>0) flag--;


#ifdef debug

#endif
    return true; // keep the device
}


bool
MSDevice_Messenger::notifyEnter(SUMOVehicle& veh, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
    //std::cout << "device '" << getID() << "' notifyEnter: reason=" << reason << " currentEdge=" << veh.getEdge()->getID() << "\n";

    void* result = NULL;
    if (MarkerSystem::isMarkerID(veh.getEdge()->getID())) {
        result = MarkerSystem::getInstance().findMarkerByID(veh.getEdge()->getID())->onEnter(&veh);
        //EntryMarker:
        if (result!=NULL && groupData.groupLeader==NULL){
            std::vector<ExitMarker*>* exitMarkers = static_cast< std::vector<ExitMarker*> *>(result);
            exitMarker = NULL;
            for (auto i = exitMarkers->begin(); i!=exitMarkers->end(); ++i){
                if (veh.getRoute().contains((*i)->getPosition())) exitMarker = (*i);
            }
            //delete exitMarkers;

            flag = STEP_FLAG;//GroupMessageHandler::Join(&veh);
            MSVehicle* me = (MSVehicle*) &veh;
            MSLCM_Smart* smartLeader = (MSLCM_Smart*) &(me->getLaneChangeModel());
            smartLeader->requestChange(-100);
        }
    }

    return true; // keep the device
}


bool
MSDevice_Messenger::notifyLeave(SUMOVehicle& veh, double /*lastPos*/, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
    //std::cout << "device '" << getID() << "' notifyLeave: reason=" << reason << " currentEdge=" << veh.getEdge()->getID() << "\n";

    /*LeaveGroupMessage* message = new LeaveGroupMessage(&veh, leader, NULL);
    message -> processMessage();
    setIsLeader(true);*/

    void* result = NULL;
    if (MarkerSystem::isMarkerID(veh.getEdge()->getID())) {
        result = MarkerSystem::getInstance().findMarkerByID(veh.getEdge()->getID())->onExit(&veh);
        //ExitMarker:
        if (result!=NULL && groupData.groupLeader!=NULL){

            GroupMessageHandler::tryToLeave(groupData.groupLeader, &veh);
        } else if (groupData.groupLeader == &veh) {         //Leader left the entryMarker
            groupData.canJoin = false;
        }
    }

    return true; // keep the device
}


void
MSDevice_Messenger::generateOutput() const {
    if (OptionsCont::getOptions().isSet("tripinfo-output")) {
        OutputDevice& os = OutputDevice::getDeviceByOption("tripinfo-output");
        os.openTag("example_device");
        os.writeAttr("customValue1", toString(myCustomValue1));
        os.writeAttr("customValue2", toString(myCustomValue2));
        os.closeTag();
    }
}

std::string
MSDevice_Messenger::getParameter(const std::string& key) const {
    if (key == "customValue1") {
        return toString(myCustomValue1);
    } else if (key == "customValue2") {
        return toString(myCustomValue2);
    } else if (key == "meaningOfLife") {
        return "42";
    }
    throw InvalidArgument("Parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
}


void
MSDevice_Messenger::setParameter(const std::string& key, const std::string& value) {
    double doubleValue;
    try {
        doubleValue = TplConvert::_2double(value.c_str());
    } catch (NumberFormatException) {
        throw InvalidArgument("Setting parameter '" + key + "' requires a number for device of type '" + deviceName() + "'");
    }
    if (key == "customValue1") {
        myCustomValue1 = doubleValue;
    } else {
        throw InvalidArgument("Setting parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
    }
}



void MSDevice_Messenger::sendBroadcastMessage(Message *message) {
    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    MSVehicleControl::constVehIt begin = vc.loadedVehBegin();
    MSVehicleControl::constVehIt end = vc.loadedVehEnd();

    for (MSVehicleControl::constVehIt i = begin; i!=end; ++i){
        const MSVehicle* veh = static_cast<const MSVehicle*>((*i).second);
        MSVehicle* myVeh = const_cast<MSVehicle*>(veh);
        message->setReceiver(myVeh);
        message->processMessage();
    }
}

void MSDevice_Messenger::sendGroupcastMessage(Message *message) {
    MSDevice_Messenger* messenger;
    if (!(groupData.groupLeader!=&myHolder)) throw "Cannot send groupcast message, since it is not a leader.";
    for (int i=0; i < groupData.nMembers; ++i){
        //messenger = getMessengerDeviceFromVehicle(group.at(i));
        message->setReceiver(groupData.memberData[i]->vehicle);
        message->processMessage();
    }
}


SUMOVehicle *MSDevice_Messenger::getLeader() const {
    return groupData.groupLeader;
}


ExitMarker* MSDevice_Messenger::getExitMarker() {
    return exitMarker;
}

bool MSDevice_Messenger::isAbleToJoin(SUMOVehicle *who) {
    return (groupData.nMembers < MAX_GROUP_MEMBERS && groupData.canJoin);
}

void MSDevice_Messenger::resetFlag() {
    flag = 0;
}

void MSDevice_Messenger::joinNewMember(SUMOVehicle *who) {
    MSLCM_Smart& smartLaneCh = (MSLCM_Smart&)((MSVehicle*)who)->getLaneChangeModel();
    smartLaneCh.requestChange(0);
    MSVehicle* me = (MSVehicle*) &myHolder;
    ((MSCFModel_Smart*) &(const_cast<MSCFModel&>(me -> getCarFollowModel()))) ->setSmart(true);

    MSLCM_Smart* smartLeader = (MSLCM_Smart*) &(me->getLaneChangeModel());
    smartLaneCh.setLeader(smartLeader);
    VehData* newMember = new VehData();
    newMember->vehicle = who;
    newMember->maxAccel = libsumo::VehicleType::getAccel(who->getVehicleType().getID());
    newMember->maxDecel = libsumo::VehicleType::getDecel(who->getVehicleType().getID());

    ++groupData.nMembers;
    groupData.memberData.push_back(newMember);

    WelcomeMessageContent* content = new WelcomeMessageContent();
    content->leader = &myHolder;
    content->color = &groupData.groupColor;
    GroupWelcomeMessage* message = new GroupWelcomeMessage(&myHolder, who, content);
    message->processMessage();
    delete message;
    delete content;
}

inline void GroupData::clear() {
    nMembers = 0;
    nExited = 0;

    memberData.clear();
    groupLeader = NULL;
    canJoin = false;
}

void MSDevice_Messenger::newGroup() {
    groupData.clear();
    groupData.groupLeader = &myHolder;
    groupData.canJoin = true;
    flag = 0;
    MSLCM_Smart& smartLaneCh = (MSLCM_Smart&)((MSVehicle*)(&myHolder))->getLaneChangeModel();
    smartLaneCh.requestChange(-1);

    libsumo::TraCIColor color;
    color.a = 255;
    color.r = rand() % 256;
    color.g = rand() % 256;
    color.b = rand() % 256;
    groupData.groupColor = color;
    color.r -= color.r*0.33;
    color.g -= color.g*0.33;
    color.b -= color.b*0.33;
#ifdef GROUP_DEBUG
    color.r = 255;
    color.g = 255;
    color.b = 0;
    groupData.groupColor = color;
    color.g = 0;
#endif
    libsumo::Vehicle::setColor(this->getHolder().getID(), color);
}

void MSDevice_Messenger::setGroupMemberData(libsumo::TraCIColor color, SUMOVehicle *leader) {
    groupData.clear();
    groupData.groupColor = color;
    groupData.groupLeader = leader;
    libsumo::Vehicle::setColor(myHolder.getID(),color);
}

void MSDevice_Messenger::notifyLeaved(SUMOVehicle *who) {

    for (auto i = groupData.memberData.begin(); i!=groupData.memberData.end(); ++i){
        if ((*i)->vehicle == who && !(*i)->exited) {
            groupData.nExited++;
            (*i)->exited = true;
        }
    }
    if (groupData.nExited == groupData.nMembers) finishGroup();
}

void MSDevice_Messenger::finishGroup() {
#ifdef debug
    std::cout << "Csoportbontas." << std::endl;
#endif
    MSDevice_Messenger* other;
    for (auto i = groupData.memberData.begin(); i!=groupData.memberData.end(); ++i){
        other = getMessengerDeviceFromVehicle((*i)->vehicle);
        other->groupData.clear();
        other->resetOriginalColor();
        MSLCM_Smart& smartLaneCh = (MSLCM_Smart&)((MSVehicle*)(*i)->vehicle)->getLaneChangeModel();
        smartLaneCh.requestChange(-1);
        smartLaneCh.setLeader(NULL);
        delete *i;
    }
    groupData.clear();
    resetOriginalColor();
}

void MSDevice_Messenger::resetOriginalColor() {
    libsumo::Vehicle::setColor(myHolder.getID(),*originalColor);
}

/****************************************************************************/

