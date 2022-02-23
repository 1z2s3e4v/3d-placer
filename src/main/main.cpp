#include <iostream>
#include <string>
#include <cmath>
#include <sstream>
#include <vector>
#include <map>
using namespace std;

#include "../utility/paramHdl.h"
#include "../parser/parser.h"

int main(int argc, char** argv){
    // parameter handler
    ParamHdl paramHdl = ParamHdl(argc, argv);
    if(!paramHdl.ok())
        return 0;
    // parser
    Parser parser;
    if(!parser.read_file(paramHdl.get_input_fileName()))
        return 0;

    return 0;
}