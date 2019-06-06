#ifndef RAINGENERATOR_H
#define RAINGENERATOR_H

#include "rain-attenuation-structs.h"
#include "rain-attenuation-control.h"


namespace ns3{


class RainGenerator{

public:

    RainGenerator(Control controlSettings);

    char DecideClimaticRegion();

    void ITUR837_calculation();

    void RainValues();
    void SplitInRainEvents();
    void SimulateRainYear();
    std::vector<double> GetRainValues(int i);

    void Run();

private:

    //Helper functions for internal use
    std::vector<Koppen> ReadKoppen();
    std::vector<double> ReadCoordinates(const char* filename);

    int FindMinIndex(std::vector<double> map);
    std::vector<Cords> ClosestPoints(std::vector<double> latMap,std::vector<double> lonMap, int latMinIndex,int lonMinIndex);
    double ReadCsvValue(const char* filename,int i,int latMinIndex,int lonMinIndex);
    double BilinearInterpolation(std::vector<double> T, std::vector<Cords> sq, double lat,double lon, std::vector<double> latMap,std::vector<double>lonMap);

    std::vector<Matrix> ReadRainValues(const char* filename);
    std::vector<Matrix> ConvertRainValues(std::vector<Matrix> R_60);
    //
    
    Cords loc;
    char cl_region;
    ITUR837_values itu_v;
    Control control;
    std::vector<Matrix> R_01;
    std::vector< std::vector<RainEvent> > RainEvents;
    std::vector<Matrix> R_01_simulated;

};

}

#endif