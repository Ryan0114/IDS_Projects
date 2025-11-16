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
string filename = "raw_data/box_rt.dat";
// string filename = "raw_data/ball_static_rt.dat";
// string filename = "raw_data/ball_moving_rt.dat";

string file = bot + filename; 

vector<Segment> segments;

int main() {
    // data segmentation
    cout << "Loading file..." << endl;
    ifstream rt_data(file);
    
    vector<vector<double>> xy_data = rt2xy(read_rt_file(rt_data)); 
    

    int tol_t = xy_data.size();
    cout << tol_t << endl;
    
    cout << "Segmenting data..." << endl;
    vector<Segment> seg = segment(xy_data, true);
    
    /*   
    // feature extraction 
    cout << "Loading files..." << endl; 
    ifstream box_labeled_data(bot + "labeled_data/box_labeled.dat");
    // ifstream ball_static_labeled_data(bot + "labeled_data/ball_static_labeled.dat");
    ifstream ball_moving_labeled_data(bot + "labeled_data/ball_moving_labeled.dat");

    cout << "Extracting features..." << endl;
    vector<vector<double>> output = feature_extraction(read_labeled_file(box_labeled_data));
    // vector<vector<double>> output2 = feature_extraction(read_labeled_file(ball_static_labeled_data));
    vector<vector<double>> output3 = feature_extraction(read_labeled_file(ball_moving_labeled_data));

    cout << "finish extracting features." << endl;

    int n1 = output[0].size();
    // int n2 = output2[0].size();
    int n3 = output3[0].size();
    
    vector<double> labels=output[0];
    // vector<double> labels2=output2[0];
    vector<double> labels3=output3[0];

    // for (int i=0; i<n2; i++) {labels2[i] = (labels2[i]==1 ? 2: 0); labels.push_back(labels2[i]);}
    for (int i=0; i<n3; i++) {labels3[i] = (labels3[i]==1 ? 2: 0); labels.push_back(labels3[i]);}
	
    vector<vector<double>> features;
    cout << output.size() << " " << output[0].size() << endl;
    for(int i=0; i<output.size()-1; i++) features.push_back(output[i+1]);
    // for(int i=0; i<5; i++) features[i].insert(features[i].end(), output2[i+1].begin(), output2[i+1].end());
    for(int i=0; i<output.size()-1; i++) features[i].insert(features[i].end(), output3[i+1].begin(), output3[i+1].end());
    

    vector<vector<double>> feats(features.size());
    for (int i=0; i<features[i].size(); i++) {
        if (features[0][i]!=1) {
            for (int j=0; j<features.size(); j++) {
                feats[j].push_back(features[j][i]);
            }
        }
    }
    */


    // training
    /*
    cout << "Training..." << endl;
    auto result = adaboost(features,labels);
    
    vector<double> alphas = result.first;
    vector<vector<NB>> nbs = result.second;

    int TT=0, TF=0, FT=0, FF=0;
    ofstream file("features.dat");
    for (int i=0; i<features[0].size(); i++) {
        vector<double> feat;
        file << labels[i] << " ";
        for (int j=0; j<features.size(); j++) {
            feat.push_back(features[j][i]);
            file << features[j][i] << " ";
        }
        file << endl;
        int l_predict = adaboost_predict(feat, alphas, nbs);
        if (l_predict==0 && labels[i]==0) FF++;
        else if (l_predict==0 && labels[i]==1) TF++;
        else if (l_predict==1 && labels[i]==0) FT++;
        else TT++;
    }
    file.close();
    cout << "FF " << FF << " TF " << TF << " FT " << FT << " TT " << TT << endl;
    */ 

    /*
    auto model = adaboost_train(feats, 10);

    cout << "feats: " << feats.size() << " " << feats[0].size() << endl;
    for (int i=0; i< feats[0].size(); i++) {
        vector<double> x;
        for (int j=1; j<feats.size(); j++) x.push_back(feats[j][i]);
        cout << i << endl;
        int pred = adaboost_predict(x, model);
        cout << "Segment " << i << " predicted: " << (pred ? "Ball" : "Box") << " true: " << (feats[0][i] ? "Ball" : "Box") << endl;
    }
    */
}
