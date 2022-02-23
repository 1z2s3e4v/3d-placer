#ifndef PARAMHDL_H
#define PARAMHDL_H

#include "../utility/color.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
using namespace std;

class ParamHdl{
public:
    ParamHdl();
    ParamHdl(int, char**);

    bool ok();
    bool check_exist(string);
    void print_help();
    string get_input_fileName();
    string get_output_fileName();
private:
    bool check_is_single_flag(string);

    map<string, string> m_paras;
    bool _isOK = true;
    // flags for help
    map<string, string> m_singleFlag{{"help", "print the usage."},
                                     {"h", "same as -help."},
                                     {"debug", "debug_mode, print the debug messages."}, 
                                     {"d", "same as -debug."}, 
                                     {"dump", "dump all messages."},
                                     {"dump_no_debug", "dump all messages without debug messages."}};
    map<string, string> m_otherFlag{{"time_limit", "time limit, default=3600(s)."}, 
                                    {"repeat", "repeat the code with n times and store the best result. defalt=1."}};
};

#endif