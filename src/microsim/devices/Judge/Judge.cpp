//
// Created by levente on 2018.04.05..
//
#include <libsumo/Simulation.h>
#include "Judge.h"
#include <microsim/devices/Messages/helper.h>
#include <libsumo/VehicleType.h>
#include <math.h>
#include <microsim/MSVehicle.h>
#include <libsumo/Vehicle.h>

Judge::Judge(const std::string &path, const std::string &name) {
    initializeConflictMatrix(path);
    inside = 0;
    this->name = name;


    colors[0].a = 255;
    colors[0].r = 255;
    colors[0].g = 0;
    colors[0].b = 0;

    colors[1].a = 255;
    colors[1].r = 0;
    colors[1].g = 255;
    colors[1].b = 0;

    colors[2].a = 255;
    colors[2].r = 0;
    colors[2].g = 0;
    colors[2].b = 255;

    colors[3].a = 255;
    colors[3].r = 255;
    colors[3].g = 255;
    colors[3].b = 0;

    colors[4].a = 255;
    colors[4].r = 255;
    colors[4].g = 0;
    colors[4].b = 255;

    colors[5].a = 255;
    colors[5].r = 255;
    colors[5].g = 255;
    colors[5].b = 255;

    colors[6].a = 255;
    colors[6].r = 128;
    colors[6].g = 128;
    colors[6].b = 255;

    colors[7].a = 255;
    colors[7].r = 255;
    colors[7].g = 128;
    colors[7].b = 128;
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
    //todo: check if already exists
   /* if (groupLeader->getID() == "carflow6.11") {
        std::cout << "MGVan\n";
    }*/
    if (getReqByLeader(groupLeader, conflictStore) != -1) return;
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
    //std::cout << groupLeader -> getID() << " : " << tWillArrive << "; " << tNeeded << std::endl;

    MSVehicle* myVech = static_cast<MSVehicle*>(groupLeader);
    std::pair< const MSVehicle *const, double > vech = myVech->getLeader(MAX_DISTANCE);
    MSDevice_Messenger* leader;
    if (vech.first == NULL) leader = NULL;
    else leader= getMessengerDeviceFromVehicle(getMessengerDeviceFromVehicle((SUMOVehicle*)vech.first)->getLeader());

    int conflictClass = -1;
    if (leader!=NULL) {
        std::vector<ConflictClass*> *confs = getPossibleConflictClasses(*newRequest);
        int j=0;
        if (confs!=NULL) {
            for (auto i = confs->begin(); i != confs->end(); ++i) {
                if (*i == leader->actualCC) {
                    for (int k=0; k<conflictClasses.size(); ++k) {
                        if (conflictClasses[k] == *i) conflictClass = k;
                    }
                    break;
                }
                ++j;
            }
            if (j>=confs->size()) conflictClass = getOptimalConflictClass(*newRequest);
            delete confs;
        }
    } else conflictClass = getOptimalConflictClass(*newRequest);
    if (conflictClass == -1 || (conflictClasses.at(conflictClass)->canJoin==false)) {
        ConflictClass* newCC = new ConflictClass();
        newCC->requests = new std::vector<PassRequest*>();
        newCC->requests->push_back(newRequest);
        newCC->directions[getDirByStr(newRequest->directon)] = true;
        newCC->nMembers = 1;
        newCC->canJoin = true;
        newCC->nPassed = 0;
        //std::cout << groupLeader -> getID() << " has to make a new CC: " << conflictClasses.size() <<std::endl;
        conflictClasses.push_back(newCC);
        getMessengerDeviceFromVehicle(groupLeader)->actualCC = newCC;

        libsumo::Vehicle::setColor(groupLeader->getID(), colors[conflictClasses.size()-1]);
        getMessengerDeviceFromVehicle(groupLeader)->debugJudgeSetColor(colors[conflictClasses.size()-1]);

        newRequest->myConflictClass = newCC;
    } else {
        ConflictClass* cc = conflictClasses.at(conflictClass);
        cc->nMembers += 1;
        if (cc->nMembers > DEFAULT_CC_SIZE) cc->canJoin = false;
        cc->directions[getDirByStr(newRequest->directon)] = true;
        cc->requests->push_back(newRequest);
        newRequest->myConflictClass = cc;
        getMessengerDeviceFromVehicle(groupLeader)->actualCC = conflictClasses[conflictClass];
        libsumo::Vehicle::setColor(groupLeader->getID(), colors[conflictClass]);
        getMessengerDeviceFromVehicle(groupLeader)->debugJudgeSetColor(colors[conflictClass]);
        //std::cout << groupLeader -> getID() << " can join a CC: " << conflictClass <<std::endl;
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
                //std::cout << "Időkonfliktus!!!!" << std::endl;
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
    if (crossFlag) {
        canChange = true;
        crossFlag = false;
    }
    //std::cout << groupLeader->getID() << ": we crossed." << std::endl;

    int idx = getReqByLeader(groupLeader, this->conflictStore);
   //if (idx==-1) return; //why happens?
    PassRequest* req = conflictStore[idx];
    auto i=conflictStore.begin();
    for (int j=0; j<idx; ++j) ++i;
    conflictStore.erase(i);

    ConflictClass* cc = req->myConflictClass;
    if (cc != NULL && cc->requests != NULL) {
        auto j = cc->requests->begin();
        //std::cout << "Conflict class: ";
        for (int i = 0; i < cc->requests->size(); ++i) {
            if ((*j) == req) cc->requests->erase(j);
            //std::cout << (*j)->groupLeader->getID() << ", ";
            ++j;
        }
        //std::cout << std::endl;
        //std::cout << "npassed: " << cc->nPassed << " nmembers: " << cc->nMembers << std::endl;
        //cc->requests->erase(j);
        //std::remove(cc->requests->begin(), cc->requests->end(), req);
        cc->nPassed += 1;
        if (cc->requests->size()==0) terminateCC(cc);
        getMessengerDeviceFromVehicle(groupLeader)->actualCC = NULL;
    }

    /*for (i = conflictStore.begin(); i != conflictStore.end(); ++i) {
        std::remove((*i)->heldBy.begin(), (*i)->heldBy.end(), groupLeader);
        if ((*i)->heldBy.size() == 0) {
            getMessengerDeviceFromVehicle((*i)->groupLeader)->setVehicleSpeed((*i)->groupLeader->getEdge()->getSpeedLimit()*0.2);
        }
    }*/
    delete req->directon;
    delete req;
}

bool Judge::canIPass(SUMOVehicle *groupLeader) {
    /*int conflict = isThereConflict(groupLeader, this->conflictStore);
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
        } else { //licit
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
                //return false;
            //}
     //   }
   // }
    if (libsumo::Simulation::getCurrentTime()/1000 - lastCheck > DEFAULT_RECHECK_TIME) recheck();
    return (actualConflictClass>=0 && conflictStore[getReqByLeader(groupLeader, conflictStore)]->myConflictClass == conflictClasses.at(actualConflictClass));
}


inline double calculateTime(double s, double a, double v0, double v1){
    double t1 = (v1-v0)/a;
    double s1 = v0*t1 + a/2*t1*t1;
    return (s1 > s) ? (-v0+sqrt(v0*v0-2*a*s))/a : t1+(s-s1)/v1;
}

inline double Judge::calculatePassFunction(int groupSize, int T, int requestArrived){
    return atan(-0.25*passTime*groupSize) + pow (1.1, groupSize) + pow(2, 0.15*(T-requestArrived));
}

double Judge::calculateSumOfPassFunctions(const std::vector<PassRequest*> *passes, int T) {
    double ans = 0;
    if (passes->size() == 0) return 0;
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

std::vector<ConflictClass*>* Judge::getPossibleConflictClasses(const PassRequest &request) {
    int j, dir = getDirByStr(request.directon);
    bool ok;
    //std::cout << request.groupLeader->getID() <<" can join CCs: ";
    std::vector<ConflictClass*>* answ = new std::vector<ConflictClass*>();
    for (auto i = conflictClasses.begin(); i != conflictClasses.end(); ++i){
        if ((*i)->canJoin == false) continue;
        ok = true;
        for (j=0; j<mtxSize; ++j){
            if ((*i)->directions[j] == 1 && conflictMatrix[j][dir]) ok = false;
        }
        if (ok) {
           // std::cout << (*i)->requests->at(0)->groupLeader->getID() <<" ";
            answ->push_back(*i);
        }
    }
    //std::cout << std::endl;
    if (answ->size()!=0) return answ;
    else {
        delete answ;
        return NULL;
    }
}

int Judge::getOptimalConflictClass(const PassRequest &request) {
    std::vector<ConflictClass*>* answ = getPossibleConflictClasses(request);
    int max = 0, pos = 0, akt, aktPos = 0;
    bool eqAct = false;
    if (answ != NULL){
        for (auto i = answ->begin(); i != answ->end(); ++i){
            akt = 0;
            for (int j = 0; j < mtxSize; ++j){
                if ((*i)->directions[j] == 0) akt += mtxSize;
                else for (int k = 0; k < mtxSize; ++k) {
                        if (conflictMatrix[j][k] == 0) ++akt;
                    }
            }
            if (akt > max) {
                max = akt;
                pos = aktPos;
            }
            if (actualConflictClass!=-1 && (*i) == conflictClasses[actualConflictClass]) eqAct = conflictClasses[actualConflictClass]->canJoin;
            ++aktPos;
        }
        aktPos = 0;
        for (auto i = conflictClasses.begin(); i != conflictClasses.end(); ++i){
            if ((*i) == answ->at(pos)) {
                delete answ;
                return eqAct?actualConflictClass:aktPos;
            }
            ++aktPos;
        }
    } else {
        delete answ;
        return -1;
    }
}

void resetCarsSpeed(const std::vector<PassRequest*>& passRequests){
    for (auto i = passRequests.begin(); i != passRequests.end(); ++i){
        getMessengerDeviceFromVehicle((*i)->groupLeader)->setVehicleSpeed(-1);//(*i)->groupLeader->getEdge()->getSpeedLimit());
    }
}

void Judge::recheck() {
    std::cout << "Rechecking... "<< inside << std::endl;
    int T = libsumo::Simulation::getCurrentTime();
    canChange = inside == 0;
    lastCheck = T/1000;
    ConflictClass* maxPos;
    double max =0, akt;
    int j;
    std::cout << name << " results: ";
    for (j=0; j < conflictClasses.size(); ++j){
        akt = calculateSumOfPassFunctions(conflictClasses.at(j)->requests, T/1000);
        std::cout << akt << " ";
        if (akt>max) {
            max = akt;
            maxPos = conflictClasses.at(j);
        }
    }
    std::cout << std::endl;
    if (max>DEADLOCK_THRESHOLD && actualConflictClass != -1) {
        antiDeadLock();
        return;
    }

    if (actualConflictClass != -1 && maxPos != conflictClasses[actualConflictClass]) {
        conflictClasses[actualConflictClass]->canJoin = false;
    }

    if (actualConflictClass!=-1) {
        //conflictClasses.at(actualConflictClass)->canJoin = false;
        //std::cout << "rechecked: "<< conflictClasses[actualConflictClass]->nMembers <<"; " << conflictClasses[actualConflictClass]->nPassed << std::endl;
        //std::cout << conflictClasses[actualConflictClass]->requests->size() << std::endl;
        /*for (auto i = conflictClasses[actualConflictClass]->requests->begin(); i!=conflictClasses[actualConflictClass]->requests->end(); ++i){
            std::cout << (*i)->groupLeader->getID() << ", ";
        }*/
        //std::cout << std::endl;
        /*if (conflictClasses.at(actualConflictClass)->nMembers-1 != conflictClasses.at(actualConflictClass)->nPassed) {
            lastCheck = 0;
        } else {*/

            for (j=0; j<conflictClasses.size() && conflictClasses.at(j)!=maxPos; ++j);
            if (j == conflictClasses.size()) {
                actualConflictClass = -1;
                return;
            }
            if (canChange) {
                actualConflictClass = j;
                canChange = false;
                std::cout << "Actual CC has changed, currently: " << actualConflictClass << std::endl;
                for (int j=0; j<conflictClasses.size(); ++j) {
                    resetCarsSpeed(*conflictClasses[j]->requests);
                }
            }
        //}
    } else {
        for (j=0; conflictClasses.at(j)!=maxPos; ++j);
        if (canChange) {
            actualConflictClass = j;
            canChange = false;

            //std::cout << "Actual CC has changed, currently: " << actualConflictClass << std::endl;
            resetCarsSpeed(*conflictClasses[actualConflictClass]->requests);
        }
    }
}

