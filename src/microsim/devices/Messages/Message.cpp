//
// Created by levente on 2018.02.27..
//

#include "Message.h"
#include <string>

Message::Message(SUMOVehicle *sender, SUMOVehicle *receiver, void* content):
    sender(sender),
    receiver(receiver),
    content(content)
{
}

Message::~Message() {}

void* Message::getContent() {
    return content;
}

void Message::setContent(std::string* content) {
    this->content = content;
}

SUMOVehicle* Message::getReceiver() {
    return receiver;
}

void Message::setReceiver(SUMOVehicle *receiver) {
    this->receiver = receiver;
}

SUMOVehicle* Message::getSender() const {
    return sender;
}