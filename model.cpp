#include <bits/stdc++.h>
#include "data_format.h"
#include "model.h"
using namespace std;

NB NB_train(vector<double> &data) {
    // TODO find mean and sigma of data
    NB nb;
    double mean = 0;
    double sigma = 0;

    nb.mean = mean;
    nb.sigma = sigma;
    return nb;
}

double NB_eval(vector<vector<double>> &test_data) {

}

// (DT-BF based adaboost)
pair<vector<double>, vector<NB>> adaboost(vector<vector<double>> &features) {

}
