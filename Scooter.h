#ifndef SCOOTER_H
#define SCOOTER_H

#include <iostream>

#include "Agent.h"

using namespace std;

class Scooter : public Agent{
public:
    //  baterie 200 , viteza 2 , consum 5 , cost 4, capacitate 2
    Scooter(string id) 
        : Agent("Scooter_" + id,"SCUTER",  200, 2, 5, 4, 2) // 
    {
    }

    bool isAerial() const override 
    {
        //  ocoleste zidurile 
        return false; 
    }
};

#endif