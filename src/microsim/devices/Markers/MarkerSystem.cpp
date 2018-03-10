//
// Created by levente on 2018.03.10..
//

#include "MarkerSystem.h"
#include "EntryMarker.h"
#include "ExitMarker.h"
#include <microsim/MSEdgeControl.h>

//MarkerSystem::singleton = NULL;

static inline bool isMarkerID(const std::string& ID){
    return ID.compare(0,6,"marker") == 0;
}

MarkerSystem::MarkerSystem(){
    std::vector<std::string> edgeNames = MSNet::getInstance()->getEdgeControl().getEdgeNames();
    for (auto i = edgeNames.begin(); i != edgeNames.end(); ++i){
        //if marker
        if (isMarkerID(*i)){
            BaseMarker* newMarker = NULL;
            if ((*i).compare(6, 5, "Entry") == 0) {
                newMarker = new EntryMarker(*i, NULL);

            } else if ((*i).compare(0,5,"Exit") == 0) {
                newMarker = new ExitMarker(*i, NULL);
            }
            markerMap.insert(std::make_pair(*i, newMarker));
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