void Judge::antiDeadLock() {
    //std::cout << "Antideadlock." << std::endl;
    crossFlag = true;
    ConflictClass* cc = new ConflictClass(*conflictClasses[actualConflictClass]);
    delete  conflictClasses[actualConflictClass]->requests;
    delete conflictClasses[actualConflictClass];
    conflictClasses[actualConflictClass] = cc;
    for (auto i = cc->requests->begin(); i != cc->requests->end(); ++i) {
        (*i)->tRequestArrived = libsumo::Simulation::getCurrentTime()/1000;
        (*i)->myConflictClass = cc;
    }
    actualConflictClass = -1;
}

ConflictClass::ConflictClass(const ConflictClass & rhs) {
    //std::cout <<"New cc: " <<rhs.nMembers << "; " << rhs.nPassed << std::endl;
    nMembers = rhs.nMembers - rhs.nPassed;
    nPassed = 0;
    for (int i=0; i<100; ++i) directions[i] = rhs.directions[i];
    canJoin = false;
    requests = new std::vector<PassRequest*>();
    for (auto i = rhs.requests->begin(); i != rhs.requests->end(); ++i) {
        (*i)->myConflictClass = this;
        requests->push_back(*i);
    }
}

ConflictClass::ConflictClass() {
    canJoin = true;
    for (int i=0; i<100; ++i) directions[i] = false;
}

void Judge::terminateCC(ConflictClass *cc) {
    canChange = true;
    auto i = conflictClasses.begin();
    while (*i!=cc) ++i;
    conflictClasses.erase(i);
    delete cc->requests;
    cc->requests = NULL;
    for (int j=0; j<conflictClasses.size(); ++j){
        for (int k=0; k<conflictClasses[j]->requests->size(); ++k){
            //conflictClasses[j]->requests->at(k)->myConflictClass = NULL;
            libsumo::Vehicle::setColor(conflictClasses[j]->requests->at(k)->groupLeader->getID(), colors[j]);
            getMessengerDeviceFromVehicle(conflictClasses[j]->requests->at(k)->groupLeader)->debugJudgeSetColor(colors[j]);
        }
    }
    //std::remove(conflictClasses.begin(), conflictClasses.end(), cc);
    delete cc;
}

void Judge::reportCome() {
    ++inside;
}

void Judge::reportLeave(){
    --inside;
}