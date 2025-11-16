#include <bits/stdc++.h>
#include <Eigen/Dense>
#include "data_format.h"
#include "feature.h"
using namespace std;
using namespace Eigen;

vector<Segment> read_labeled_file(ifstream &fin) {
    vector<Segment> segments;
    Segment seg;
    int Sn, si_n, label;
    double x, y;
    while (fin >> Sn) {
        for (int i=0; i<Sn; i++) {
            fin >> si_n >> label;
            seg.label = label;
            for (int j=0; j<si_n; j++) {
                fin >> x >> y;
                seg.points.push_back(Point(x, y));
                seg.size++;
            }
            segments.push_back(seg);
            seg.points.clear();
            seg.size = 0;
        }
    } 

    return segments;
}


vector<vector<double>> feature_extraction(vector<Segment> segments) {
    int seg_num = segments.size();
    vector<vector<double>> features(14, vector<double>(seg_num, 0.0));
    // features:
    // 0: label
    // 1: n_points
    // 2: circle_radius
    // 3: circle_residual
    // 4: std_var (dispersion)
    // 5: centroid_distance
    // 6: line_residual
    // 7: ratio_line_circle
    // 8: eccentricity (lambda1/lambda2)
    // 9: mean_curvature
    // 10: std_curvature
    // 11: chord_length
    // 12: arc_length
    // 13: arc_chord_ratio

    for (int i = 0; i < seg_num; i++) {
        int n = segments[i].points.size();
        if (n < 3) continue; // too few for geometric features

        features[0][i] = segments[i].label;
        features[1][i] = n;

        // ---------- Circle fitting ----------
        MatrixXd A(n, 3);
        VectorXd b(n);
        for (int j = 0; j < n; j++) {
            double x = segments[i].points[j].x;
            double y = segments[i].points[j].y;
            A(j, 0) = -2 * x;
            A(j, 1) = -2 * y;
            A(j, 2) = 1;
            b(j) = -(x * x + y * y);
        }

        VectorXd sol = A.colPivHouseholderQr().solve(b);
        double x_c = sol(0);
        double y_c = sol(1);
        double r_c = sqrt(x_c * x_c + y_c * y_c - sol(2));
        features[2][i] = r_c;

        double s_c = 0;
        for (auto &p : segments[i].points) {
            double d = hypot(p.x - x_c, p.y - y_c);
            s_c += (r_c - d) * (r_c - d);
        }
        features[3][i] = s_c;

        // ---------- Centroid & dispersion ----------
        double mean_x = 0, mean_y = 0;
        for (auto &p : segments[i].points) {
            mean_x += p.x;
            mean_y += p.y;
        }
        mean_x /= n; mean_y /= n;

        double var_sum = 0;
        for (auto &p : segments[i].points)
            var_sum += (p.x - mean_x)*(p.x - mean_x) + (p.y - mean_y)*(p.y - mean_y);
        double std_var = sqrt(var_sum / n);
        features[4][i] = std_var;
        features[5][i] = hypot(mean_x, mean_y);

        // ---------- Line fitting ----------
        Matrix2d C = Matrix2d::Zero();
        for (auto &p : segments[i].points) {
            Vector2d d(p.x - mean_x, p.y - mean_y);
            C += d * d.transpose();
        }
        SelfAdjointEigenSolver<Matrix2d> eig(C);
        Vector2d eigval = eig.eigenvalues();
        double lambda1 = eigval(1), lambda2 = eigval(0);
        double eccentricity = lambda1 / lambda2;
        features[8][i] = eccentricity;

        Vector2d normal = eig.eigenvectors().col(0); // small eigenvalue = normal
        double d_line = -(normal.dot(Vector2d(mean_x, mean_y)));
        double line_residual = 0;
        for (auto &p : segments[i].points) {
            double dis = fabs(normal.dot(Vector2d(p.x, p.y)) + d_line);
            line_residual += dis * dis;
        }
        features[6][i] = line_residual;
        features[7][i] = (s_c > 1e-9) ? line_residual / s_c : 0.0;

        // ---------- Curvature ----------
        vector<double> curv;
        for (int j = 1; j < n - 1; j++) {
            double x1 = segments[i].points[j - 1].x, y1 = segments[i].points[j - 1].y;
            double x2 = segments[i].points[j].x,     y2 = segments[i].points[j].y;
            double x3 = segments[i].points[j + 1].x, y3 = segments[i].points[j + 1].y;
            double a = hypot(x2 - x1, y2 - y1);
            double b = hypot(x3 - x2, y3 - y2);
            double c = hypot(x3 - x1, y3 - y1);
            double area = fabs((x1*(y2 - y3) + x2*(y3 - y1) + x3*(y1 - y2)) / 2.0);
            double curvature = (a * b * c < 1e-6) ? 0.0 : (4 * area) / (a * b * c);
            curv.push_back(curvature);
        }
        double mean_curv = accumulate(curv.begin(), curv.end(), 0.0) / curv.size();
        double var_curv = 0;
        for (double c : curv) var_curv += (c - mean_curv) * (c - mean_curv);
        var_curv = sqrt(var_curv / curv.size());
        features[9][i] = mean_curv;
        features[10][i] = var_curv;

        // ---------- Arc & chord lengths ----------
        double chord = hypot(segments[i].points.front().x - segments[i].points.back().x,
                             segments[i].points.front().y - segments[i].points.back().y);
        double arc = 0;
        for (int j = 0; j < n - 1; j++) {
            arc += hypot(segments[i].points[j + 1].x - segments[i].points[j].x,
                         segments[i].points[j + 1].y - segments[i].points[j].y);
        }
        features[11][i] = chord;
        features[12][i] = arc;
        features[13][i] = (chord > 1e-6) ? arc / chord : 1.0;
    }

    return features;
}

