//
// Created by levente on 2018.04.05..
//

#ifndef SUMO_JUDGE_H
#define SUMO_JUDGE_H

#include <microsim/devices/Markers/BaseMarker.h>
#include "utils/vehicle/SUMOVehicle.h"
//#include "microsim/devices/Markers/ExitMarker.h"
//#include "microsim/devices/Markers/EntryMarker.h"

struct PassRequest{
    SUMOVehicle* groupLeader;
    SUMOTime tRequestArrived, tNeeded, tGranted, tWillArrive;
    int groupSize;
    std::string* directon;
    std::vector<SUMOVehicle*> heldBy;
};

class Judge{
public:
    bool canIPass(SUMOVehicle* groupLeader);
    //int getRemaining
    void reportComing(SUMOVehicle* groupLeader, int groupSize, SUMOTime tNeeded, SUMOTime tWillArrive, const std::string& inDirection, const std::string& outDirection);
    void iCrossed(SUMOVehicle* groupLeader);
    const std::string& getName();
    Judge(const std::string& path, const std::string& name);
    ~Judge();

private:
    std::vector<PassRequest*> requests;
    std::string routeName[300];
    bool conflictMatrix[300][300];
    int mtxSize;
    std::string name;

    void initializeConflictMatrix(const std::string& path);
    int isThereConflict(SUMOVehicle *groupLeader, const std::vector<PassRequest *> &requests);
    inline int getReqByLeader(SUMOVehicle *groupLeader, const std::vector<PassRequest *> requests);
    inline int getDirByStr(std::string *direction);
    inline bool timeConflict(SUMOTime t1, SUMOTime d1, SUMOTime t2, SUMOTime d2);
    bool willMakeAnotherConflict(SUMOVehicle *leader1, SUMOVehicle *leader2);
};

#endif //SUMO_JUDGE_H
