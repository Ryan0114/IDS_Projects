#include <iostream>
#include <fstream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include "data_format.h"
#include "model.h"

using namespace std;

DecisionStump train_decision_stump_simple(const vector<vector<double>>& features,
                                        const vector<double>& labels,
                                        const vector<double>& weights) {
    DecisionStump best_stump;
    double best_error = 1.0;
    int n_samples = features.size();
    
    // 【修改】：只使用第3个（索引2）和第5个（索引4）特征
    vector<int> selected_features = {2, 4};
    
    // 為選定的特徵尋找最佳閾值
    for (int feature_idx : selected_features) {
        
        // 收集當前特徵的所有唯一值作為候選閾值
        vector<double> unique_values;
        for (int i = 0; i < n_samples; i++) {
            unique_values.push_back(features[i][feature_idx]);
        }
        
        sort(unique_values.begin(), unique_values.end());
        unique_values.erase(unique(unique_values.begin(), unique_values.end()), unique_values.end());
        
        // 使用少量候選閾值（避免過度計算）
        vector<double> candidate_thresholds;
        int step = max(1, (int)unique_values.size() / 20);
        for (int i = 0; i < unique_values.size(); i += step) {
            candidate_thresholds.push_back(unique_values[i]);
        }
        
        // 確保至少有3個候選閾值
        if (candidate_thresholds.size() < 3 && !unique_values.empty()) {
            candidate_thresholds.push_back(unique_values[0]);
            candidate_thresholds.push_back(unique_values[unique_values.size()/2]);
            candidate_thresholds.push_back(unique_values.back());
        }
        
        // 測試每個候選閾值
        for (double threshold : candidate_thresholds) {
            
            // 【修正 1】：排除 Feature 2 (Sc) 的 0.0 門檻
            if (feature_idx == 2 && threshold == 0.0) {
                continue; 
            }
            
            DecisionStump stump;
            stump.feature_idx = feature_idx;
            stump.threshold = threshold;
            
            // 測試兩個方向
            for (int direction : {1, -1}) {
                stump.direction = direction;
                
                // 【修正 2】：在二元分類中，弱分類器的目標類別永遠是 1 (代表 Positive)
                stump.target_class = 1; 
                
                // 計算錯誤率
                double error = 0.0;
                for (int i = 0; i < n_samples; i++) {
                    // 根據當前樹樁預測是否為 1 (Positive)
                    int prediction = (stump.direction > 0) ? 
                        (features[i][feature_idx] > threshold ? 1 : 0) :
                        (features[i][feature_idx] < threshold ? 1 : 0);
                    
                    // 檢查預測是否與訓練標籤 (labels[i] 只有 0 或 1) 不符
                    if (prediction != static_cast<int>(round(labels[i]))) { 
                        error += weights[i];
                    }
                }
                
                if (error < best_error) {
                    best_error = error;
                    best_stump = stump;
                }
            }
        }
    }
    
    return best_stump;
}

// 修復的編碼函數
void encode_stump_to_NB_fixed(const DecisionStump& stump, NB& nb) {
    nb.mean = stump.threshold;
    
    // 簡單編碼：feature_idx * 100 + target_class * 10 + (direction > 0 ? 1 : 0)
    double encoded = stump.feature_idx * 100 + stump.target_class * 10 + (stump.direction > 0 ? 1 : 0);
    nb.sigma = encoded;
}

// 修復的解碼函數
void decode_NB_to_stump_fixed(const NB& nb, DecisionStump& stump) {
    stump.threshold = nb.mean;
    
    int code = static_cast<int>(nb.sigma);
    stump.feature_idx = code / 100;
    stump.target_class = (code % 100) / 10;
    stump.direction = (code % 10 == 1) ? 1 : -1;
}

