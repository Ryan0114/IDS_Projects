#pragma once
#include "data_format.h"
using namespace std;

vector<Segment> read_labeled_file(ifstream &fin);
vector<vector<double>> feature_extraction(vector<Segment> segments);
