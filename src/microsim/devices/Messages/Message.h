//
// Created by levente on 2018.02.27..
//

#ifndef SUMO_MESSAGE_H
#define SUMO_MESSAGE_H

// ===========================================================================
// class declarations
// ===========================================================================
class SUMOVehicle;

// ========================
// class definition
// ========================
class Message{
public:
    Message(SUMOVehicle* sender, SUMOVehicle* receiver, void* content);
    ~Message();

    void* getContent();
    void setContent(void* content);
    SUMOVehicle* getSender() const;
    SUMOVehicle* getReceiver();
    void setReceiver(SUMOVehicle* receiver);


private:
    SUMOVehicle* sender;
    SUMOVehicle* receiver;
    void* content;
};


#endif //SUMO_MESSAGE_H
