#include <bits/stdc++.h>
#include "data_format.h"
#include "model.h"
using namespace std;

#define PI acos(-1)

void adaboost_save_model(string filename, vector<double> &alphas, vector<vector<NB>> &nbs);

NB NB_train(vector<double> &data) {
    int n = data.size();
    NB nb;
    double mean = 0, sigma = 0;
    
    for (int i = 0; i < n; i++) {
        mean += data[i];
    }
    mean /= data.size();

    for (int i = 0; i < n; i++) {
        sigma += (data[i] - mean) * (data[i] - mean);
    }
    sigma = sqrt(sigma / n);

    nb.mean = mean;
    nb.sigma = sigma;
    return nb;
}

double NB_eval(double x, NB nb) {
    return exp(-(x - nb.mean) * (x - nb.mean) / (2 * nb.sigma * nb.sigma)) / (nb.sigma * sqrt(2 * PI));
}

NB NB_train_weighted(vector<double> &data, vector<double> &weights) {
    int n = data.size();
    NB nb;
    double mean = 0, sigma = 0;
    double total_weight = 0;
    
    for (int i = 0; i < n; i++) {
        mean += data[i] * weights[i];
        total_weight += weights[i];
    }
    mean /= total_weight;

    for (int i = 0; i < n; i++) {
        sigma += weights[i] * (data[i] - mean) * (data[i] - mean);
    }
    sigma = sqrt(sigma / total_weight);

    nb.mean = mean;
    nb.sigma = sigma;
    return nb;
}

pair<vector<double>, vector<vector<NB>>> adaboost(vector<vector<double>> &features, vector<double> &labels) {
    int n_samples = features.size();
    vector<double> weights(n_samples, 1.0 / n_samples);
    vector<double> alphas;
    vector<vector<NB>> nbs;
    
    cout << "Training...\n";

    for (int feature_idx = 0; feature_idx < 5; feature_idx++) {
        vector<double> feature_values(n_samples);
        for (int i = 0; i < n_samples; i++) {
            feature_values[i] = features[i][feature_idx];
        }
        
        NB nb = NB_train_weighted(feature_values, weights);
        
        double error = 0.0;
        for (int i = 0; i < n_samples; i++) {
            double score = NB_eval(feature_values[i], nb);
            int prediction = (score > 0.5) ? 1 : 0;
            if (prediction != labels[i]) {
                error += weights[i];
            }
        }
        
        double alpha = 0.5 * log((1.0 - error) / (error + 1e-10));
        alphas.push_back(alpha);
        
        vector<NB> feature_nb(5);
        feature_nb[feature_idx] = nb;
        nbs.push_back(feature_nb);
        
        double sum_weights = 0.0;
        for (int i = 0; i < n_samples; i++) {
            double score = NB_eval(feature_values[i], nb);
            int prediction = (score > 0.5) ? 1 : 0;
            
            if (prediction != labels[i]) {
                weights[i] *= exp(alpha);
            } else {
                weights[i] *= exp(-alpha);
            }
            sum_weights += weights[i];
        }
        
        for (int i = 0; i < n_samples; i++) {
            weights[i] /= sum_weights;
        }
    }


    adaboost_save_model("adaboost_NB.dat", alphas, nbs);
    return make_pair(alphas, nbs);
}

int adaboost_predict(vector<double> &sample, vector<double> &alphas, vector<vector<NB>> &nbs) {
    double total_score = 0.0;
    double total_alpha = 0.0;
    
    for (size_t i = 0; i < alphas.size(); i++) {
        int feature_idx = -1;
        for (int j = 0; j < 5; j++) {
            if (nbs[i][j].sigma > 0) {
                feature_idx = j;
                break;
            }
        }
        
        if (feature_idx == -1) continue;
        
        double score = NB_eval(sample[feature_idx], nbs[i][feature_idx]);
        int prediction = (score > 0.5) ? 1 : 0;
        
        total_score += alphas[i] * prediction;
        total_alpha += alphas[i];
    }
    
    
    return (total_score > total_alpha * 0.5) ? 1 : 0;
}

void adaboost_save_model(string filename, vector<double> &alphas, vector<vector<NB>> &nbs) {
    ofstream file(filename);
    if (!file.is_open()) return;
    
    file << alphas.size() << "\n";
    
    for (size_t i = 0; i < alphas.size(); i++) {
        file << alphas[i] << " ";
        
        int feature_idx = -1;
        for (int j = 0; j < 5; j++) {
            if (nbs[i][j].sigma > 0) {
                feature_idx = j;
                break;
            }
        }
        
        file << feature_idx << " ";
        
        if (feature_idx != -1) {
            file << nbs[i][feature_idx].mean << " " << nbs[i][feature_idx].sigma << "\n";
        }
    }
    
    file.close();
}

