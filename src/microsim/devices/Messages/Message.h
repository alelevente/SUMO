//
// Created by levente on 2018.02.27..
//

#ifndef SUMO_MESSAGE_H
#define SUMO_MESSAGE_H

#include <string>

// ===========================================================================
// class declarations
// ===========================================================================
class SUMOVehicle;

// ========================
// class definition
// ========================
class Message{
public:
    Message();
    Message(SUMOVehicle* sender, SUMOVehicle* receiver, void* content);
    ~Message();

    void* getContent();
    virtual void setContent(std::string* content);
    SUMOVehicle* getSender() const;
    SUMOVehicle* getReceiver();
    void setReceiver(SUMOVehicle* receiver);
    virtual void processMessage() = 0;


protected:
    SUMOVehicle* sender;
    SUMOVehicle* receiver;
private:
    void* content;
};


#endif //SUMO_MESSAGE_H
