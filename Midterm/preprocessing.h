#pragma once
#include <bits/stdc++.h>
#include "data_format.h"
using namespace std;

vector<vector<double>> read_rt_file(ifstream &fin);
vector<vector<double>> rt2xy(vector<vector<double>>);
// segmentation
vector<Segment> segment(vector<vector<double>> &xy_data, bool output);
