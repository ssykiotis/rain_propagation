#ifndef CONTROL_H
#define CONTROL_H

#include "structs.h"

namespace rainprop{

class Control{
    
public:
    Control();
    Control(double lat,double lon, double f, double dist);

    void SetLocation(double lon, double lat);
    void SetFrequency(double freq);
    void SetDistance(double dist);
    void SetFSMCStates(int states);

    Cords GetLocation();
    double GetFrequency();
    double GetDistance();

    std::vector<int> monthdays;
    std::vector<int> monthhours;
    std::vector<int> monthhours_cumsum;

private:
    //internal functions used by the constructor
    void SetMonthdays();
    void SetMonthhours();
    void ComputeMonthhoursCumSum();

    Cords loc;                                        
    double f;                                    
    double dist;                                    
};
}

#endif