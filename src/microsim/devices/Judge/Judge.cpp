//
// Created by levente on 2018.04.05..
//
#include <libsumo/Simulation.h>
#include "Judge.h"
#include <microsim/devices/Messages/helper.h>
#include <libsumo/VehicleType.h>
#include <math.h>

Judge::Judge(const std::string &path, const std::string &name) {
    initializeConflictMatrix(path);
    this->name = name;
}

Judge::~Judge() {}

const std::string& Judge::getName() {
    return name;
}

void Judge::initializeConflictMatrix(const std::string &path) {
    std::ifstream conflictFile(path);
    int be;

    conflictFile >> mtxSize;
    for (int i=0; i<mtxSize; ++i){
        conflictFile >> routeName[i];
    }
    for (int i=0; i<mtxSize; ++i){
        for (int j=0; j<mtxSize; ++j){
            conflictFile >> be;
            conflictMatrix[i][j] = be;
            conflictMatrix[j][i] = be;
        }
    }
    conflictFile.close();
}

void Judge::reportComing(SUMOVehicle *groupLeader, int groupSize, SUMOTime tNeeded, SUMOTime tWillArrive,
                         const std::string &inDirection, const std::string &outDirection) {
    PassRequest* newRequest = new PassRequest;
    newRequest->groupLeader = groupLeader;
    newRequest->groupSize = groupSize;
    newRequest->tNeeded = tNeeded;
    newRequest->tWillArrive = tWillArrive;
    //newRequest -> tNeeded = 100;
    //newRequest -> tWillArrive = 10;
    newRequest->directon = new std::string(inDirection + "-" + outDirection);
    newRequest->tGranted = -1;
    newRequest->tRequestArrived = libsumo::Simulation::getCurrentTime()/1000;

    requests.push_back(newRequest);
    std::cout << groupLeader -> getID() << " : " << tWillArrive << "; " << tNeeded << std::endl;
}

/*void conflictTestMethod(){
    PassRequest* newRequest = new PassRequest;
    newRequest->groupLeader = ;
    newRequest->groupSize = groupSize;
    newRequest->tNeeded = tNeeded;
    newRequest->tWillArrive = tWillArrive;
    newRequest->directon = new std::string(inDirection + "-" + outDirection);
    newRequest->tGranted = -1;
    newRequest->tRequestArrived = libsumo::Simulation::getCurrentTime();

    requests.push_back(newRequest);
}*/

int Judge::isThereConflict(SUMOVehicle *groupLeader, const std::vector<PassRequest *> &requests) {
    int lead = getReqByLeader(groupLeader, requests);
    if (lead == -1) return -1;
    int dir = getDirByStr(requests[lead]->directon);
    int otherDir;

    for (int i=0; i<requests.size(); ++i){
        otherDir = getDirByStr(requests[i]->directon);
        //std::cout << *requests[i]->directon << " vs. " << *requests[lead]->directon << std::endl;
        if (conflictMatrix[dir][otherDir] != false) {
            //std::cout << "KONFLIKTUS!" << std::endl;
            if (timeConflict(requests[lead]->tWillArrive, requests[lead]->tNeeded, requests[i]->tWillArrive, requests[i]->tNeeded) ||
                    (requests[i]->tGranted!=-1 && timeConflict(requests[lead]->tWillArrive, requests[lead]->tNeeded, requests[i]->tGranted, requests[i]->tNeeded))) {
                std::cout << "Időkonfliktus!!!!" << std::endl;
                return i;
            }
        }
    }
    return -1;
}

//TODO: requests come as a new list
int Judge::getReqByLeader(SUMOVehicle *groupLeader, const std::vector<PassRequest *> requests) {
    int j=0;
    for (auto i=requests.begin(); i!=requests.end(); ++i){
        if ((*i)->groupLeader == groupLeader) return j;
        ++j;
    }
    return -1;
}

int Judge::getDirByStr(std::string *direction) {
    for (int i=0; i<mtxSize; ++i){
        if (routeName[i].compare(*direction)==0) return i;
    }
    return -1;
}

bool Judge::timeConflict(SUMOTime t1, SUMOTime d1, SUMOTime t2, SUMOTime d2) {
    return ((t1+d1 <= t2) || (t2+d2 <= t1))? false: true;
}

void Judge::iCrossed(SUMOVehicle *groupLeader) {
    int idx = getReqByLeader(groupLeader, this->requests);
    PassRequest* req = requests[idx];
    auto i=requests.begin();
    for (int j=0; j<idx; ++j) ++i;
    requests.erase(i);
    for (i = requests.begin(); i != requests.end(); ++i) {
        std::remove((*i)->heldBy.begin(), (*i)->heldBy.end(), groupLeader);
        if ((*i)->heldBy.size() == 0) {
            getMessengerDeviceFromVehicle((*i)->groupLeader)->setVehicleSpeed((*i)->groupLeader->getEdge()->getSpeedLimit()*0.2);
        }
    }
    delete req->directon;
    delete req;
}

