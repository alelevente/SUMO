//
// Created by levente on 2018.04.05..
//
#include <libsumo/Simulation.h>
#include "Judge.h
#include <microsim/devices/Messages/helper.h>
#include <libsumo/VehicleType.h>
#include <math.h>

Judge::Judge(const std::string &path) {
    initializeConflictMatrix(path);
}

Judge::~Judge() {}

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
    newRequest->directon = inDirection + " - " + outDirection;
    newRequest->tGranted = -1;
    newRequest->tRequestArrived = libsumo::Simulation::getCurrentTime();

    requests.push_back(newRequest);
}

int Judge::isThereConflict(SUMOVehicle *groupLeader, const std::vector<PassRequest *> &requests) {
    int lead = getReqByLeader(groupLeader);
    int dir = getDirByStr(requests[lead]->directon);
    int otherDir;

    for (int i=0; i<requests.size(); ++i){
        otherDir = getDirByStr(requests[i]->directon);
        if (conflictMatrix[dir][otherDir] != 0) {
            if (timeConflict(requests[lead]->tWillArrive, requests[lead]->tNeeded, requests[otherDir]->tWillArrive, requests[otherDir]->tNeeded) ||
                    (requests[otherDir]->tGranted!=-1 && timeConflict(requests[lead]->tWillArrive, requests[lead]->tNeeded, requests[otherDir]->tGranted, requests[otherDir]->tNeeded)))
                return otherDir;
        }
    }
    return -1;
}

int Judge::getReqByLeader(SUMOVehicle *groupLeader) {
    int j=0;
    for (auto i=requests.begin(); i!=requests.end(); ++i){
        ++j;
        if ((*i)->groupLeader == groupLeader) return j;
    }
    return -1;
}

int Judge::getDirByStr(const std::string &direction) {
    for (int i=0; i<mtxSize; ++i){
        if (routeName[i] == direction) return i;
    }
    return -1;
}

bool Judge::timeConflict(SUMOTime t1, SUMOTime d1, SUMOTime t2, SUMOTime d2) {
    return (t1+d1 <= t2) || (t2+d2 <= t1)? true: false;
}

void Judge::iCrossed(SUMOVehicle *groupLeader) {
    int idx = getReqByLeader(groupLeader);
    PassRequest* req = requests[idx];
    auto i=requests.begin();
    for (int j=0; j<idx; ++j) ++i;
    requests.erase(i);
    delete req;
}

bool Judge::canIPass(SUMOVehicle *groupLeader) {
    int conflict = isThereConflict(groupLeader, this->requests);
    //no conflict:
    if (conflict==0) return true;
    else {
        //can be solved by accelerating
        if (!willMakeAnotherConflict(groupLeader, requests[conflict]->groupLeader)){
            getMessengerDeviceFromVehicle(groupLeader)->setVehicleSpeed(groupLeader->getEdge()->getSpeedLimit());
            getMessengerDeviceFromVehicle(requests[conflict]->groupLeader)->setVehicleSpeed(requests[conflict]->groupLeader->getEdge()->getSpeedLimit()*0.2);
            return true;
        } else if (!willMakeAnotherConflict(requests[conflict]->groupLeader, groupLeader)) {
            getMessengerDeviceFromVehicle(groupLeader)->setVehicleSpeed(groupLeader->getEdge()->getSpeedLimit()*0.2);
            getMessengerDeviceFromVehicle(requests[conflict]->groupLeader)->setVehicleSpeed(requests[conflict]->groupLeader->getEdge()->getSpeedLimit());
            return true;
        } else { //licit
            int req = getReqByLeader(groupLeader);
            double w1 = atan(-0.25*requests[req]->tNeeded) + pow(1.1, requests[req]->groupSize) + pow(2, 0.15*(libsumo::Simulation::getCurrentTime() - requests[req]->tRequestArrived));
            double w2 = atan(-0.25*requests[conflict]->tNeeded) + pow(1.1, requests[conflict]->groupSize) + pow(2, 0.15*(libsumo::Simulation::getCurrentTime() - requests[conflict]->tRequestArrived));
            if (w1>w2) {
                getMessengerDeviceFromVehicle(requests[conflict]->groupLeader->)->setVehicleSpeed(0);
                return true;
            } else {
                getMessengerDeviceFromVehicle(requests[req]->groupLeader->)->setVehicleSpeed(0);
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
            modifiedRequest->tWillArrive = libsumo::Simulation::getCurrentTime() + calculateTime(s, a, v0, v1);
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

