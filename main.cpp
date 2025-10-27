#include <bits/stdc++.h>
#include "data_manipulation.h"
using namespace std;

string filename = "raw_data/box.dat";

struct Point {double x, y;};
struct Segment {Point p; int label;};

vector<Segment> segments;

int main() {
    ifstream rt_data(filename);

    vector<vector<double>> xy_data = rt2xy(rt_data); 
}
