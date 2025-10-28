#pragma once
#include <bits/stdc++.h>
#include "data_format.h"
using namespace std;

vector<vector<double>> rt2xy(ifstream &fin);
// segmentation
vector<Segment> segment(vector<vector<double>> &xy_data);
