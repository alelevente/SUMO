//
// Created by levente on 2018.04.05..
//

#ifndef SUMO_JUDGE_H
#define SUMO_JUDGE_H

#include <utils/vehicle/SUMOVehicle.h>
#include <microsim/devices/Markers/ExitMarker.h>
#include <microsim/devices/Markers/EntryMarker.h>

struct PassRequest{
    SUMOVehicle* groupLeader;
    SUMOTime tRequestArrived, tNeeded, tGranted, tWillArrive;
    int groupSize;
    std::string directon;
};

class Judge{
public:
    bool canIPass(SUMOVehicle* groupLeader);
    void reportComing(SUMOVehicle* groupLeader, int groupSize, SUMOTime tNeeded, SUMOTime tWillArrive, const std::string& inDirection, const std::string& outDirection);
    void iCrossed(SUMOVehicle* groupLeader);
    Judge(const std::string& path);
    ~Judge();
private:
    std::vector<PassRequest*> requests;
    std::string routeName[300];
    bool conflictMatrix[300][300];
    int mtxSize;

    void initializeConflictMatrix(const std::string& path);
    int isThereConflict(SUMOVehicle *groupLeader, const std::vector<PassRequest *> &requests);
    inline int getReqByLeader(SUMOVehicle* groupLeader);
    inline int getDirByStr(const std::string &direction);
    inline bool timeConflict(SUMOTime t1, SUMOTime d1, SUMOTime t2, SUMOTime d2);
    bool willMakeAnotherConflict(SUMOVehicle *leader1, SUMOVehicle *leader2);
};

#endif //SUMO_JUDGE_H
