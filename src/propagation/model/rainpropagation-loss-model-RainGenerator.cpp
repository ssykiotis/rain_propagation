
#include "RainGenerator.h"
#include <bits/stdc++.h>  

namespace rainprop{

RainGenerator::RainGenerator(Control controlSettings){
    this->loc.lat = controlSettings.GetLocation().lat;
    this->loc.lon = controlSettings.GetLocation().lon;
    this->control = controlSettings;
    this->cl_region = DecideClimaticRegion();
};

char RainGenerator::DecideClimaticRegion(){
    std::vector<Koppen> koppen = ReadKoppen();
    std::vector<Koppen> koppen_search = koppen;

    for (int i = 0; i < koppen_search.size(); i++){
        koppen_search[i].lat = fabs(koppen[i].lat - loc.lat);
        koppen_search[i].lon = fabs(koppen[i].lon - loc.lon);
    }

    int index = 0;
    double min = koppen_search[0].lon;

    for (int i = 0; i < koppen_search.size(); i++){
        if (koppen_search[i].lon < min){
            index = i;
            min = koppen_search[i].lon;
        }
    }

    if (koppen[index + 1].lon == koppen[index].lon){
        int last_index = index + 1;

        while (koppen_search[last_index].lon == min){
            last_index++;
        }
        min = koppen_search[index].lat;
        for (int i = index; i < last_index; i++){
            if (koppen_search[i].lat < min){
                index = i;
                min = koppen_search[i].lat;
            }
        }
    }

    std::string temp = koppen[index].region;
    char cstr[temp.size()];
    strcpy(cstr, temp.c_str());
    char a = cstr[0];
    return a;
};

char RainGenerator::GetClimaticRegion(){
    return this->cl_region;
};

Cords RainGenerator::SetLocation(Cords loc){
    this->loc = loc;
};

Cords RainGenerator::GetLocation(){
    return this->loc;
};

void RainGenerator::ITUR837_calculation(){

    //calculate Mean Temperatures
    std::vector<double> latMap = ReadCoordinates("rain_data/ITU/temperature/LAT_T.csv");
    std::vector<double> lonMap = ReadCoordinates("rain_data/ITU/temperature/LON_T.csv");

    std::vector<double> latMap_or = latMap;
    std::vector<double> lonMap_or = lonMap;
    for (int i = 0; i < latMap.size(); i++){
        latMap[i] = fabs(latMap[i] - loc.lat);
    }
    for (int i = 0; i < lonMap.size(); i++) {
        lonMap[i] = fabs(lonMap[i] - loc.lon);
    }

    int latMinIndex = FindMinIndex(latMap);
    int lonMinIndex = FindMinIndex(lonMap);

    std::vector<Cords> sq = ClosestPoints(latMap,lonMap,latMinIndex,lonMinIndex);
    std::vector<double> T;
    for (int i = 1; i < 13; i++){
        const char* filename = "rain_data/ITU/temperature/T_Month%.2d.csv";
        for (int j = 0; j < sq.size(); j++){
           T.push_back(ReadCsvValue(filename, i, sq[j].lat, sq[j].lon));    
        }
        itu_v.Temperature.push_back(BilinearInterpolation(T,sq,loc.lat,loc.lon,latMap_or,lonMap_or));
        T.clear();     
    } 

    latMap.clear();
    lonMap.clear();

    //calculate Mean Monthly Total Rainfall
    latMap = ReadCoordinates("rain_data/ITU/monthly_total/LAT_MT.csv");
    lonMap = ReadCoordinates("rain_data/ITU/monthly_total/LON_MT.csv");

    latMap_or = latMap;
    lonMap_or = lonMap;

    for (int i = 0; i < latMap.size(); i++){
        latMap[i] = fabs(latMap[i] - loc.lat);
    }
    for (int i = 0; i < lonMap.size(); i++){
        lonMap[i] = fabs(lonMap[i] - loc.lon);
    }

    latMinIndex = FindMinIndex(latMap);
    lonMinIndex = FindMinIndex(lonMap);

    sq = ClosestPoints(latMap,lonMap,latMinIndex,lonMinIndex);
    std::vector<double> MT;
    for (int i = 1; i < 13; i++){
        const char* filename = "rain_data/ITU/monthly_total/MT_Month%.2d.csv";
        for (int j = 0; j < sq.size(); j++){
           MT.push_back(ReadCsvValue(filename, i, sq[j].lat, sq[j].lon));    
        }
        itu_v.MonthlyTotal.push_back(BilinearInterpolation(MT,sq,loc.lat,loc.lon,latMap_or,lonMap_or));
        MT.clear();     
    } 

    for (int i = 0; i < 12; i++){
        itu_v.Temperature[i] -= 273.15;
        if (itu_v.Temperature[i]>=0){
            itu_v.r.push_back(pow(0.578,0.083*itu_v.Temperature[i]));
        }
        else{
            itu_v.r.push_back(0.5874);
        }
        itu_v.P_rain.push_back(itu_v.MonthlyTotal[i]/(24*control.monthdays[i]*itu_v.r[i]));
        if(itu_v.P_rain[i]>0.7){
            itu_v.P_rain[i] = 0.7;
            itu_v.r[i] = 100*itu_v.MonthlyTotal[i]/(70*24);
        }
    }
};

void RainGenerator::RainValues(){

    const char* filename;
    switch (cl_region){
    case 'A':
        filename = "rain_data/equatorial.csv";
        break;    
    case 'B':
        filename = "rain_data/arid.csv";
        break;
    case 'C':
        filename = "rain_data/temperate.csv";
        break;    
    case 'D':
        filename = "rain_data/snow.csv";
        break;
    case 'E':
        filename = "rain_data/snow.csv";
    default:
        break;
    }

    std::vector<Matrix> R_60 = ReadRainValues(filename);
    std::vector<Matrix> R_01 = ConvertRainValues(R_60);
    this->R_01 = R_01;
};

void RainGenerator::SplitInRainEvents(){
    int j;
    int months = R_01.size();
    int hours;
    int length;
    RainEvent event;
    std::vector<std::vector<double> > R_01_month;

    std::vector<RainEvent> RainEvents_month;
    std::vector<std::vector<RainEvent> > RainEvents_year;

    for (int i = 0; i < months; i++){
        hours = control.monthhours[i];
        R_01_month = R_01[i].v;
        for (int k = 0; k < R_01_month[0].size(); k++){
            j=0;
            while(j<hours){
                if(R_01_month[j][k]!=0){
                    length =1;
                    if(j+length<hours){
                        while(R_01_month[j+length][k]!=0){
                            length++;
                            if((j+length)>hours-1){
                                length--;
                                break;
                            }
                        }  
                    }
                    event = {j,k,length};
                    j=j+length;
                    RainEvents_month.push_back(event);
                }
                else{
                    j++;
                }
            }
        }
        RainEvents_year.push_back(RainEvents_month);
        RainEvents_month.clear();
    }
    this->RainEvents = RainEvents_year;
};

void RainGenerator::SimulateRainYear(){
    std::vector<Matrix> SimulatedValues(12);
    int months = SimulatedValues.size();
    int hours;
    double P_rain_month;
    double val;
    int j;
    std::vector<std::vector<double> > values;
    RainEvent event; 
    int event_index;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> d;

    for (int i = 0; i < months; i++){   
        j=0;
        hours = control.monthhours[i];
        values.resize(hours);
        P_rain_month = itu_v.P_rain[i]*100;
        d = {100-P_rain_month,P_rain_month};
        while(j<hours){
            val = d(gen);
            if(val!=0){
                event_index = std::rand()%RainEvents[i].size();
                event = RainEvents[i][event_index];
                if(event.length+j>hours){
                    event.length = j-hours;
                }
                while(event.length>0){
                    values[j].push_back(R_01[i].v[event.i][event.j]);
                    event.length--;
                    event.i++;
                    j++;
                }
            }
            else{
                values[j].push_back(0);
                j++;
            }
        }
        SimulatedValues[i].v=values;
        values.clear();
    }

    double mu;
    double sigma;
    std::default_random_engine generator;
    std::normal_distribution<double> distribution;
    double nvalue;

    for (int i = 0; i < SimulatedValues.size(); i++){
        for (int j = 0; j < SimulatedValues[i].v.size(); j++){
            if (SimulatedValues[i].v[j][0]!=0){
                mu = SimulatedValues[i].v[j][0];
                sigma = mu/3;
                distribution= std::normal_distribution<double>(mu,sigma);
                for (int l = 0; l < 59; l++) {
                    nvalue = distribution(generator);
                    while (nvalue<=0){
                        nvalue = distribution(generator);
                    }
                    SimulatedValues[i].v[j].push_back(nvalue);
                }  
            }
            else{
                for (int l = 0; l < 59; l++) {
                   SimulatedValues[i].v[j].push_back(0);
                }  
            }  
        }   
    }
    R_01_simulated=SimulatedValues;   
};

std::vector<std::vector<double> > RainGenerator::GetSimulatedValues(int i){
    return R_01_simulated[i].v;
};

void RainGenerator::Run(){
    ITUR837_calculation();
    RainValues();
    SplitInRainEvents();
    SimulateRainYear();
};

std::vector<Koppen> RainGenerator::ReadKoppen(){
    std::vector<Koppen> v;
    std::ifstream ip("rain_data/koppen.csv");

    std::string readlat;
    std::string readlon;
    std::string readregion;

    Koppen cords;

    while (ip.good()){
        getline(ip, readlon, ',');
        getline(ip, readlat, ',');
        getline(ip, readregion, '\n');

        cords.lat = atof(readlat.c_str());
        cords.lon = atof(readlon.c_str());
        cords.region = readregion;

        v.push_back(cords);
    }

    ip.close();
    return v;
};

std::vector<double> RainGenerator::ReadCoordinates(const char *filename){
    std::vector<double> v;
    std::ifstream ip(filename);
    std::string readval;

    while (ip.good()){
        getline(ip, readval, '\n');
        v.push_back(atof(readval.c_str()));
    }
    ip.close();
    return v;
};

int RainGenerator::FindMinIndex(std::vector<double> map){
    int minIndex = 0;
    double min = map[0];

    for (int i = 0; i < map.size(); i++){
        if (map[i] < min){
            minIndex = i;
            min = map[i];
        }
    }
    return minIndex;
};

std::vector<Cords> RainGenerator::ClosestPoints(std::vector<double> latMap,std::vector<double> lonMap, int latIndex,int lonIndex){

    std::vector<Cords> sq1;
    sq1.push_back( {latIndex  , lonIndex-1} );
    sq1.push_back( {latIndex-1, lonIndex-1} );
    sq1.push_back( {latIndex-1, lonIndex  } );
    sq1.push_back( {latIndex  , lonIndex  } );

    std::vector<Cords> sq2;
    sq2.push_back( {latIndex  , lonIndex  } );
    sq2.push_back( {latIndex-1, lonIndex  } );
    sq2.push_back( {latIndex-1, lonIndex+1} );
    sq2.push_back( {latIndex  , lonIndex+1} );

    std::vector<Cords> sq3;
    sq3.push_back( {latIndex+1, lonIndex-1} );
    sq3.push_back( {latIndex  , lonIndex-1} );
    sq3.push_back( {latIndex  , lonIndex  } );
    sq3.push_back( {latIndex+1, lonIndex  } );
    
    std::vector<Cords> sq4;
    sq4.push_back( {latIndex+1, lonIndex  } );
    sq4.push_back( {latIndex  , lonIndex  } );
    sq4.push_back( {latIndex  , lonIndex+1} );
    sq4.push_back( {latIndex+1, lonIndex+1} );
    
    std::vector<double> dist; 
    Cords point;

    double d =0;
    for (int i = 0; i < sq1.size(); i++){
        point = sq1[i];
        d+= pow(latMap[point.lat],2) + pow(lonMap[point.lon],2);
    }
    dist.push_back(d);

    d =0;
    for (int i = 0; i < sq2.size(); i++){
        point = sq2[i];
        d+= pow(latMap[point.lat],2) + pow(lonMap[point.lon],2);
    }
    dist.push_back(d);
    
    d =0;
    for (int i = 0; i < sq3.size(); i++){
        point = sq3[i];
        d+= pow(latMap[point.lat],2) + pow(lonMap[point.lon],2);
    }
    dist.push_back(d);

    d =0;
    for (int i = 0; i < sq4.size(); i++){
        point = sq4[i];
        d+= pow(latMap[point.lat],2) + pow(lonMap[point.lon],2);
    }
    dist.push_back(d);

    int minIndex = 0;
    double min = dist[0];

    for (int i = 0; i < dist.size(); i++){
        if (dist[i]<min){
            minIndex = i;
            min = dist[i];
        } 
    }

    switch (minIndex){
    case 0:
        return sq1;
    case 1:
        return sq2;    
    case 2:
        return sq3;    
    case 3:
        return sq4;
    default:
        break;
    }
};

double RainGenerator::BilinearInterpolation(std::vector<double> T, std::vector<Cords> sq, double lat,double lon, std::vector<double> latMap,std::vector<double>lonMap){

    double R = sq[0].lat;
    double C = sq[0].lon;
    double r = R+ (lat - latMap[R])/(latMap[R+1]-latMap[R]);
    double c = C+ (lon-lonMap[C])/(lonMap[C+1]-lonMap[C]);

    double I_rc = T[0]*(R-r+1)*(C-c+1)+T[1]*(r-R)*(C-c+1) +T[2]*(R-r+1)*(c-C) + T[3]*(r-R)*(c-C);
    return I_rc; 
};

double RainGenerator::ReadCsvValue(const char* filename,int i, int latMinIndex, int lonMinIndex){
        
        char buffer[100];
        int n = sprintf(buffer, filename, i);
        std::ifstream ip(buffer);
        std::string line;
        int LinesToGo = latMinIndex;
        while (getline(ip, line) && (LinesToGo > 0)){
            LinesToGo--;
        }
        ip.close();
        int index = 0;
        std::string delimiter = " ";
        while (index != lonMinIndex){
            line.erase(0, line.find(delimiter) + delimiter.length());
            index++;
        }
        std::string val = line.substr(0, line.find(delimiter));

        return atof(val.c_str());
};

std::vector<Matrix> RainGenerator::ReadRainValues(const char* filename){
    std::vector<Matrix> R_year;
    Matrix R_temp;
    std::vector <std::vector<double> > R_month;
    std::string val;
    std::vector<double> readvalues_line;
    std::vector<int> monthhours_cumsum = this->control.monthhours_cumsum;
    std::ifstream ip(filename);

    for (int i = 0; i < (monthhours_cumsum.size()-1); i++){         //loop for months
    
        for (int j = monthhours_cumsum[i]; j< monthhours_cumsum[i+1]; j++){   //loop for lines of month
            for (int k = 0; k < 10; k++){                //read one line
                getline(ip,val,',');
                readvalues_line.push_back(atof(val.c_str()));
            }
            getline(ip,val,'\n');
            readvalues_line.push_back(atof(val.c_str()));
            R_month.push_back(readvalues_line);
            readvalues_line.clear();
        }
        R_temp.v = R_month;
        R_year.push_back(R_temp);
        R_month.clear();
    }
    return R_year;
};

std::vector<Matrix> RainGenerator::ConvertRainValues(std::vector<Matrix> R_60){
    double a = 0.509;
    double b= 1.394;
    std::vector<Matrix> R_01 = R_60;
    for (int k = 0; k < R_01.size(); k++){
        for (int i = 0; i < R_01[k].v.size(); i++){
            for (int j=0; j<R_01[k].v[i].size() ; j++){
                R_01[k].v[i][j] = pow(a*R_01[k].v[i][j],b);
            }    
        }   
    }
    return R_01;
};

}