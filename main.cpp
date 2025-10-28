#include <bits/stdc++.h>
#include "data_format.h"
#include "preprocessing.h"
using namespace std;

string filename = "raw_data/box_rt.dat";

vector<Segment> segments;

int main() {
    ifstream rt_data(filename);

    vector<vector<double>> xy_data = rt2xy(rt_data); 

    int tol_t = xy_data.size();
    cout << tol_t << endl;

    vector<Segment> seg = segment(xy_data);

} 
