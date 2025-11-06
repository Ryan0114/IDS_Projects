#include <bits/stdc++.h>
#include <Eigen/Dense>
#include "data_format.h"
#include "feature.h"
using namespace std;
using namespace Eigen;

vector<vector<double>> feature_extraction(ifstream &fin) {
    vector<Segment> segments;
    
    Segment seg;
    int Sn, si_n, label;
    double x, y;
    while (fin >> Sn) {
        for (int i=0; i<Sn; i++) {
            fin >> si_n >> label;
            for (int j=0; j<si_n; j++) {
                fin >> x >> y;
                seg.points.push_back(Point(x, y));
            }
            segments.push_back(seg);
            seg.points.clear();
            seg.size = 0;
        }
    }  

    int seg_num = segments.size();
    vector<vector<double>> features(seg_num, vector<double> (5));
    // features: number of points, radius, curvature, distance of the center of circle to the origin
     
    cout << "seg_num: " << seg_num << endl;
    for (int i=0; i<seg_num; i++) {
        int n = segments[i].points.size();
        features[i][0] = n;

        if (n < 3) continue;

        MatrixXd A(n, 3);
        VectorXd b(n);
        for (int j=0; j<n; j++) {
            double x = segments[i].points[j].x;
            double y = segments[i].points[j].y;
            A(j,0) = -2*x;
            A(j,1) = -2*y;
            A(j,2) = 1;
            b(j)   = -(x*x + y*y);
        }

        VectorXd x = A.colPivHouseholderQr().solve(b); 
        cout << "Segment " << i << ":\n";
        cout << "x=\n" << x.transpose() << "\n";
        cout << "rank(A)=" << A.fullPivHouseholderQr().rank() << "\n\n";
    }

    return features;
}
