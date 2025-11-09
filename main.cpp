#include <bits/stdc++.h>
#include "data_format.h"
#include "preprocessing.h"
#include "feature.h"
#include "model.h"
using namespace std;

string bot = "turtlebot/";

void train_detector(const string& data_file, const string& model_name, const string& object_type) {
    cout << "=== Training " << object_type << " detector ===" << endl;
    
    cout << "Loading data: " << data_file << endl;
    ifstream labeled_data(bot + "labeled_data/" + data_file);
    
    if (!labeled_data.is_open()) {
        cout << "Error: Cannot open file: " << data_file << endl;
        return;
    }
    
    cout << "Extracting features..." << endl;
    // output[0] = labels, output[1]..[5] = features
    vector<vector<double>> output = feature_extraction(read_labeled_file(labeled_data));
    
    cout << "Number of output vectors: " << output.size() << endl;
    // 檢查 output 向量是否包含足夠的特徵和標籤
    if (output.size() < 6) {
        cout << "Error: Feature extraction failed, expected at least 6 vectors (1 label + 5 features)." << endl;
        return;
    }

    // 原始標籤：output[0]
    vector<double> labels_raw = output[0];
    vector<vector<double>> features_by_feature;
    
    // 提取 5 個特徵 (features_by_feature[0] 是 feature 1 的所有樣本值，以此類推)
    for(int i = 0; i < 5; i++){
        features_by_feature.push_back(output[i+1]);
    }
    
    // =========================================================================
    // 【修正 1】: 標籤二元化 (訓練時，標籤 1 視為正樣本)
    // =========================================================================
    vector<double> labels; // AdaBoost 訓練需要的二元標籤 (1.0 或 0.0)
    int pos_count = 0, neg_count = 0;
    
    for (double label : labels_raw) {
        if (label == 1.0) { // 原始標籤 1 (Box 或 Ball) 視為正樣本
            labels.push_back(1.0);
            pos_count++;
        } else { // 原始標籤 0 視為負樣本
            labels.push_back(0.0);
            neg_count++;
        }
    }
    
    // -------------------------------------------------------------------------
    // 準備訓練：將特徵矩陣轉置成 (n_samples x n_features) 格式
    // -------------------------------------------------------------------------

    if (features_by_feature.empty()) {
        cout << "Error: Features are empty!" << endl;
        return;
    }
    
    cout << "Label distribution - Positive(Label 1): " << pos_count 
         << ", Negative: " << neg_count << endl;

    cout << "Feature matrix: " << features_by_feature.size() << " features x " << features_by_feature[0].size() << " samples" << endl;
    
    vector<vector<double>> transposed_features;
    int n_samples = features_by_feature[0].size();
    int n_features = features_by_feature.size();
    
    for (int i = 0; i < n_samples; i++) {
        vector<double> sample;
        for (int j = 0; j < n_features; j++) {
            sample.push_back(features_by_feature[j][i]);
        }
        transposed_features.push_back(sample);
    }
    
    cout << "After transpose: " << transposed_features.size() << " samples x " 
         << (transposed_features.empty() ? 0 : transposed_features[0].size()) << " features" << endl;
    
    cout << "Training " << object_type << " detector..." << endl;
    auto [alphas, nbs] = adaboost(transposed_features, labels);
    
    // =========================================================================
    // 【修正 2】: 模型儲存邏輯 (解決 .dat 文件只剩一個數字的問題 & target_class 修正)
    // =========================================================================

    // [2.1] 硬編碼 target_to_save 以區分 Box (1) 和 Ball (2)
    int target_to_save;
    if (object_type == "Box") {
        target_to_save = 1; 
    } else if (object_type.find("Ball") != string::npos) {
        // "Moving Ball" 和 "Static Ball" 都應使用不同的 target_class (2)
        target_to_save = 2; 
    } else {
        target_to_save = 0; 
    }

    string model_filename = model_name + ".dat";
    ofstream model_file(model_filename);
    
    if (model_file.is_open()) {
        model_file << alphas.size() << "\n";
        
        // 寫入每個弱分類器的迴圈 (恢復並修正)
        for (size_t i = 0; i < alphas.size(); i++) {
            
            // 安全檢查 (防止 nbs 結構問題導致崩潰)
            if (i >= nbs.size() || nbs[i].size() != features_by_feature.size()) { 
                 model_file << "0 0 0.0 " << (target_to_save * 10) << "\n"; 
                 continue; 
            }
            
            model_file << alphas[i] << " ";
            
            int feature_idx = -1;
            // 尋找被選中的特徵 (sigma > 0)
            for (int j = 0; j < n_features; j++) {
                if (abs(nbs[i][j].sigma) > numeric_limits<double>::epsilon()) { // 使用安全比較
                    feature_idx = j;
                    break;
                }
            }
            
            if (feature_idx != -1) {
                
                double original_mean = nbs[i][feature_idx].mean;
                int original_code = static_cast<int>(round(nbs[i][feature_idx].sigma)); 
                
                // 解碼方向 (direction code)
                int direction_code = original_code % 10;
                
                // 重新編碼 sigma，使用正確的目標類別 target_to_save (1 或 2)
                double correct_sigma = feature_idx * 100 + target_to_save * 10 + direction_code;
                
                model_file << feature_idx << " " << original_mean
                           << " " << correct_sigma << "\n";
            } else {
                // 如果找不到有效的弱分類器，使用默認值 (防止崩潰，確保有寫入一行)
                model_file << "0 0.0 " << (target_to_save * 10) << "\n"; 
            }
        }
        model_file.close();
        cout << object_type << " detector saved to: " << model_filename << endl;
    } else {
        cout << "Error: Cannot save model: " << model_filename << endl;
    }
    
    cout << object_type << " detector training completed. " << alphas.size() << " weak classifiers obtained.\n" << endl;
}

// main 函數保持不變
int main() {
    cout << "=== Starting training for three independent detectors ===" << endl;
    
    train_detector("box_labeled.dat", "box", "Box");
    train_detector("ball_moving_labeled.dat", "ball_moving", "Moving Ball");
    train_detector("ball_static_labeled.dat", "ball_static", "Static Ball");
    
    cout << "=== All detectors training completed ===" << endl;
    return 0;
}
