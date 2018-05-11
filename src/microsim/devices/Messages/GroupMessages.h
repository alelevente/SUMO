//
// Created by levente on 2018.02.28..
//

#include "Message.h"
#include <libsumo/TraCIDefs.h>
#include <vector>

#ifndef SUMO_GROUPMESSAGES_H
#define SUMO_GROUPMESSAGES_H



class SUMOVehicle;

struct RedefineGroupLeaderContent{
    std::vector<SUMOVehicle*> *groupMembers;
    SUMOVehicle* leader;
};

struct WelcomeMessageContent{
    libsumo::TraCIColor* color;
    SUMOVehicle* leader;
};

struct LanechangeContent{
    int laneOffset, result;
};

class JoinGroupMessage: public Message{
public:
    JoinGroupMessage(SUMOVehicle* sender, SUMOVehicle* receiver, SUMOVehicle* who);
    ~JoinGroupMessage();
    SUMOVehicle* getContent();
    void setContent (SUMOVehicle* who);
    void processMessage();

private:
    SUMOVehicle* content;
};

class GroupWelcomeMessage: public Message{
public:
    GroupWelcomeMessage(SUMOVehicle* sender, SUMOVehicle* receiver, WelcomeMessageContent* content);
    WelcomeMessageContent* getContent();
    ~GroupWelcomeMessage();
    void processMessage();

private:
    WelcomeMessageContent* content;
};

class LeaveGroupMessage: public Message{
public:
    LeaveGroupMessage(SUMOVehicle* sender, SUMOVehicle* receiver, void* content);
    ~LeaveGroupMessage();
    void processMessage();
};

class NewLeaderMessage: public Message{
public:
    NewLeaderMessage(SUMOVehicle* sender, SUMOVehicle* receiver, RedefineGroupLeaderContent* content);
    ~NewLeaderMessage();
    void processMessage();

private:
    RedefineGroupLeaderContent* content;
};

class LanechangeMessage: public Message{
public:
    LanechangeMessage(SUMOVehicle* sender, SUMOVehicle* receiver, LanechangeContent* content);
    ~LanechangeMessage();
    void processMessage();
private:
    LanechangeContent* content;
};

#endif //SUMO_GROUPMESSAGES_H