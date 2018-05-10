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

#define DEFAULT_PASS_TIME 3
#define DEFAULT_COME_IN_TIME 10
#define DEFAULT_CC_SIZE 15
#define DEFAULT_RECHECK_TIME 10
#define DEADLOCK_THRESHOLD 20000

struct PassRequest;

struct ConflictClass{
    int nMembers = 0, nPassed = 0;
    bool directions[100];
    bool canJoin = true;
    std::vector<PassRequest*> *requests;
    ConflictClass();
    ConflictClass(const ConflictClass& rhs);
};

struct PassRequest{
    SUMOVehicle* groupLeader;
    SUMOTime tRequestArrived, tNeeded, tGranted, tWillArrive;
    int groupSize;
    std::string* directon;
    std::vector<SUMOVehicle*> heldBy;
    ConflictClass* myConflictClass;
};



class Judge{
public:

    int getRemaining(SUMOVehicle* groupLeader);
    void reportComing(SUMOVehicle* groupLeader, int groupSize, SUMOTime tNeeded, SUMOTime tWillArrive, const std::string& inDirection, const std::string& outDirection);
    void iCrossed(SUMOVehicle* groupLeader);
    const std::string& getName();
    Judge(const std::string& path, const std::string& name);
    ~Judge();
    bool canIPass(SUMOVehicle* groupLeader);

private:
    std::vector<PassRequest*> conflictStore;
    std::string routeName[300];
    int counter[300];
    bool conflictMatrix[300][300];
    int mtxSize;
    std::string name;
    int passTime = DEFAULT_PASS_TIME, comeInTime = DEFAULT_COME_IN_TIME;
    int state = 0;
    libsumo::TraCIColor colors[10];


    void initializeConflictMatrix(const std::string& path);
    int isThereConflict(SUMOVehicle *groupLeader, const std::vector<PassRequest *> &requests);
    inline int getReqByLeader(SUMOVehicle *groupLeader, const std::vector<PassRequest *> requests);
    inline int getDirByStr(std::string *direction);
    inline double calculatePassFunction(int groupSize, int T, int requestArrived);
    inline int searchEqualsInTime(const std::vector<PassRequest*>* pass1, const std::vector<PassRequest*>* pass2, int simuTime);
    inline double calculateSumOfPassFunctions(const std::vector<PassRequest*>* passes, int T);
    inline bool timeConflict(SUMOTime t1, SUMOTime d1, SUMOTime t2, SUMOTime d2);
    inline int howManyCanPass(const PassRequest* request, int deadline);
    std::vector<PassRequest*>* getRequestsByDirection(const std::vector<PassRequest*>* passRequests, const std::string* direction);
    bool willMakeAnotherConflict(SUMOVehicle *leader1, SUMOVehicle *leader2);
    inline void resetCounterToZero();

    std::vector<ConflictClass*> conflictClasses;
    int actualConflictClass = -1;
    int lastCheck = 0;
    void recheck();
    void antiDeadLock();
    std::vector<ConflictClass*>* getPossibleConflictClasses(const PassRequest& request);
    int getOptimalConflictClass(const PassRequest& request);

};

#endif //SUMO_JUDGE_H
