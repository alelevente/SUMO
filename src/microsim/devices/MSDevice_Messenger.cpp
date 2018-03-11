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
#include "Messages/helper.h"
#include "Markers/MarkerSystem.h"

#ifndef debug

#endif

#ifdef debug
MessagesHelper* helper= NULL;
bool done = false;
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
            std::cout << "vehicle '" << v.getID() << "' does not supply vehicle parameter 'messenger'. Using default of " << customParameter2 << "\n";
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
            std::cout << "vehicle '" << v.getID() << "' does not supply vType parameter 'messenger'. Using default of " << customParameter3 << "\n";
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
    leader(&holder)
{
    std::cout << "initialized device '" << id << "' with myCustomValue1=" << myCustomValue1 << ", myCustomValue2=" << myCustomValue2 << ", myCustomValue3=" << myCustomValue3 << "\n";
    //setIsLeader(true);
}


MSDevice_Messenger::~MSDevice_Messenger() {
}


bool
MSDevice_Messenger::notifyMove(SUMOVehicle& veh, double /* oldPos */,
                             double /* newPos */, double newSpeed) {

    MSVehicle* myVech = static_cast<MSVehicle*>(&veh);
    std::pair< const MSVehicle *const, double > vech = myVech->getLeader(50);
    double tavolsag = (double)vech.second;

    MSVehicle* other = const_cast<MSVehicle*>(vech.first);
    if (isLeader && tavolsag>0 && tavolsag<35) {
        JoinGroupMessage* joinMessage = new JoinGroupMessage(&veh, other, &veh);
        joinMessage -> processMessage();
        delete joinMessage;
    } else


    //std::cout << "device '" << getID() << "' notifyMove: newSpeed=" << newSpeed << "\n";
    // check whether another device is present on the vehicle:
    /*MSDevice_Tripinfo* otherDevice = static_cast<MSDevice_Tripinfo*>(veh.getDevice(typeid(MSDevice_Tripinfo)));
    if (otherDevice != 0) {
        std::cout << "  veh '" << veh.getID() << " has device '" << otherDevice->getID() << "'\n";
    }*/


#ifdef debug
    if (!done && isLeader && helper!=NULL) {
        std::string *str = new std::string("Szia!");
       // Message *message = new Message(&veh, helper->partner, str);
       // sendBroadcastMessage(message);
        done = true;
        delete str;
       // delete message;
    }
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
        if (result!=NULL){
            std::vector<ExitMarker*>* exitMarkers = static_cast< std::vector<ExitMarker*> *>(result);
            exitMarker = NULL;
            for (auto i = exitMarkers->begin(); i!=exitMarkers->end(); ++i){
                if (veh.getRoute().contains((*i)->getPosition())) exitMarker = (*i);
            }
            delete exitMarkers;

            GroupMessageHandler::canJoin(&veh);
        }
    }

    return true; // keep the device
}


bool
MSDevice_Messenger::notifyLeave(SUMOVehicle& veh, double /*lastPos*/, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
    //std::cout << "device '" << getID() << "' notifyLeave: reason=" << reason << " currentEdge=" << veh.getEdge()->getID() << "\n";
    LeaveGroupMessage* message = new LeaveGroupMessage(&veh, leader, NULL);
    message -> processMessage();
    setIsLeader(true);
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
    if (!leader) throw "Cannot send groupcast message, since it is not a leader.";
    for (int i=0; i < group.size(); ++i){
        //messenger = getMessengerDeviceFromVehicle(group.at(i));
        message->setReceiver(group.at(i));
        message->processMessage();
    }
}

bool MSDevice_Messenger::isIsLeader() const {
    return isLeader;
}

void MSDevice_Messenger::setIsLeader(bool isLeader) {
    MSDevice_Messenger::isLeader = isLeader;
    libsumo::TraCIColor color;
    color.a = 255;
    color.r = 255;
    color.g = 0;
    color.b = 0;
    libsumo::Vehicle::setColor(this->getHolder().getID(), color);
}

SUMOVehicle *MSDevice_Messenger::getLeader() const {
    return leader;
}

void MSDevice_Messenger::setLeader(SUMOVehicle *leader) {
    MSDevice_Messenger::leader = leader;
}

const libsumo::TraCIColor &MSDevice_Messenger::getColor() const {
    return color;
}

void MSDevice_Messenger::setColor(const libsumo::TraCIColor &color) {
    MSDevice_Messenger::color = color;
}

void MSDevice_Messenger::addVehicleToGroup(SUMOVehicle *vehicle) {
    group.push_back(vehicle);
}

void MSDevice_Messenger::removeVehicleFromGroup(SUMOVehicle *vehicle) {
    std::vector<SUMOVehicle*>::iterator it = find(group.begin(), group.end(), vehicle);
    if (it != group.end()){
        group.erase(it);
    }
}

void MSDevice_Messenger::removeFirstVehicleFromGroup() {
    group.erase(group.begin());
}

SUMOVehicle *MSDevice_Messenger::getVehicleOfGroup(int pos) {
    return group.size()>0 ? group.at(pos) : NULL;
}

const std::vector<SUMOVehicle *> &MSDevice_Messenger::getGroup() const {
    return group;
}

void MSDevice_Messenger::newGroup(std::vector<SUMOVehicle *> *const group) {
    this->group = *group;
}

ExitMarker* MSDevice_Messenger::getExitMarker() {
    return exitMarker;
}

/****************************************************************************/

