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
    ofstream file;
    file.open("segmented_data.dat");

    vector<Segment> segments;
    double threshold = 0.5;
    
    int tol_t = xy_data.size();

    Segment seg;
    for (int t=0; t<tol_t; t++) {
        int s_n = 0; // the number of segments in this second

        vector<int> n0ind; 
        for (int i=0; i<360; i++) {
            if (xy_data[t][2*i]!=0 || xy_data[t][2*i+1]!=0) {
                n0ind.push_back(i);
            }
        }

        for (int i=0; i<n0ind.size()-1; i++) {
            Point current(xy_data[t][2*n0ind[i]], xy_data[t][2*n0ind[i]+1]);
            seg.segment.push_back(current);
            seg.size++; 

            int nxt_idx = n0ind[i+1];
            
            Point neighbor(xy_data[t][2*nxt_idx], xy_data[t][2*nxt_idx+1]);

            if (current.dist_to(neighbor)<threshold) {
                seg.segment.push_back(neighbor);
                seg.size++;
            } else {
                segments.push_back(seg);
                s_n++;
                seg.segment.clear();
                seg.size = 0;
            }
        }

        file << s_n << endl;
        for (int i=segments.size()-s_n; i<segments.size(); i++) {
            file << segments[i].size << " ";
            for (int j=0; j<segments[i].size; j++) {
                file << segments[i].segment[j].x << " " << segments[i].segment[j].y << " "; 
            } file << endl;
        }
    }
    file.close();
    return segments;
}
