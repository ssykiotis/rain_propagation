#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <vector>


namespace ns3{

    struct Cords{
        double lat;
        double lon;
    };  

    struct Koppen{
        double lat;
        double lon;
        std::string region;
    };

    struct ITUR837_values{
        std::vector<double> Temperature;
        std::vector<double> MonthlyTotal;
        std::vector<double> P_rain;
        std::vector<double> r; 
    };

    struct RainEvent{
        int i;
        int j;
        int length;
    };

    struct SpecRainAttCoeff{
        double k_h;
        double k_v;
        double a_h;
        double a_v;
    };

    struct RainAttCoeff{
        double k;
        double a;
    };

    struct Matrix {
        std::vector< std::vector<double> > v;
    };
}

#endif