#include <iostream>
#include <string>
#include <cmath>
#include <sstream>
#include <vector>
#include <map>
using namespace std;

#include "../utility/paramhdl.h"

int main(int argc, char** argv){
    // parameter handler
    ParamHdl paraHdl = new ParamHdl(argc, argv);
    if(!paraHdl.ok()){
        paraHdl.print_help();
        return 0;
    }
    // start


    return 0;
}