#include "rain-attenuation-RainAttenuation.h"
#include <math.h> 
#include <bits/stdc++.h>  


namespace ns3{

RainAttenuation::RainAttenuation(){

};


RainAttenuation::RainAttenuation(double f, std::vector<double> R, double prctile)
{
    this->f = f/1e09;
    this->theta = 0;
    this->tau = 0;
    this->prctile = prctile/100;
    this->R_vec = R;
    this->SpecGammaCoeffs = SpecRainAttCoeffs();
    this->GammaCoeffs     = RainAttCoeffs();
    CalcRainPrctile();
};

RainAttenuation::RainAttenuation(double f, double theta, double tau,std::vector<double> R, double prctile)
{
    this->theta = theta;
    this->tau = tau;
    this->prctile = prctile; 
    this->R_vec = R;
    this->SpecGammaCoeffs = SpecRainAttCoeffs();
    this->GammaCoeffs = RainAttCoeffs(); 
};


SpecRainAttCoeff RainAttenuation::SpecRainAttCoeffs()
{
    std::vector <std::vector<double> > k_h_table{  {-5.33980,-0.10008, 1.13098},
                                                   {-0.35351, 1.26970, 0.45400},
                                                   {-0.23789, 0.86036, 0.15354},
                                                   {-0.94158, 0.64552, 0.16817}
                                                 };

    std::vector <std::vector<double> > k_v_table{  {-3.80595, 0.56934, 0.81061},
                                                   {-3.44965,-0.22911, 0.51059},
                                                   {-0.39902, 0.73042, 0.11899},
                                                   { 0.50167, 1.07319, 0.27195}
                                                  };

    std::vector <std::vector<double> > a_h_table{  {-0.14318, 1.82442,-0.55187},
                                                   { 0.29591, 0.77564, 0.19822},
                                                   { 0.32177, 0.63773, 0.13164},
                                                   {-5.37610,-0.96230, 1.47828},
                                                   {16.17210,-3.29980, 3.43990}
                                                  };

    std::vector <std::vector<double> > a_v_table{  {-0.07771, 2.33840,-0.76284},
                                                   { 0.56727, 0.95545, 0.54039},
                                                   {-0.20238, 1.14520, 0.26809},
                                                   {-48.2991,0.791669,0.116226},
                                                   { 48.5833,0.791459,0.116479}
                                                  };                                                  

    double k_h_m = -0.18961;
    double k_h_c = 0.71147;

    double k_v_m = -0.16398;
    double k_v_c = 0.63297;

    double a_h_m = 0.67849;
    double a_h_c = -1.95537;
    
    double a_v_m = -0.053739;
    double a_v_c = 0.83433;

    double log_f =log10(f);
    SpecRainAttCoeff gammacoeffs {0,0,0,0};

    for (uint i = 0; i < k_h_table.size(); i++)
    {
        gammacoeffs.k_h += k_h_table[i][0]*exp(-pow((log_f-k_h_table[i][1])/k_h_table[i][2],2));
        gammacoeffs.k_v += k_v_table[i][0]*exp(-pow((log_f-k_v_table[i][1])/k_v_table[i][2],2)); 
    }
    gammacoeffs.k_h+=log_f*k_h_m+k_h_c;
    gammacoeffs.k_v+=log_f*k_v_m+k_v_c;

    for (uint i = 0; i < a_h_table.size(); i++)
    {
        gammacoeffs.a_h += a_h_table[i][0]*exp(-pow((log_f-a_h_table[i][1])/a_h_table[i][2],2));
        gammacoeffs.a_v += a_v_table[i][0]*exp(-pow((log_f-a_v_table[i][1])/a_v_table[i][2],2)); 
    }
    gammacoeffs.a_h+=log_f*a_h_m+a_h_c;
    gammacoeffs.a_v+=log_f*a_v_m+a_v_c;
    gammacoeffs.k_h = pow(10,gammacoeffs.k_h);
    gammacoeffs.k_v = pow(10,gammacoeffs.k_v);

    return gammacoeffs;
};

RainAttCoeff RainAttenuation::RainAttCoeffs()
{
    RainAttCoeff attcoeffs;
    attcoeffs.k = (SpecGammaCoeffs.k_h+SpecGammaCoeffs.k_v+(SpecGammaCoeffs.k_h-SpecGammaCoeffs.k_v)*pow(cos(theta),2)*cos(2*tau))/2;
    attcoeffs.a = (SpecGammaCoeffs.k_h*SpecGammaCoeffs.a_h+SpecGammaCoeffs.k_v*SpecGammaCoeffs.a_v
        +(SpecGammaCoeffs.k_h*SpecGammaCoeffs.a_h-SpecGammaCoeffs.k_v*SpecGammaCoeffs.a_h)*pow(cos(theta),2)*cos(2*tau))/(2*attcoeffs.k);
    return attcoeffs;
};

double RainAttenuation::SpecAtt(double R) const
{
    return GammaCoeffs.k*pow(R,GammaCoeffs.a);
};


double RainAttenuation::EffectivePathLength(double R,double d) const
{

    double effpl;
    double term1;
    double term2;

        if (R==0){
            effpl=0;
        }
        else{
            term1 = 0.477*pow(d,0.633)*pow(R,0.073*GammaCoeffs.a)*pow(f,0.123);
            term2 = 10.579*(1-exp(-0.024*d));
            effpl = 1/(term1-term2);    
        }

        if (effpl>2.5)
        {
            effpl=2.5;
        }

        return effpl;
};

double RainAttenuation::CalcRainAtt(double d) const
{
    double gamma_r = SpecAtt(R_prctile);
    double effpl = EffectivePathLength(R_prctile,d);
    return gamma_r*effpl*d;
};


void RainAttenuation::CalcRainPrctile(){
    
    double r;
    double k;
    double y;

    std::sort(R_vec.begin(),R_vec.end());
    r = prctile*R_vec.size();
    k = floor(r+0.5);
    r-=k;
    y = (0.5-r)*R_vec[k-1]+(0.5+r)*R_vec[k-1];

    R_prctile = y;    
};

}

