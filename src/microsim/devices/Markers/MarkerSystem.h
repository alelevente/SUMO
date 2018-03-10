//
// Created by levente on 2018.03.10..
//

#ifndef SUMO_MARKERSYSTEM_H
#define SUMO_MARKERSYSTEM_H

#include <vector>
#include "BaseMarker.h"

class MarkerSystem{
    MarkerSystem();
    ~MarkerSystem();
   // static MarkerSystem& singleton;
    std::map<std::string, BaseMarker*> markerMap;

public:
    static MarkerSystem& getInstance();
    BaseMarker* findMarkerByID(const std::string& ID);
    static inline bool isMarkerID(const std::string& ID);
};

#endif //SUMO_MARKERSYSTEM_H
