#pragma once
#include <bits/stdc++.h>
using namespace std;

// Naive Bayes
struct NB {
    double mean, sigma;
}; 

NB NB_train(vector<double> &data);
double NB_eval(vector<vector<double>> &test_data);

// (DT-BF based adaboost)
pair<vector<double>, vector<NB>> adaboost(vector<vector<double>> &features);
