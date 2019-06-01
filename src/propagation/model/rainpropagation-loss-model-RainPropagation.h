#ifndef RAINPROPAGATION_H
#define RAINPROPAGATION_H


#include "structs.h"
#include "control.h"

namespace rainprop{
class RainPropagation{

public:
    RainPropagation(Control controlSettings);
    RainPropagation(Control controlSettings,double theta, double tau);
    RainPropagation(Control controlSettings,std::vector<std::vector<double> > R_01); 
    RainPropagation(Control controlSettings,std::vector<std::vector<double> > R_01,double theta,double tau);  

    SpecRainAttCoeff SpecRainAttCoeffs();
    RainAttCoeff RainAttCoeffs();

    double SpecAtt(double R);
    std::vector<double> SpecAtt(std::vector<double> R);

    std::vector<double> EffectivePathLength();

    std::vector<double> TotalRainAtt();
    
    void SetRainValues(std::vector<std::vector<double>>R_01);

    void CalculateRainAttenuation();
    std::vector<double> Reshape(std::vector<std::vector<double> > V);

    std::vector<double> GetGammaR();
    std::vector<double> GetEffpl();
    std::vector<double> GetTotalAtt();

    double GetNextValue();

    void Run();

private:

    double f;
    double d;
    std::vector<double> R_01;

    double theta;
    double tau;
    SpecRainAttCoeff SpecGammaCoeffs;
    RainAttCoeff GammaCoeffs;

    std::vector<double> gamma_r;
    std::vector<double> effpl;
    std::vector<double> totalatt;
    int currval;

};
}


#endif