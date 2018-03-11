//
// Created by levente on 2018.03.10..
//

#ifndef SUMO_GROUPMESSAGEHANDLER_H
#define SUMO_GROUPMESSAGEHANDLER_H

#include "GroupMessages.h"

class GroupMessageHandler{
public:

    /** @brief Try to join a SUMOVehicle to a group.
     *
     *  This function has the side-effect that the car will be joined to the group
     *
     *
     *  @return True if the joining was successful.
     */
    static bool joinGroup (SUMOVehicle* joiner, SUMOVehicle* leader);

    /** @brief Ask the vehicle in front if its group can be good for us.
     *
     * @return A pointer to the group's leader if the group is good. Otherwise: NULL
     */

    //todo: implement path
    static SUMOVehicle* canJoin(SUMOVehicle* who);
};

#endif //SUMO_GROUPMESSAGEHANDLER_H
