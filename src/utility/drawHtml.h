#ifndef DRAWHTML_H
#define DRAWHTML_H

#include <string>
#include <tuple>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

typedef tuple<double, double> drawPos; // x, y
typedef tuple<drawPos, drawPos> drawBox; // ll, ur

class Drawer_C{
public:
    Drawer_C();
    Drawer_C(string);
    
    
    // svg
    void start_svg();
    void end_svg();
    void setting(double,double,double,double,double); // outline_x outline_y scaling offset_x offset_y
    void drawRect(string name, drawBox, string color);
    void drawLine(string name, drawPos, drawPos, string color,double width);
    void drawLine(string name, drawPos, drawPos, int* color,double width);
    void drawLine(string name, drawPos, drawPos, string color,double width,double opacity);
    void drawLine(string name, drawPos, drawPos, int* color,double width,double opacity);
    void drawText(string name, drawPos, string);
private:
    string fileName;
    ofstream fout;
    // svg
    double outline_x = 1600;
    double outline_y = 1200;
    double scaling = 200;
    double offset_x = 200;
    double offset_y = 600;
};

#endif