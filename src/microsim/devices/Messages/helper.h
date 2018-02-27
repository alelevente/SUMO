//
// Created by levente on 2018.02.27..
//

#ifndef SUMO_HELPER_H
#define SUMO_HELPER_H

#ifndef debug
#define debug
#endif

class SUMOVehicle;

class MessagesHelper{
#ifdef debug
public:
    MessagesHelper();
    ~MessagesHelper();

    SUMOVehicle *partner;
#endif
};

#endif //SUMO_HELPER_H
