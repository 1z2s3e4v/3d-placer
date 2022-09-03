#ifndef PARAMHDL_H
#define PARAMHDL_H

#include "color.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
using namespace std;

class ParamHdl_C{
public:
    ParamHdl_C();
    ParamHdl_C(int, char**);

    bool ok();
    bool check_flag_exist(string);
    void print_help();
    string get_input_fileName();
    string get_output_fileName();
    string get_case_name();
    string get_para(string);
private:
    bool check_is_single_flag(string&);

    map<string, string> m_paras;
    bool _isOK = true;
    // flags for help --- {"flag", "describtion"}
    map<string, string> m_singleFlag{{"help", "print the usage."},
                                     {"h", "same as -help."},
                                     {"debug", "debug_mode, print the debug messages."}, 
                                     {"d", "same as -debug."}, 
                                     {"dump", "dump all messages."},
                                     {"dump_no_debug", "dump all messages without debug messages."},
                                     {"safe_mode", "run the safe-flow once and than run stronger flow."},
                                     {"no_dump", "without any dump file"},
                                     {"no_draw", "without any visualization"},
                                     {"only_draw_result", "only visualize the result"},
                                     {"draw_gp", "draw analytical step result."},
                                     {"preplace","Pararrel RePlAce"}};
    map<string, string> m_otherFlag{{"time_limit", "time limit, default=3600(s)."}, 
                                    {"repeat", "repeat the code with n times and store the best result. defalt=1."},
                                    {"flow", "placer flow (default is shrunk2d_replace)"},
                                    {"seed", "random seed"},
                                    {"rParaInd", "test replace"}};
};

#endif
