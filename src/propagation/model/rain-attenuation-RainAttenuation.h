#ifndef RAINATTENUATION_H
#define RAINATTENUATION_H


#include "rain-attenuation-structs.h"

namespace ns3{
class RainAttenuation{

public:
    RainAttenuation();
    RainAttenuation(double f,std::vector<double> R, double prctile);
    RainAttenuation(double f, double theta, double tau,std::vector<double> R, double prctile);

    SpecRainAttCoeff SpecRainAttCoeffs();
    RainAttCoeff RainAttCoeffs();

    double SpecAtt(double R) const;
    double EffectivePathLength(double R,double d) const;
    double CalcRainAtt(double d) const;

    void CalcRainPrctile();
        

private:

    double f;
    double prctile;

    double R_prctile;    
    std::vector<double> R_vec;

    double theta;
    double tau;
    SpecRainAttCoeff SpecGammaCoeffs;
    RainAttCoeff GammaCoeffs;

};

}

#endif