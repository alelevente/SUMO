//
// Created by levente on 2018.03.10..
//

#include "MarkerSystem.h"
#include "EntryMarker.h"
#include "ExitMarker.h"
#include <microsim/MSEdgeControl.h>

//MarkerSystem::singleton = NULL;

static inline bool MarkerSystem::isMarkerID(const std::string& ID){
    return ID.compare(0,6,"marker") == 0;
}

MarkerSystem::MarkerSystem(){
    //std::vector<std::string> edgeNames = MSNet::getInstance()->getEdgeControl().getEdgeNames();
    std::vector<MSEdge*> edges = MSNet::getInstance()->getEdgeControl().getEdges();
    for (auto i = edges.begin(); i != edges.end(); ++i){
        //if marker
        if (isMarkerID((*i)->getID()){
            BaseMarker* newMarker = NULL;
            if ((*i)->getID().compare(6, 5, "Entry") == 0) {
                newMarker = new EntryMarker((*i)->getID(), (*i));

            } else if ((*i)->getID().compare(6,4,"Exit") == 0) {
                newMarker = new ExitMarker((*i)->getID(), (*i));
            }
            markerMap.insert(std::make_pair(*i, newMarker));
        }
    }

    for (auto i=markerMap.begin(); i!=markerMap.end(); ++i){
        if ((*i).first.compare(6,4,"Entry") == 0) {
            EntryMarker* entryMarker = static_cast<EntryMarker*>((*i).second);
            entryMarker->exitMarkers = findExitMarkerstoEntry((*i).first);
        }
    }
}

MarkerSystem::~MarkerSystem() {
    for (auto i = markerMap.begin(); i != markerMap.end(); ++i){
        delete (*i).second;
    }
}

BaseMarker* MarkerSystem::findMarkerByID(const std::string &ID) {
    return markerMap[ID];
}

static MarkerSystem& MarkerSystem::getInstance() {
    static MarkerSystem markerSystem;
    return markerSystem;
}

static inline std::string getJunctionName(const std::string& ID){
    std::string name;
    int i = 0;
    while (ID[i]!='_') ++i;
    ++i;
    while (ID[i]!='_'){
        name += ID[i];
        ++i;
    }
    return name;
}

std::vector<ExitMarker*> MarkerSystem::findExitMarkerstoEntry(const std::string& entryID){
    std::string junctionName = getJunctionName(entryID);
    std::vector<ExitMarker*> answer;

    for (auto i=markerMap.begin(); i!=markerMap.end(); ++i){
        if ((*i).first.compare(6,4,"Exit") == 0) {
            if (getJunctionName((*i).first).compare(junctionName) == 0)
                answer.push_back(static_cast<ExitMarker*>((*i).second));
        }
    }
}