bool Judge::canIPass(SUMOVehicle *groupLeader) {
    int conflict = isThereConflict(groupLeader, this->requests);
    //no conflict:
    if (conflict == -1) {
        std::cout << groupLeader->getID() << " can safely pass" << std::endl;
        requests[getReqByLeader(groupLeader, this->requests)]->tGranted = requests[getReqByLeader(groupLeader, this->requests)]->tWillArrive;
        return true;
    }
    else {
        //can be solved by accelerating
        if (!willMakeAnotherConflict(groupLeader, requests[conflict]->groupLeader)){
            getMessengerDeviceFromVehicle(groupLeader)->setVehicleSpeed(groupLeader->getEdge()->getSpeedLimit());
            getMessengerDeviceFromVehicle(requests[conflict]->groupLeader)->setVehicleSpeed(requests[conflict]->groupLeader->getEdge()->getSpeedLimit()*0.2);
            requests[getReqByLeader(groupLeader, this->requests)]->tGranted = requests[getReqByLeader(groupLeader, this->requests)]->tWillArrive;

            std::cout << groupLeader->getID() << " will accelerate and " << requests[conflict]->groupLeader->getID() << " will deccelerate." << std::endl;
            return true;
        } else if (!willMakeAnotherConflict(requests[conflict]->groupLeader, groupLeader)) {
            getMessengerDeviceFromVehicle(groupLeader)->setVehicleSpeed(groupLeader->getEdge()->getSpeedLimit()*0.2);
            getMessengerDeviceFromVehicle(requests[conflict]->groupLeader)->setVehicleSpeed(requests[conflict]->groupLeader->getEdge()->getSpeedLimit());
            requests[conflict]->tGranted = requests[conflict]->tWillArrive;

            std::cout << requests[conflict]->groupLeader->getID() << " will accelerate and " << groupLeader->getID() << " will deccelerate." << std::endl;
            return true;
        } else { //licit
            std::cout << groupLeader->getID() << " has to make an auction with "<< requests[conflict]->groupLeader->getID() <<": ";
            int req = getReqByLeader(groupLeader, this->requests);
            double w1 = atan(-0.25*requests[req]->tNeeded) + pow(1.1, requests[req]->groupSize) + pow(2, 0.15*(libsumo::Simulation::getCurrentTime()/1000 - requests[req]->tRequestArrived));
            double w2 = atan(-0.25*requests[conflict]->tNeeded) + pow(1.1, requests[conflict]->groupSize) + pow(2, 0.15*(libsumo::Simulation::getCurrentTime()/1000 - requests[conflict]->tRequestArrived));
            if (w1>w2) {
                getMessengerDeviceFromVehicle(requests[conflict]->groupLeader)->setVehicleSpeed(0);
                //getMessengerDeviceFromVehicle(groupLeader)->setVehicleSpeed(groupLeader->getEdge()->getSpeedLimit()*0.6);
                requests[conflict]->heldBy.push_back(groupLeader);
                std::cout << w1 << " > " << w2 << ", so can pass: " << groupLeader->getID() << std::endl;
                requests[getReqByLeader(groupLeader, this->requests)]->tGranted = requests[getReqByLeader(groupLeader, this->requests)]->tWillArrive;
                return true;
            } else {
                getMessengerDeviceFromVehicle(requests[req]->groupLeader)->setVehicleSpeed(0);
                //getMessengerDeviceFromVehicle(requests[conflict]->groupLeader)->setVehicleSpeed(groupLeader->getEdge()->getSpeedLimit()*0.6);
                requests[req]->heldBy.push_back(groupLeader);
                std::cout << w2 << " > " << w1 << ", so can pass: " << requests[conflict]->groupLeader->getID() << std::endl;
                return false;
            }
        }
    }
}

inline double calculateTime(double s, double a, double v0, double v1){
    double t1 = (v1-v0)/a;
    double s1 = v0*t1 + a/2*t1*t1;
    return (s1 > s) ? (-v0+sqrt(v0*v0-2*a*s))/a : t1+(s-s1)/v1;
}

bool Judge::willMakeAnotherConflict(SUMOVehicle *leader1, SUMOVehicle *leader2) {
    std::vector<PassRequest*> myRequests;
    PassRequest* needToDelete[2];
    int deleter=0;
    for (int i=0; i<requests.size(); ++i){
        if (requests[i]->groupLeader == leader1 || requests[i]->groupLeader == leader2){
            PassRequest* modifiedRequest = new PassRequest;
            modifiedRequest->groupLeader = requests[i]->groupLeader;
            modifiedRequest->tGranted = requests[i]->tGranted;
            double v1, v0 = modifiedRequest->groupLeader->getSpeed();
            double a, s = modifiedRequest->groupLeader->getEdge()->getLength() - modifiedRequest->groupLeader->getPositionOnLane();
            if (modifiedRequest->groupLeader == leader1) {
                a = libsumo::VehicleType::getAccel(leader1->getVehicleType().getID());
                v1 = leader1->getEdge()->getSpeedLimit();
            } else {
                a = (-1)*libsumo::VehicleType::getDecel(leader2->getVehicleType().getID());
                v1 = 0.2*leader2->getEdge()->getSpeedLimit();
            }
            modifiedRequest->tWillArrive = libsumo::Simulation::getCurrentTime()/1000 + calculateTime(s, a, v0, v1);
            modifiedRequest->directon = requests[i]->directon;
            modifiedRequest->tRequestArrived = requests[i]->tRequestArrived;
            modifiedRequest->tNeeded = requests[i]->tNeeded;

            myRequests.push_back(modifiedRequest);
            needToDelete[deleter++] = modifiedRequest;
        }
    }
    bool result = isThereConflict(leader1, myRequests) || isThereConflict(leader2, myRequests);
    delete needToDelete[0];
    delete needToDelete[1];
    return result;
}

