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
    vector<vector<double>> features(6, vector<double> (seg_num));
    // features: number of points, radius, curvature, stadard value, distance of the center of circle to the origin
     
    for (int i=0; i<seg_num; i++) {
        int n = segments[i].points.size();
        features[0][i] = segments[i].label;
        features[1][i] = n;

        MatrixXd A(n, 3);
        VectorXd b(n);
        for (int j=0; j<n; j++) {
            double x = segments[i].points[j].x;
            double y = segments[i].points[j].y;
            A(j,0) = -2*x;
            A(j,1) = -2*y;
            A(j,2) = 1;
            b(j) = -(x*x + y*y);
        }

        VectorXd x = A.colPivHouseholderQr().solve(b); 
               

        double x_c = x(0);
        double y_c = x(1);
        double r_c = sqrt(x_c*x_c+y_c*y_c-x(2));
        
        double s_c = 0;
        for (int j=0; j<segments[i].size; j++) {
            double dis = Point(x_c, y_c).dist_to(segments[i].points[j]);
            s_c += (r_c - dis) * (r_c - dis);
        }

        features[2][i] = r_c;
        features[3][i] = s_c;

        double sum_x=0.0;
        double sum_y=0.0;

        for (int j = 0; j < n; j++) {
            sum_x += segments[i].points[j].x;
            sum_y += segments[i].points[j].y;
        }

        double mean_x = sum_x / n;
        double mean_y = sum_y / n;
        double distance_sum=0.0;

        for (int j=0;j<n;j++){
            double dx = segments[i].points[j].x-mean_x;
            double dy = segments[i].points[j].y-mean_y;
            double distance = dx*dx+dy*dy;
            distance_sum+=distance;
        }
        
        double std_var=sqrt(distance_sum/n);
        features[4][i]=std_var;

        double distance_mean=sqrt(mean_x*mean_x+mean_y*mean_y);
        features[5][i]=distance_mean;
    }

    return features;
}
