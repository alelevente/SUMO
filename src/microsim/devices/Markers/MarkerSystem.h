//
// Created by levente on 2018.03.10..
//

#ifndef SUMO_MARKERSYSTEM_H
#define SUMO_MARKERSYSTEM_H

#include <vector>
#include "BaseMarker.h"
#include "ExitMarker.h"

class MarkerSystem{
    MarkerSystem();
    ~MarkerSystem();
    MarkerSystem(MarkerSystem const&);
    void operator=(const MarkerSystem&);
    //static MarkerSystem singleton;
    std::map<std::string, BaseMarker*> markerMap;
    std::vector<ExitMarker*> findExitMarkerstoEntry(const std::string& entryID);

public:
    static MarkerSystem& getInstance();
    BaseMarker* findMarkerByID(const std::string& ID);
    static inline bool isMarkerID(const std::string& ID);
    const std::map<std::string, BaseMarker*>& getMarkerMap();
};

#endif //SUMO_MARKERSYSTEM_H
