#include <bits/stdc++.h>
#include "data_format.h"
#include "preprocessing.h"
using namespace std;

#define PI acos(-1)

double deg2rad(double deg) {
    return deg * PI / 180.0; 
}

vector<vector<double>> read_rt_file(ifstream &fin) {
    vector<vector<double>> rt_data;

    double ang, dist;
    int pt_cnt = 0;
    vector<double> second;
    while (fin >> ang >> dist) {
        if (pt_cnt==360) {
            rt_data.push_back(second);
            second.clear();
            pt_cnt = 0;
        }

        second.push_back(ang);
        second.push_back(dist);

        pt_cnt++;
    }

    return rt_data;
}

vector<vector<double>> rt2xy(vector<vector<double>> rt_data) {
    vector<vector<double>> xy_data;

    for (int i=0; i<rt_data.size(); i++) {
		vector<double> second;
        for (int j=0; j<360; j++) {
            double ang = rt_data[i][2*j];
            double dist = rt_data[i][2*j+1];
            double ang_rad = deg2rad(ang); 
            double x = dist * cos(ang_rad);
            double y = dist * sin(ang_rad);
            second.push_back(x);
            second.push_back(y);
        }    
        xy_data.push_back(second);
    } 
    return xy_data;
}

vector<Segment> segment(vector<vector<double>> &xy_data, bool output) {
    ofstream file;
    if (output) {
        file.open("segmented_data.dat");
    }

    vector<Segment> segments;
    double threshold = 0.1;
    
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

        Point current(xy_data[t][2*n0ind[0]], xy_data[t][2*n0ind[0]+1]);
        seg.points.push_back(current);
        seg.size++;
        for (int i=0; i<n0ind.size()-1; i++) {
            int nxt_idx = n0ind[i+1];
            
            Point neighbor(xy_data[t][2*nxt_idx], xy_data[t][2*nxt_idx+1]);

            if (current.dist_to(neighbor)<threshold) {
                seg.points.push_back(neighbor);
                current = neighbor;
                seg.size++;
            } else {
                if (seg.size > 3) {
                    segments.push_back(seg);
                    s_n++;
                }
                seg.points.clear();
                seg.size = 0;

                current = neighbor;
            }
        }
        
        if (output) {
            file << s_n << endl;
            for (int i=segments.size()-s_n; i<segments.size(); i++) {
                if (segments[i].size <= 3) continue;
                file << segments[i].size << " ";
                for (int j=0; j<segments[i].size; j++) {
                    file << segments[i].points[j].x << " " << segments[i].points[j].y << " "; 
                } file << endl;
            }
        }
    }
    if (output) file.close();

    return segments;
}
