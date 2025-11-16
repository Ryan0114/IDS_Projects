#include <bits/stdc++.h>
#include "model.h"
using namespace std;

vector<WeakClassifier> adaboost_train(vector<vector<double>> &features, int T) {
    int F = features.size() - 1; // exclude label
    int N = features[0].size();

    vector<int> y(N);
    vector<vector<double>> X(N, vector<double>(F));
    for (int i = 0; i < N; i++) {
        y[i] = (features[0][i] == 1 ? 1 : -1); // convert label 0->-1
        for (int j = 1; j <= F; j++)
            X[i][j-1] = features[j][i];
    }

    vector<double> w(N, 1.0 / N);
    vector<WeakClassifier> model;

    for (int t = 0; t < T; t++) {
        double best_error = 1e9;
        WeakClassifier best;

        for (int j = 0; j < F; j++) {
            // Generate candidate thresholds
            vector<pair<double,int>> values;
            for (int i = 0; i < N; i++)
                values.emplace_back(X[i][j], y[i]);
            sort(values.begin(), values.end());

            for (int k = 0; k < N-1; k++) {
                double thr = (values[k].first + values[k+1].first)/2.0;
                for (int polarity : {1, -1}) {
                    double err = 0.0;
                    for (int i = 0; i < N; i++) {
                        int pred = (polarity * (X[i][j] < thr ? 1 : -1));
                        if (pred != y[i])
                            err += w[i];
                    }
                    if (err < best_error) {
                        best_error = err;
                        best = {j, thr, polarity, 0};
                    }
                }
            }
        }

        // Avoid division by zero
        best_error = max(1e-9, min(best_error, 1 - 1e-9));
        best.alpha = 0.5 * log((1 - best_error) / best_error);

        // Update weights
        double Z = 0;
        for (int i = 0; i < N; i++) {
            int pred = best.polarity * (X[i][best.feature_index] < best.threshold ? 1 : -1);
            w[i] *= exp(-best.alpha * y[i] * pred);
            Z += w[i];
        }
        for (double &wi : w) wi /= Z;

        model.push_back(best);
        cerr << "Round " << t+1 << ": feature " << best.feature_index
             << ", thr=" << best.threshold
             << ", alpha=" << best.alpha
             << ", error=" << best_error << endl;
    }
    return model;
}

int adaboost_predict(const vector<double> &x, const vector<WeakClassifier> &model) {
    double sum = 0;
    for (auto &m : model) {
        int pred = m.polarity * (x[m.feature_index] < m.threshold ? 1 : -1);
        sum += m.alpha * pred;
    }
    return sum >= 0 ? 1 : 0;
}

