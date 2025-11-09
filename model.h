#pragma once
#include <vector>
#include <cmath>

// 三類別標籤
enum ObjectClass {
    NONE = 0,    // 都不是
    BALL = 1,    // 球
    BOX = 2      // 箱子
};

// 決策樹樁（單層決策樹）
struct DecisionStump {
    int feature_idx;    // 使用的特徵索引
    double threshold;   // 決策閾值
    int direction;      // 方向：1表示大於閾值為正類，-1表示小於閾值為正類
    int target_class;   // 目標類別（用於一對多分類）
};

// 為了保持文件格式兼容性
struct NB {
    double mean, sigma; // mean存儲threshold，sigma存儲direction和target_class的編碼
}; 

// 三類別分類的訓練函數
std::pair<std::vector<double>, std::vector<std::vector<NB>>> 
adaboost(std::vector<std::vector<double>> &features, std::vector<double> &labels);

// 三類別預測函數
int adaboost_predict(std::vector<double> &sample, 
                    std::vector<double> &alphas, 
                    std::vector<std::vector<NB>> &nbs);

// 輔助函數
DecisionStump train_decision_stump(const std::vector<std::vector<double>>& features,
                                 const std::vector<double>& labels,
                                 const std::vector<double>& weights,
                                 int target_class);
void encode_stump_to_NB(const DecisionStump& stump, NB& nb);
void decode_NB_to_stump(const NB& nb, DecisionStump& stump);
int stump_predict(double feature_value, const DecisionStump& stump);
double calculate_stump_error(const std::vector<std::vector<double>>& features,
                           const std::vector<double>& labels,
                           const std::vector<double>& weights,
                           const DecisionStump& stump);
