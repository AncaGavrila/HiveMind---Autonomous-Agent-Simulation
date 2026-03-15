#ifndef DRONA_H
#define DRONA_H

#include<iostream>

#include "Agent.h"
using namespace std;

class Drona : public Agent {

public:
    Drona(string id) : Agent("Drona_" + id,"DRONA", 100, 3, 10, 15, 1) {} 
    
    bool isAerial() const override
     { 
        return true;
    
    }
};
#endif