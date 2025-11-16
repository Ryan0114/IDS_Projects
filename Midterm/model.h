#pragma once
#include <bits/stdc++.h>
using namespace std;
/*
// Naive Bayes
struct NB {
    double mean, sigma;
}; 

NB NB_train(vector<double> &data);
double NB_eval(vector<vector<double>> &test_data);

// (DT-BF based adaboost)
pair<vector<double>, vector<vector<NB>>> adaboost(vector<vector<double>> &features, vector<double> &labels);
int adaboost_predict(vector<double> &sample, vector<double> &alphas, vector<vector<NB>> &nbs); 
 
LogisticModel train_logistic(const vector<vector<double>>& X, const vector<int>& y, double lr = 0.01, int epochs = 1000);
void save_model(const string& filename, const LogisticModel& model);
*/ 

struct WeakClassifier {
    int feature_index;
    double threshold;
    int polarity;    // +1 or -1
    double alpha;
};

vector<WeakClassifier> adaboost_train(vector<vector<double>> &features, int T);
int adaboost_predict(const vector<double> &x, const vector<WeakClassifier> &model);
