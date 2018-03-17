/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2018 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    MSDevice_Example.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    11.06.2013
/// @version $Id$
///
// A device which stands as an implementation example and which outputs movereminder calls
/****************************************************************************/
#ifndef MSDevice_Messenger_h
#define MSDevice_Messenger_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "MSDevice.h"
#include <utils/common/SUMOTime.h>
#include <microsim/devices/Messages/Message.h>
#include <microsim/devices/Messages/GroupMessages.h>
#include <microsim/devices/Markers/ExitMarker.h>

static const double MAX_DISTANCE=100;
static const int MAX_GROUP_MEMBERS=15;
// ===========================================================================
// class declarations
// ===========================================================================
class SUMOVehicle;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDevice_Example
 * @brief A device which collects info on the vehicle trip (mainly on departure and arrival)
 *
 * Each device collects departure time, lane and speed and the same for arrival.
 *
 * @see MSDevice
 */

struct VehData{
    double maxAccel, maxDecel;
    SUMOVehicle* vehicle;
    bool exited = false;
};

struct GroupData{
    int nMembers, nExited;
    std::vector<VehData*> memberData;
    SUMOVehicle* groupLeader;
    libsumo::TraCIColor groupColor;
    bool canJoin = false;
    inline void clear();
};

class MSDevice_Messenger : public MSDevice {
public:
    /** @brief Inserts MSDevice_Example-options
     * @param[filled] oc The options container to add the options to
     */
    static void insertOptions(OptionsCont& oc);


    /** @brief Build devices for the given vehicle, if needed
     *
     * The options are read and evaluated whether a example-device shall be built
     *  for the given vehicle.
     *
     * The built device is stored in the given vector.
     *
     * @param[in] v The vehicle for which a device may be built
     * @param[filled] into The vector to store the built device in
     */
    static void buildVehicleDevices(SUMOVehicle& v, std::vector<MSDevice*>& into);



public:
    /// @brief Destructor.
    ~MSDevice_Messenger();



    /// @name Methods called on vehicle movement / state change, overwriting MSDevice
    /// @{

    /** @brief Checks for waiting steps when the vehicle moves
     *
     * @param[in] veh Vehicle that asks this reminder.
     * @param[in] oldPos Position before move.
     * @param[in] newPos Position after move with newSpeed.
     * @param[in] newSpeed Moving speed.
     *
     * @return True (always).
     */
    bool notifyMove(SUMOVehicle& veh, double oldPos,
                    double newPos, double newSpeed);


    /** @brief Saves departure info on insertion
     *
     * @param[in] veh The entering vehicle.
     * @param[in] reason how the vehicle enters the lane
     * @return Always true
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     */
    bool notifyEnter(SUMOVehicle& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


    /** @brief Saves arrival info
     *
     * @param[in] veh The leaving vehicle.
     * @param[in] lastPos Position on the lane when leaving.
     * @param[in] isArrival whether the vehicle arrived at its destination
     * @param[in] isLaneChange whether the vehicle changed from the lane
     * @return True if it did not leave the net.
     */
    bool notifyLeave(SUMOVehicle& veh, double lastPos,
                     MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);
    /// @}


    /// @brief return the name for this type of device
    const std::string deviceName() const {
        return "smart messenger";
    }

    /// @brief try to retrieve the given parameter from this device. Throw exception for unsupported key
    std::string getParameter(const std::string& key) const;

    /// @brief try to set the given parameter for this device. Throw exception for unsupported key
    void setParameter(const std::string& key, const std::string& value);

    /** @brief Called on writing tripinfo output
     *
     * @param[in] os The stream to write the information into
     * @exception IOError not yet implemented
     * @see MSDevice::generateOutput
     */
    void generateOutput() const;

    void sendGroupcastMessage(Message* message);
    void sendBroadcastMessage(Message* message);



private:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     * @param[in] id The ID of the device
     */
    MSDevice_Messenger(SUMOVehicle& holder, const std::string& id, double customValue1,
                     double customValue2, double customValue3);


private:
    // private state members of the Example device

    /// @brief a value which is initialised based on a commandline/configuration option
    double myCustomValue1;

    /// @brief a value which is initialised based on a vehicle parameter
    double myCustomValue2;

    /// @brief a value which is initialised based on a vType parameter
    double myCustomValue3;

    bool isLeader = true;
public:

    SUMOVehicle *getLeader() const;

    void newGroup();
    SUMOVehicle* getVehicleOfGroup (int pos);
    ExitMarker* getExitMarker();

    bool isAbleToJoin (SUMOVehicle* who);
    void joinNewMember(SUMOVehicle* who);
    void setGroupMemberData(libsumo::TraCIColor color, SUMOVehicle* leader);
    void notifyLeaved(SUMOVehicle* who);
    void finishGroup();
    void resetFlag();

private:
    ExitMarker* exitMarker;
    GroupData groupData;
    libsumo::TraCIColor* originalColor;
    int flag;

private:
    void resetOriginalColor();
public:
    const std::vector<SUMOVehicle *> &getGroup() const;

private:
    /// @brief Invalidated copy constructor.
    MSDevice_Messenger(const MSDevice_Messenger&);

    /// @brief Invalidated assignment operator.
    MSDevice_Messenger operator=(const MSDevice_Messenger&);

};


#endif

/****************************************************************************/

