#include <libsumo/Simulation.h>
#include "Judge.h"
#include <microsim/devices/Messages/helper.h>
#include <libsumo/VehicleType.h>
#include <math.h>
#include <microsim/MSVehicle.h>
#include <libsumo/Vehicle.h>

//-------CTors, Dtors, loaders---------
Judge::Judge(const std::string &path, const std::string &name) {
    initializeConflictMatrix(path);
    inside = 0;
    lastCheck = 0;
    recoverFromBadInside = 0;
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

ConflictClass::ConflictClass(const ConflictClass & rhs) {
    //std::cout <<"New cc: " <<rhs.nMembers << "; " << rhs.nPassed << std::endl;
    nMembers = rhs.nMembers - rhs.nPassed;
    nPassed = 0;
    for (int i=0; i<100; ++i) directions[i] = rhs.directions[i];
    canJoin = false;
    for (auto i = rhs.requests.begin(); i != rhs.requests.end(); ++i) {
        (*i)->myConflictClass = this;
        requests.push_back(*i);
    }
}

ConflictClass::ConflictClass() {
    canJoin = true;
    for (int i=0; i<100; ++i) directions[i] = false;
}


//------- Basic helper functions --------

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

inline double Judge::calculatePassFunction(int groupSize, int T, int requestArrived){
    return atan(-0.25*passTime*groupSize) + pow (1.1, groupSize) + pow(2, 0.15*(T-requestArrived));
}

double Judge::calculateSumOfPassFunctions(const std::vector<PassRequest*> *passes, int T) {
    double ans = 0;
    if (passes->size() == 0) return 0;
    for (auto i = passes->begin(); i!=passes->end(); ++i){
        //std::cout << (*i)->groupLeader->getID() << " pass: " << (*i)->tRequestArrived << std::endl;
        ans += calculatePassFunction((*i)->groupSize, T, (*i)->tRequestArrived);
    }
    return ans;
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

    //can join to the leader's CC:
    MSVehicle* myVech = static_cast<MSVehicle*>(request.groupLeader);
    std::pair< const MSVehicle *const, double > vech = myVech->getLeader(MAX_DISTANCE);
    MSDevice_Messenger* leader;
    if (vech.first == NULL) leader = NULL;
    else leader= getMessengerDeviceFromVehicle(getMessengerDeviceFromVehicle((SUMOVehicle*)vech.first)->getLeader());
    if (leader!=NULL) {
        int j = 0;
        if (answ != NULL) {
            for (auto i = answ->begin(); i != answ->end(); ++i) {
                if (*i == leader->actualCC) {
                    for (int k = 0; k < conflictClasses.size(); ++k) {
                        if (conflictClasses[k] == *i && conflictClasses[k]->canJoin) {
                            delete answ;
                            return k;
                        }
                    }
                }
            }
        }
    }

    //if cannot join leaders' CC:
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

void Judge::terminateCC(ConflictClass *cc) {
    //std::cout << "terminated" << std::endl;
    canChange = true;
    auto i = conflictClasses.begin();
    while ((*i)!=cc) ++i;
    conflictClasses.erase(i);
    for (int j=0; j<conflictClasses.size(); ++j){
        for (int k=0; k<conflictClasses[j]->requests.size(); ++k){
            //conflictClasses[j]->requests->at(k)->myConflictClass = NULL;
            libsumo::Vehicle::setColor(conflictClasses[j]->requests[k]->groupLeader->getID(), colors[j]);
            getMessengerDeviceFromVehicle(conflictClasses[j]->requests[k]->groupLeader)->debugJudgeSetColor(colors[j]);
        }
    }
    //std::remove(conflictClasses.begin(), conflictClasses.end(), cc);
    delete cc;
}



//--------- Logic functions ------------
void Judge::reportComing(SUMOVehicle *groupLeader, int groupSize, SUMOTime tNeeded, SUMOTime tWillArrive,
                         const std::string &inDirection, const std::string &outDirection) {
    if (getReqByLeader(groupLeader, conflictStore) != -1) return;
    PassRequest* newRequest = new PassRequest;
    newRequest->groupLeader = groupLeader;
    newRequest->groupSize = groupSize;
    newRequest->directon = new std::string(inDirection + "-" + outDirection);
    newRequest->tRequestArrived = libsumo::Simulation::getCurrentTime()/1000;
    conflictStore.push_back(newRequest);

    int conflictClass = getOptimalConflictClass(*newRequest);
    if (conflictClass == -1 || conflictClasses[conflictClass]->canJoin == false){
        ConflictClass* newCC = new ConflictClass();
        newCC->requests.push_back(newRequest);
        newCC->directions[getDirByStr(newRequest->directon)] = true;
        newCC->nMembers = 1;
        newCC->canJoin = true;
        newCC->nPassed = 0;
        conflictClasses.push_back(newCC);

        libsumo::Vehicle::setColor(groupLeader->getID(), colors[conflictClasses.size()-1]);
        getMessengerDeviceFromVehicle(groupLeader)->debugJudgeSetColor(colors[conflictClasses.size()-1]);
        getMessengerDeviceFromVehicle(groupLeader)->actualCC = newCC;

        newRequest->myConflictClass = newCC;
    } else {
        ConflictClass* cc = conflictClasses[conflictClass];
        cc->nMembers += 1;
        if (cc->nMembers > DEFAULT_CC_SIZE) cc->canJoin = false;
        cc->directions[getDirByStr(newRequest->directon)] = true;
        cc->requests.push_back(newRequest);
        newRequest->myConflictClass = cc;

        getMessengerDeviceFromVehicle(groupLeader)->actualCC = cc;
        libsumo::Vehicle::setColor(groupLeader->getID(), colors[conflictClass]);
        getMessengerDeviceFromVehicle(groupLeader)->debugJudgeSetColor(colors[conflictClass]);
    }
}

void Judge::iCrossed(SUMOVehicle *groupLeader) {
    //std::cout << groupLeader->getID() << " crossed at " << name << std::endl;
    int idx = getReqByLeader(groupLeader, conflictStore);
    if (idx == -1) {
        //std::cerr << "Bad request." << std::endl;
        return;
    }
    PassRequest* req = conflictStore[idx];
    auto deller = conflictStore.begin();
    while ((*deller)!=req) ++deller;
    conflictStore.erase(deller);

    ConflictClass* cc = req->myConflictClass;
    auto j = cc->requests.begin();
    for (j = cc->requests.begin(); j != cc->requests.end(); ++j) {
        if ((*j) == req) break;
    }
    cc->requests.erase(j);

    cc->nPassed += 1;
    if (cc->requests.size() == 0) terminateCC(cc);
    getMessengerDeviceFromVehicle(groupLeader)->actualCC = NULL;
    delete req;
}

bool Judge::canIPass(SUMOVehicle *groupLeader) {
   // if (libsumo::Simulation::getCurrentTime()/1000 - lastCheck > DEFAULT_RECHECK_TIME) recheck();
    recheck();
    if (actualConflictClass>=0 && conflictStore[getReqByLeader(groupLeader, conflictStore)]->myConflictClass == conflictClasses[actualConflictClass])
        //std::cout << groupLeader->getID() << " has permission to pass at: " << name << std::endl;
    return (actualConflictClass>=0 && conflictStore[getReqByLeader(groupLeader, conflictStore)]->myConflictClass == conflictClasses[actualConflictClass]);
}

void Judge::recheck() {
    lastCheck = libsumo::Simulation::getCurrentTime()/1000;
    ConflictClass* maxPos;
    double max = 0, akt;
    int j;
    int T = libsumo::Simulation::getCurrentTime();

    //std::cout << name <<": ";
    for (j=0; j < conflictClasses.size(); ++j) {
        akt = calculateSumOfPassFunctions(&conflictClasses[j]->requests, T/1000);
       // std::cout << akt <<", ";
        if (akt>max) {
            max = akt;
            maxPos = conflictClasses[j];
        }
    }
    //std::cout << std::endl;
    if (max>DEADLOCK_THRESHOLD && actualConflictClass!=-1 && T/1000-lastDeadlock > 10) {
        antiDeadLock();
        return;
    }

    if (changeNeededTo != NULL && inside<=0) {
        inside = 0;
        for (j=0; j<conflictClasses.size(); ++j) if (conflictClasses[j] == changeNeededTo) {
                changeNeededTo = NULL;
                actualConflictClass = j;
                for (auto ptr=conflictClasses.begin(); ptr!=conflictClasses.end(); ++ptr) {
                    resetCarsSpeed((*ptr)->requests);
                }
                return;
            }
    }

    if (actualConflictClass != -1 && maxPos != conflictClasses[actualConflictClass]) {
        conflictClasses[actualConflictClass]->canJoin = false;
    }

    if (actualConflictClass != -1) {
        for (j=0; j<conflictClasses.size() && conflictClasses.at(j)!=maxPos; ++j);
        if (j == conflictClasses.size()) {
            actualConflictClass = -1;
            return;
        }

        if (actualConflictClass != j) {
            changeNeededTo = maxPos;
            //std::cout << "actual CC has changed" << std::endl;
            return;
        }
    } else {
        //std::cout << "actual CC has changed" << std::endl;
        changeNeededTo = maxPos;
    }
}

void Judge::antiDeadLock() {
    //std::cout << "antideadlock" << std::endl;
    if (inside!=0 && recoverFromBadInside<50*passTime) {
      //  std::cerr << "There are cars in." << std::endl;
        ++recoverFromBadInside;
        return;
    }
    recoverFromBadInside = 0;
    inside = 0;
    lastDeadlock = libsumo::Simulation::getCurrentTime() / 1000;

    ConflictClass* cc = new ConflictClass(*conflictClasses[actualConflictClass]);
    ConflictClass* deletendo = conflictClasses[actualConflictClass];

    for (auto i = cc->requests.begin(); i != cc->requests.end(); ++i) {
        (*i)->tRequestArrived = libsumo::Simulation::getCurrentTime()/1000;
        (*i)->myConflictClass = cc;
        //cc->requests.push_back((*i));
        getMessengerDeviceFromVehicle((*i)->groupLeader)->actualCC = cc;
    }

    if (changeNeededTo == deletendo) changeNeededTo = NULL;

    /*int k = 0;
    for (auto i = conflictClasses.begin(); i!=conflictClasses.end(); ++i) {
        if ((*i) == deletendo || k == actualConflictClass) {
            std::cout << "erased" << std::endl;
            conflictClasses.erase(i);
            break;
        }
        ++k;
    }*/
    terminateCC(deletendo);
    //std::cout << "anti deadlock end." << std::endl;
    //delete deletendo;

    conflictClasses.push_back(cc);
    actualConflictClass = -1;
    /*for (auto i = conflictClasses.begin(); i != conflictClasses.end(); ++i) {
        resetCarsSpeed((*i)->requests);
    }*/
}

void Judge::reportCome() {
    ++inside;
}

void Judge::reportLeave(){
    --inside;
}