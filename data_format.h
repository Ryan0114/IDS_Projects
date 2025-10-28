#pragma once
#include <bits/stdc++.h>
using namespace std;

struct Point {
    double x, y;

    Point(double x, double y): x(x), y(y) {}

    Point operator+(const Point& other) const {
        return Point(x+other.x, y+other.y); 
    }

    Point operator-(const Point& other) const {
        return Point(x-other.x, y-other.y);
    }

    double dist_to(const Point& other) const {
        double dx = x-other.x;
        double dy = y-other.y;
        return sqrt(dx*dx + dy*dy);
    }
};

struct Segment {
    int size=0;
    vector<Point> segment;
};
