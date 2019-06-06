#ifndef CONTROL_H
#define CONTROL_H

#include "rain-attenuation-structs.h"

namespace ns3{

class Control{
    
public:
    Control();
    Control(double lat,double lon);

    void SetLocation(double lon, double lat);
    Cords GetLocation();

    std::vector<int> monthdays;
    std::vector<int> monthhours;
    std::vector<int> monthhours_cumsum;

private:
    //internal functions used by the constructor
   
    void SetHelperVectors();
    void ComputeMonthhoursCumSum();
    Cords loc;                                        
                                 
};
}

#endif