// 修復的訓練函數
pair<vector<double>, vector<vector<NB>>> adaboost(vector<vector<double>> &features, vector<double> &labels) {
    cout << "=== Starting Fixed AdaBoost Training (Using Only Features 2 and 4) ===" << endl;
    
    // 基本檢查
    if (features.empty() || labels.empty()) {
        cerr << "ERROR: Empty features or labels!" << endl;
        return make_pair(vector<double>(), vector<vector<NB>>());
    }
    
    int n_samples = features.size();
    int n_features = features[0].size();
    
    cout << "Training data: " << n_samples << " samples, " << n_features << " features" << endl;
    cout << "Using only features: 2 and 4" << endl;
    
    // 檢查標籤分布
    vector<int> label_count(3, 0);
    for (double label : labels) {
        if (label >= 0 && label < 3) {
            label_count[label]++;
        }
    }
    
    // 檢查特徵範圍（只檢查使用的特徵）
    vector<int> selected_features = {2, 4};
    for (int feature_idx : selected_features) {
        double min_val = 1e10, max_val = -1e10;
        for (int j = 0; j < n_samples; j++) {
            if (features[j][feature_idx] < min_val) min_val = features[j][feature_idx];
            if (features[j][feature_idx] > max_val) max_val = features[j][feature_idx];
        }
        cout << "Feature " << feature_idx << " range: [" << min_val << ", " << max_val << "]" << endl;
    }
    
    // 初始化權重
    vector<double> weights(n_samples, 1.0 / n_samples);
    vector<double> alphas;
    vector<vector<NB>> nbs;
    
    // 訓練固定數量的分類器
    int total_classifiers = 30;
    
    for (int round = 0; round < total_classifiers; round++) {
        cout << "Round " << round << endl;
        
        // 訓練決策樹樁
        DecisionStump stump = train_decision_stump_simple(features, labels, weights);
        
        cout << "  Stump: feature=" << stump.feature_idx 
             << ", threshold=" << stump.threshold 
             << ", direction=" << stump.direction 
             << ", target_class=" << stump.target_class << endl;
        
        // 計算錯誤率
        double error = 0.0;
        for (int i = 0; i < n_samples; i++) {
            int prediction = (stump.direction > 0) ? 
                (features[i][stump.feature_idx] > stump.threshold ? 1 : 0) :
                (features[i][stump.feature_idx] < stump.threshold ? 1 : 0);
            
            int true_label = (labels[i] == stump.target_class) ? 1 : 0;
            if (prediction != true_label) {
                error += weights[i];
            }
        }
        
        cout << "  Error: " << error << endl;
        
        // 避免極端錯誤率
        error = max(min(error, 0.999), 0.001);
        
        // 計算alpha
        double alpha = 0.5 * log((1.0 - error) / error);
        alpha = max(alpha, 0.1);
        
        cout << "  Alpha: " << alpha << endl;
        
        alphas.push_back(alpha);
        
        // 編碼並保存
        vector<NB> feature_nb(5);
        NB encoded_nb;
        encode_stump_to_NB_fixed(stump, encoded_nb);
        feature_nb[stump.feature_idx] = encoded_nb;
        nbs.push_back(feature_nb);
        
        // 更新權重
        double sum_weights = 0.0;
        for (int i = 0; i < n_samples; i++) {
            int prediction = (stump.direction > 0) ? 
                (features[i][stump.feature_idx] > stump.threshold ? 1 : 0) :
                (features[i][stump.feature_idx] < stump.threshold ? 1 : 0);
            
            int true_label = (labels[i] == stump.target_class) ? 1 : 0;
            if (prediction != true_label) {
                weights[i] *= exp(alpha);
            } else {
                weights[i] *= exp(-alpha);
            }
            sum_weights += weights[i];
        }
        
        // 歸一化
        for (int i = 0; i < n_samples; i++) {
            weights[i] /= sum_weights;
        }
    }
    
    // 檢查特徵值是否都一樣（只檢查使用的特徵）
    vector<int> used_features = {2, 4};
    for (int i : used_features) {
        bool all_same = true;
        double first_val = features[0][i];
        for (size_t j = 1; j < features.size(); j++) {
            if (features[j][i] != first_val) {
                all_same = false;
                break;
            }
        }
        if (all_same) {
            cout << "feature:" << i << "all value are same: " << first_val << endl;
        }
    }

    return make_pair(alphas, nbs);
}

// 修復的預測函數（不需要修改，因為解碼後的特徵索引自然會限制在2和4）
int adaboost_predict_fixed(vector<double> &sample, vector<double> &alphas, vector<vector<NB>> &nbs) {
    vector<double> scores(3, 0.0);  // NONE, BALL, BOX
    
    for (size_t i = 0; i < alphas.size(); i++) {
        DecisionStump stump;
        for (int j = 0; j < 5; j++) {
            if (nbs[i][j].sigma > 0) {
                decode_NB_to_stump_fixed(nbs[i][j], stump);
                break;
            }
        }
        
        if (stump.feature_idx < sample.size()) {
            int prediction = (stump.direction > 0) ? 
                (sample[stump.feature_idx] > stump.threshold ? 1 : 0) :
                (sample[stump.feature_idx] < stump.threshold ? 1 : 0);
            
            if (prediction == 1) {
                scores[stump.target_class] += alphas[i];
            }
        }
    }
    
    // 返回得分最高的類別
    return max_element(scores.begin(), scores.end()) - scores.begin();
}
