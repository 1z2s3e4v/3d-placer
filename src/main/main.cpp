#include <iostream>
#include <string>
#include <cmath>
#include <sstream>
#include <vector>
#include <map>
using namespace std;

#include "../utility/paramHdl.h"

int main(int argc, char** argv){
    // parameter handler
    ParamHdl* paramHdl = new ParamHdl(argc, argv);
    if(paramHdl->check_exist("help") || paramHdl->check_exist("h")){
        paramHdl->print_help();
        return 0;
    }
    else if(!paramHdl->ok()){
        cout << "\033[94m[ParamHdl]\033[0m - Invalid argvs\n";
        paramHdl->print_help();
        return 0;
    }
    // start


    return 0;
}