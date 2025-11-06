#include <bits/stdc++.h>
#include "data_format.h"
#include "model.h"
using namespace std;

#define PI acos(-1)

NB NB_train(vector<double> &data) {
    int n = data.size();

    NB nb;
    double mean=0, sigma;
    for (int i=0; i<n; i++) {
        mean += data[i];
    }
    mean /= data.size();

    for (int i=0; i<n; i++) {
        sigma += (data[i] - mean) * (data[i] - mean);
    }
    sigma = sqrt(sigma/n);

    nb.mean = mean;
    nb.sigma = sigma;
    return nb;
}

double NB_eval(double x, NB nb) {
    return exp(-(x-nb.mean)*(x-nb.mean)/(2*nb.sigma*nb.sigma)) / (nb.sigma * sqrt(2*PI));
}

// (DT-BF based adaboost)
pair<vector<double>, vector<NB>> adaboost(vector<vector<double>> &features) {
    NB nb = NB_train(features[2]);
    cout << NB_eval(3) << endl;
}
