#include <bits/stdc++.h>
#include "data_format.h"
#include "preprocessing.h"
#include "feature.h"
#include "model.h"
using namespace std;

// choose bot type
string bot = "turtlebot/";
// string bot = "minibot/";

// choose file
// string filename = "raw_data/box_rt.dat";
// string filename = "raw_data/ball_static_rt.dat";
string filename = "raw_data/ball_moving_rt.dat";

string file = bot + filename; 

vector<Segment> segments;

int main() {
    // data segmentation
    /*
    ifstream rt_data(file);

    vector<vector<double>> xy_data = rt2xy(rt_data); 

    int tol_t = xy_data.size();
    cout << tol_t << endl;

    vector<Segment> seg = segment(xy_data, true);
    */ 
   
    // feature extraction 
    ifstream labeled_data(bot + "labeled_data/box_labeled.dat");
    vector<vector<double>> output = feature_extraction(labeled_data);
    
    cout << "Finish extracting features!\n";

    vector<double> labels=output[0];

    cout<<"Save\n";
	
    vector<vector<double>> features;
    for(int i=0;i<5;i++){
    	features.push_back(output[i+1]);
    }

    
    // training
    adaboost(features,labels);
} 
