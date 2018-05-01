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
        counter[i] = -1;
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
    newRequest->tNeeded = groupSize * passTime;
    newRequest->tWillArrive = libsumo::Simulation::getCurrentTime()/1000 + comeInTime;
    //newRequest -> tNeeded = 100;
    //newRequest -> tWillArrive = 10;
    newRequest->directon = new std::string(inDirection + "-" + outDirection);
    newRequest->tGranted = -1;
    newRequest->tRequestArrived = libsumo::Simulation::getCurrentTime()/1000;
    conflictStore.push_back(newRequest);
    std::cout << groupLeader -> getID() << " : " << tWillArrive << "; " << tNeeded << std::endl;

    if (!canIPass(newRequest->groupLeader)){
        std::vector<PassRequest*> *r1, *r2;
        conflictStore.pop_back();
        r1 = &conflictStore;
        r2 = new std::vector<PassRequest*>();
        r2->push_back(newRequest);
        int t = searchEqualsInTime(r1, r2, libsumo::Simulation::getCurrentTime()/1000);
        int canPass = 0;
        if (t!=0) {
            state = 1;
            if (t>0) {
                if (r1->size() == 0) return;
                resetCounterToZero();
                int dir = getDirByStr(r1->at(0)->directon);
                for (int j=0; j<mtxSize; ++j){
                    std::vector<PassRequest*> *requests = getRequestsByDirection(r1, &routeName[j]);
                    if (requests->size() != 0) {
                        for (auto requ = requests->begin(); requ != requests->end(); ++requ){
                            canPass = howManyCanPass(*requ, t);
                            counter[j] += canPass;
                        }
                    }
                }
            } else {
                
            }
        }
    }

}

bool compareRequest(PassRequest *i, PassRequest *j){
    return i->tWillArrive < j->tWillArrive;
}

void Judge::resetCounterToZero() {
    for (int i=0; i<mtxSize; ++i){
        counter[i] = 0;
    }
}

std::vector<PassRequest*>* Judge::getRequestsByDirection(const std::vector<PassRequest *> *passRequests,
                                                         const std::string *direction) {
    std::vector<PassRequest*> *ans = new std::vector<PassRequest*>();
    for (auto i = passRequests->begin(); i != passRequests->end(); ++i) {
        if (*direction == *((*i)->directon)) ans->push_back(*i);
    }

    std::sort(ans->begin(), ans->end(), compareRequest);
    return ans;
}

inline int Judge::howManyCanPass(const PassRequest* request, int deadline){
    int szaml =(request->tWillArrive+(request->groupSize)*passTime) - deadline;
    return szaml>0 ? szaml / passTime : request->groupSize;
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

    conflictStore.push_back(newRequest);
}*/

