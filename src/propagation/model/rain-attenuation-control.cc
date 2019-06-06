
#include "rain-attenuation-control.h"

namespace ns3{

    Control::Control()
    {
        loc.lat = 0;
        loc.lon = 0;
        SetHelperVectors();
    };

    Control::Control(double lat,double lon)
    {
        this->loc.lat = lat;
        this->loc.lon = lon;
        SetHelperVectors();
    };


    void Control::SetLocation(double lat, double lon)
    {
       Cords loc = {lat,lon};
       this-> loc = loc;
    };


    Cords Control::GetLocation()
    {
        return this->loc;
    };


    void Control::SetHelperVectors()
    {
        monthdays  = {31 , 28 , 31 , 30 , 31 , 30  , 31 , 31  , 30 , 31 , 30 , 31 };
        monthhours = {744, 672, 744, 720, 744, 720 , 744, 744 , 720, 744, 720, 744};
        ComputeMonthhoursCumSum();
    };



    void Control::ComputeMonthhoursCumSum()
    {
        std::vector<int> v_cum;
        v_cum.push_back(0);

        for (int i = 1; i < monthhours.size()+1; i++)
        {
          v_cum.push_back( v_cum[i-1] + monthhours [i-1] );
        }

        this->monthhours_cumsum = v_cum;
    };

}