#ifndef DRAWHTML_H
#define DRAWHTML_H

#include <string>
#include <tuple>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
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
    void drawRect(string name, drawBox, int* color);
    void drawRect(string name, drawBox, int* color, double opacity);
    void drawRect(string name, drawBox, string color, double opacity);
    void drawRect(string name, drawBox, int* color, double opacity, map<string,string> m_para);
    void drawRect(string name, drawBox, string color, double opacity, map<string,string> m_para);
    void drawLine(string name, drawPos, drawPos, string color,double width);
    void drawLine(string name, drawPos, drawPos, int* color,double width);
    void drawLine(string name, drawPos, drawPos, string color,double width,double opacity);
    void drawLine(string name, drawPos, drawPos, int* color,double width,double opacity);
    void drawLine(string name, drawPos, drawPos, string color,double width,double opacity, map<string,string> m_para);
    void drawLine(string name, drawPos, drawPos, int* color,double width,double opacity, map<string,string> m_para);
    void drawBBox(string name, drawBox, int* color,double width,double opacity, map<string,string> m_para);

    void drawText(string name, drawPos, string);
    void drawText(string name, drawPos, string, double, double); // offset_x offset_y
private:
    string fileName;
    ofstream fout;
    string fileName_head = "src/utility/drawHtml_head.txt";
    string fileName_tail = "src/utility/drawHtml_tail.txt";
    // svg
    double outline_x = 1600;
    double outline_y = 1200;
    double scaling = 200;
    double offset_x = 200;
    double offset_y = 600;
};

#endif