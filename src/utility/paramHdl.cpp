#include "paramHdl.h"

ParamHdl_C::ParamHdl_C(){}
ParamHdl_C::ParamHdl_C(int argc, char** argv){
    string lastflag = "";
    int countNoFlagItem = 0;
    for(int i=1;i<argc;++i){
        string item(argv[i]);
        if(item[0] == '-'){ // flag
            string flag = item.substr(1,item.size()-1);
            if(check_is_single_flag(flag)){ // single flag
                m_paras.emplace(flag, "1");
            }else if(i+1 <= argc){ // the flag need value
                string item2(argv[i+1]);
                if(item2[0] == '-'){
                    _isOK = false;
                    return;
                }
                m_paras.emplace(flag, item2);
            }else{ // invalid flag
                _isOK = false;
                return;
            }
        }
        else{ // input and output fileName
            if(countNoFlagItem == 0){ // input
                m_paras.emplace("input", item);
                ++countNoFlagItem;
            }
            else if(countNoFlagItem == 1){ // output
                m_paras.emplace("output", item);
                ++countNoFlagItem;
            }
            else{ // invalid
                _isOK = false;
                return;
            }
        }
    }
    // check input and output is saved
    if(!check_flag_exist("input") || !check_flag_exist("output")){
        _isOK = false;
        return;
    }
}

bool ParamHdl_C::ok(){
    if(check_flag_exist("help") || check_flag_exist("h")){
        print_help();
    }
    else if(!_isOK){
        cout << BLUE << "[ParamHdl]" << RESET << " - Invalid argvs\n";
        cout << "Usage: " << BOLDYELLOW << "bin/3d-placer <input.txt> <output.txt>" << RESET << "\n\n";
    }
    return _isOK;
}

bool ParamHdl_C::check_flag_exist(string flag){
    if(m_paras.find(flag) != m_paras.end()){
        if(m_paras[flag] != ""){
            return true;
        }
    }
    return false;
}

void ParamHdl_C::print_help(){
    cout << "Usage: " << BOLDYELLOW << "bin/3d-placer <input.txt> <output.txt>" << RESET << "\n\n";
    cout << "Suporting Flags:\n";
    for(auto it : m_singleFlag){
        cout << "  -" << setw(20) << left << it.first << ": " << it.second << "\n";
    }
    for(auto it : m_otherFlag){
        cout << "  -" << setw(20) << left << it.first + " <...>" << ": " << it.second << "\n";
    }
}

string ParamHdl_C::get_input_fileName(){
    return get_para("input");
}

string ParamHdl_C::get_output_fileName(){
    return get_para("output");
}

string ParamHdl_C::get_para(string flag){
    return m_paras[flag];
}

bool ParamHdl_C::check_is_single_flag(string& flag){
    return (m_singleFlag.find(flag) != m_singleFlag.end());
}