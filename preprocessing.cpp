#include <bits/stdc++.h>
#include "data_format.h"
#include "preprocessing.h"
using namespace std;

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

vector<Segment> segment(vector<vector<double>> &xy_data) {
    vector<Segment> segments;
    double threshold = 0.5;
    
    int tol_t = xy_data.size();

    for (int t=0; t<tol_t; t++) {
        vector<bool> segmented(360, false);
        for (int i=0; i<360; i++) {
            if (segmented[i]) continue;
            
            Point current(xy_data[t][2*i], xy_data[t][2*i+1]);
            for (int k=-3; k<=3; k++) {
                int nxt_idx = (i+k+360)%360;
                if (segmented[nxt_idx]) continue;
                
                Point neighbor(xy_data[t][2*nxt_idx], xy_data[t][2*nxt_idx+1]);

                // TODO think about how to segment (detecting 7 neighboring point)
            }
        }
    }
    return segments;
}
