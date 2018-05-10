//
// Created by levente on 2018.03.11..
//

#include <utils/vehicle/SUMOVehicle.h>
#include <microsim/devices/Messages/helper.h>
#include "EntryMarker.h"
#include "MarkerSystem.h"

void* EntryMarker::onEnter(SUMOVehicle *who) {
    //std::vector<ExitMarker*> *answer = new std::vector<ExitMarker*> (exitMarkers);
    EntryMarkerAnswer* answer = new EntryMarkerAnswer();
    answer->exitMarkers = &exitMarkers;
    answer->judge = myJudge;
    return answer;
}

void* EntryMarker::onExit(SUMOVehicle *who) {
    return NULL;
}

Judge* findJudgeByName(const std::string& name, const std::map<std::string, BaseMarker*>& markerMap){
    for (auto i=markerMap.begin(); i!=markerMap.end(); ++i){
        if ((*i).first.compare(6,5,"Entry") == 0) {
            if (((EntryMarker*)(*i).second)->getJudge()->getName().compare(name)==0)
                return ((EntryMarker*)(*i).second)->getJudge();
        }
    }
    return NULL;
}

EntryMarker::EntryMarker(std::string markerID, MSEdge *position, const std::map<std::string, BaseMarker*>& markerMap):
    BaseMarker(markerID, position)
{
    std::string junctionName = getJunctionName(markerID);
    Judge* judge = findJudgeByName(junctionName, markerMap);
    if (judge == NULL) {
        myJudge = new Judge("/home/levente/Egyetem/6_felev/onlab/judges/" + junctionName + ".confmtx", junctionName);
    } else myJudge = judge;
}

EntryMarker::~EntryMarker() {
    delete myJudge;
}

Judge* EntryMarker::getJudge(){
    return myJudge;
}

