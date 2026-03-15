#ifndef ROBOT_H
#define ROBOT_H
#include "Agent.h"
#include <iostream>


using namespace std;

class Robot : public Agent{
public:
    //  baterie 300, viteza 1, consum 2, cost 1, capacitate 4
    Robot(string id) 
        : Agent("Robot_" + id,"ROBOT",  300, 1, 2, 1, 4) // 
    {
   
    }

    
    bool isAerial() const override 
    {
        //  ocoleste zidurile '#' 
        return false; 
    }
};


#endif