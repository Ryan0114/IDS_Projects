#include <bits/stdc++.h>
#include "data_format.h"
#include "preprocessing.h"
using namespace std;

// choose bot type
string bot = "turtlebot/";
// string bot = "minibot/";

// choose file
string filename = "raw_data/box_rt.dat";
// string filename = "raw_data/ball_static_rt.dat";
// string filename = "raw_data/ball_moving_rt.dat";

string file = bot + filename; 

vector<Segment> segments;

int main() {
    ifstream rt_data(file);

    vector<vector<double>> xy_data = rt2xy(rt_data); 

    int tol_t = xy_data.size();
    cout << tol_t << endl;

    vector<Segment> seg = segment(xy_data, true);
} 
