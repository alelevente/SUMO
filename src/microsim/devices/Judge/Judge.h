//
// Created by levente on 2018.04.05..
//

#ifndef SUMO_JUDGE_H
#define SUMO_JUDGE_H

#include <microsim/devices/Markers/BaseMarker.h>
#include "utils/vehicle/SUMOVehicle.h"
#include <microsim/MSVehicle.h>
//#include "microsim/devices/Markers/ExitMarker.h"
//#include "microsim/devices/Markers/EntryMarker.h"

#define DEFAULT_PASS_TIME 10
#define DEFAULT_COME_IN_TIME 10
#define DEFAULT_CC_SIZE 15
#define DEFAULT_RECHECK_TIME 1
#define DEADLOCK_THRESHOLD 50000

struct PassRequest;

struct ConflictClass{
    int nMembers = 0, nPassed = 0;
    bool directions[100];
    bool canJoin = true;
    std::vector<PassRequest*> requests;
    ConflictClass();
    ConflictClass(const ConflictClass& rhs);
};

struct PassRequest{
    SUMOVehicle* groupLeader;
    SUMOTime tRequestArrived;
    int groupSize;
    std::string* directon;
    ConflictClass* myConflictClass;
};



class Judge{
public:

    void reportComing(SUMOVehicle* groupLeader, int groupSize, SUMOTime tNeeded, SUMOTime tWillArrive, const std::string& inDirection, const std::string& outDirection);
    void iCrossed(SUMOVehicle* groupLeader);
    const std::string& getName();
    Judge(const std::string& path, const std::string& name);
    ~Judge();
    bool canIPass(SUMOVehicle* groupLeader);
    void reportCome();
    void reportLeave();

private:
    std::vector<PassRequest*> conflictStore;
    std::string routeName[300];
    int lastDeadlock = 0;
    int counter[300];
    bool conflictMatrix[300][300];
    bool canChange = true;
    ConflictClass* changeNeededTo  = NULL;
    int mtxSize;
    int inside = 0;
    std::string name;
    int passTime = DEFAULT_PASS_TIME, comeInTime = DEFAULT_COME_IN_TIME;
    int state = 0;
    int recoverFromBadInside = 0;
    libsumo::TraCIColor colors[10];


    void initializeConflictMatrix(const std::string& path);
    inline int getReqByLeader(SUMOVehicle *groupLeader, const std::vector<PassRequest *> requests);
    inline int getDirByStr(std::string *direction);
    inline double calculatePassFunction(int groupSize, int T, int requestArrived);
    inline double calculateSumOfPassFunctions(const std::vector<PassRequest*>* passes, int T);
    std::vector<PassRequest*>* getRequestsByDirection(const std::vector<PassRequest*>* passRequests, const std::string* direction);

    std::vector<ConflictClass*> conflictClasses;
    int actualConflictClass = -1;
    int lastCheck = 0;
    void recheck();
    void antiDeadLock();
    std::vector<ConflictClass*>* getPossibleConflictClasses(const PassRequest& request);
    int getOptimalConflictClass(const PassRequest& request);

    void terminateCC(ConflictClass *cc);
};

#endif //SUMO_JUDGE_H
