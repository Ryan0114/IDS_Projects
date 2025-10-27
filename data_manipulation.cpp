#include "data_manipulation.h"

#define PI atan(-1)

double deg2rad(double deg) {
    return deg * PI / 180.0; 
}

vector<vector<double>> rt2xy(ifstream &fin) {
    vector<vector<double>> xy_data;
    
    double ang, dist;
    int pt_cnt = 0;
    vector<double> second; // store converted xy-data for each second
    while (fin >> ang >> dist) {
        if (pt_cnt==360) {
            xy_data.push_back(second);
            second.clear();
            pt_cnt = 0;
        } 
       
        double ang_rad = deg2rad(ang); 
        double x = dist * cos(ang_rad);
        double y = dist * sin(ang_rad);
        second.push_back(x);
        second.push_back(y);
        
        pt_cnt++;
    } 

    return xy_data;
}