int Judge::isThereConflict(SUMOVehicle *groupLeader, const std::vector<PassRequest *> &requests) {
    int lead = getReqByLeader(groupLeader, requests);
    if (lead == -1) return -1;
    int dir = getDirByStr(requests[lead]->directon);
    int otherDir;

    for (int i=0; i<requests.size(); ++i){
        otherDir = getDirByStr(requests[i]->directon);
        //std::cout << *conflictStore[i]->directon << " vs. " << *conflictStore[lead]->directon << std::endl;
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

//TODO: conflictStore come as a new list
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
    int idx = getReqByLeader(groupLeader, this->conflictStore);
    PassRequest* req = conflictStore[idx];
    auto i=conflictStore.begin();
    for (int j=0; j<idx; ++j) ++i;
    conflictStore.erase(i);
    for (i = conflictStore.begin(); i != conflictStore.end(); ++i) {
        std::remove((*i)->heldBy.begin(), (*i)->heldBy.end(), groupLeader);
        if ((*i)->heldBy.size() == 0) {
            getMessengerDeviceFromVehicle((*i)->groupLeader)->setVehicleSpeed((*i)->groupLeader->getEdge()->getSpeedLimit()*0.2);
        }
    }
    delete req->directon;
    delete req;
}

int Judge::getRemaining(SUMOVehicle *groupLeader) {
    /*for (auto i= conflictStore.begin(); i!=conflictStore.end(); ++i){
        if ((*i)->groupLeader == groupLeader) break;
    }*/

}

bool Judge::canIPass(SUMOVehicle *groupLeader) {
    int conflict = isThereConflict(groupLeader, this->conflictStore);
    //no conflict:
    if (conflict == -1) {
        std::cout << groupLeader->getID() << " can safely pass" << std::endl;
        conflictStore[getReqByLeader(groupLeader, this->conflictStore)]->tGranted = conflictStore[getReqByLeader(groupLeader, this->conflictStore)]->tWillArrive;
        return true;
    }
    else {
        //can be solved by accelerating
        if (!willMakeAnotherConflict(groupLeader, conflictStore[conflict]->groupLeader)){
            getMessengerDeviceFromVehicle(groupLeader)->setVehicleSpeed(groupLeader->getEdge()->getSpeedLimit());
            getMessengerDeviceFromVehicle(conflictStore[conflict]->groupLeader)->setVehicleSpeed(conflictStore[conflict]->groupLeader->getEdge()->getSpeedLimit()*0.2);
            conflictStore[getReqByLeader(groupLeader, this->conflictStore)]->tGranted = conflictStore[getReqByLeader(groupLeader, this->conflictStore)]->tWillArrive;

            std::cout << groupLeader->getID() << " will accelerate and " << conflictStore[conflict]->groupLeader->getID() << " will deccelerate." << std::endl;
            return true;
        } else if (!willMakeAnotherConflict(conflictStore[conflict]->groupLeader, groupLeader)) {
            getMessengerDeviceFromVehicle(groupLeader)->setVehicleSpeed(groupLeader->getEdge()->getSpeedLimit()*0.2);
            getMessengerDeviceFromVehicle(conflictStore[conflict]->groupLeader)->setVehicleSpeed(conflictStore[conflict]->groupLeader->getEdge()->getSpeedLimit());
            conflictStore[conflict]->tGranted = conflictStore[conflict]->tWillArrive;

            std::cout << conflictStore[conflict]->groupLeader->getID() << " will accelerate and " << groupLeader->getID() << " will deccelerate." << std::endl;
            return true;
        } else { /*//licit
            std::cout << groupLeader->getID() << " has to make an auction with "<< conflictStore[conflict]->groupLeader->getID() <<": ";
            int req = getReqByLeader(groupLeader, this->conflictStore);
            double w1 = atan(-0.25*conflictStore[req]->tNeeded) + pow(1.1, conflictStore[req]->groupSize) + pow(2, 0.15*(libsumo::Simulation::getCurrentTime()/1000 - conflictStore[req]->tRequestArrived));
            double w2 = atan(-0.25*conflictStore[conflict]->tNeeded) + pow(1.1, conflictStore[conflict]->groupSize) + pow(2, 0.15*(libsumo::Simulation::getCurrentTime()/1000 - conflictStore[conflict]->tRequestArrived));
            if (w1>w2) {
                getMessengerDeviceFromVehicle(conflictStore[conflict]->groupLeader)->setVehicleSpeed(0);
                //getMessengerDeviceFromVehicle(groupLeader)->setVehicleSpeed(groupLeader->getEdge()->getSpeedLimit()*0.6);
                conflictStore[conflict]->heldBy.push_back(groupLeader);
                std::cout << w1 << " > " << w2 << ", so can pass: " << groupLeader->getID() << std::endl;
                conflictStore[getReqByLeader(groupLeader, this->conflictStore)]->tGranted = conflictStore[getReqByLeader(groupLeader, this->conflictStore)]->tWillArrive;
                return true;
            } else {
                getMessengerDeviceFromVehicle(conflictStore[req]->groupLeader)->setVehicleSpeed(0);
                //getMessengerDeviceFromVehicle(conflictStore[conflict]->groupLeader)->setVehicleSpeed(groupLeader->getEdge()->getSpeedLimit()*0.6);
                conflictStore[req]->heldBy.push_back(groupLeader);
                std::cout << w2 << " > " << w1 << ", so can pass: " << conflictStore[conflict]->groupLeader->getID() << std::endl;*/
                return false;
            //}
        }
    }
}


inline double calculateTime(double s, double a, double v0, double v1){
    double t1 = (v1-v0)/a;
    double s1 = v0*t1 + a/2*t1*t1;
    return (s1 > s) ? (-v0+sqrt(v0*v0-2*a*s))/a : t1+(s-s1)/v1;
}

inline double Judge::calculatePassFunction(int groupSize, int T, int requestArrived){
    return atan(-0.25*passTime*groupSize) + pow (1.1, groupSize) + pow(2, 0.15*(T-requestArrived-comeInTime));
}

double Judge::calculateSumOfPassFunctions(const std::vector<PassRequest*> *passes, int T) {
    double ans = 0;
    for (auto i = passes->begin(); i!=passes->end(); ++i){
        ans += calculatePassFunction((*i)->groupSize, T, (*i)->tRequestArrived);
    }
    return ans;
}

inline int Judge::searchEqualsInTime(const std::vector<PassRequest*> *pass1, const std::vector<PassRequest*> *pass2, int simuTime) {
    int W1 = calculateSumOfPassFunctions(pass1, simuTime);
    int W2 = calculateSumOfPassFunctions(pass2, simuTime);
    int t = simuTime + 1;

    int oldW1, oldW2;
    do {
        oldW1 = W1;
        oldW2 = W2;

        W1 = calculateSumOfPassFunctions(pass1, t);
        W2 = calculateSumOfPassFunctions(pass2, t);

        ++t;
    } while ((t < simuTime + 100) &&
             ((oldW1 <= oldW2) && (W1 <= W2) ||
              (oldW1 >= oldW2) && (W1 >= W1)));

    //if W2 is bigger currently, its positive.
    return (t < simuTime + 100)? ((W1 <= W2)? t: (-1)*t) : 0;
}

bool Judge::willMakeAnotherConflict(SUMOVehicle *leader1, SUMOVehicle *leader2) {
    std::vector<PassRequest*> myRequests;
    PassRequest* needToDelete[2];
    int deleter=0;
    for (int i=0; i<conflictStore.size(); ++i){
        if (conflictStore[i]->groupLeader == leader1 || conflictStore[i]->groupLeader == leader2){
            PassRequest* modifiedRequest = new PassRequest;
            modifiedRequest->groupLeader = conflictStore[i]->groupLeader;
            modifiedRequest->tGranted = conflictStore[i]->tGranted;
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
            modifiedRequest->directon = conflictStore[i]->directon;
            modifiedRequest->tRequestArrived = conflictStore[i]->tRequestArrived;
            modifiedRequest->tNeeded = conflictStore[i]->tNeeded;

            myRequests.push_back(modifiedRequest);
            needToDelete[deleter++] = modifiedRequest;
        }
    }
    bool result = isThereConflict(leader1, myRequests) || isThereConflict(leader2, myRequests);
    delete needToDelete[0];
    delete needToDelete[1];
    return result;
}

