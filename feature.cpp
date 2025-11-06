#include <bits/stdc++.h>
#include <Eigen/Dense>
#include "data_format.h"
#include "feature.h"
using namespace std;
using namespace Eigen;

vector<vector<double>> feature_extraction(ifstream &fin) {
    vector<Segment> segments;
    
    Segment seg;
    int Sn, si_n;
    while (fin >> Sn) {
        for (int i=0; i<Sn; i++) {
            fin >> si_n;
            for (int i=0; i<si_n; i++) {
                double x, y; 
                fin >> x >> y;
                seg.points.push_back(Point(x, y));
            }
            segments.push_back(seg);
        }
    }  

    int seg_num = segments.size();
    vector<vector<double>> features(seg_num, vector<double> (5));
    // features: number of points, radius, curvature, distance of the center of circle to the origin
     
    for (int i=0; i<seg_num; i++) {
        int n = segments[i].size;
        features[i][0] = n;

        MatrixXd A(n, 3);
        VectorXd b(n);
        
        for (int j=0; j<segments[i].size; j++) {
            A(j, 0) = -2*segments[i].points[j].x;
            A(j, 1) = -2*segments[i].points[j].y;
            A(j, 2) = 1;
            b(j) = -segments[i].points[j].x*segments[i].points[j].x-segments[i].points[j].y*segments[i].points[j].y;
        }

        VectorXd x = A.colPivHouseholderQr().solve(b); 
        cout << "A=\n" << A << endl << flush;
        cout << "b=\n" << b << endl << flush;
        cout << "rank(A)=" << A.fullPivHouseholderQr().rank() << endl;
    }
}
