#include "paramHdl.h"

ParamHdl::ParamHdl(){}
ParamHdl::ParamHdl(int argc, char** argv){
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
    if(!check_exist("input") || !check_exist("output")){
        _isOK = false;
        return;
    }
}

bool ParamHdl::ok(){
    return _isOK;
}

bool ParamHdl::check_exist(string flag){
    if(m_paras.find(flag) != m_paras.end()){
        if(m_paras[flag] != ""){
            return true;
        }
    }
    return false;
}

void ParamHdl::print_help(){
    cout << "Usage: " << BOLDYELLOW << "bin/3d-placer <input.txt> <output.txt>" << RESET << "\n\n";
    cout << "Suporting Flags:\n";
    for(auto it : m_singleFlag){
        cout << "  -" << setw(20) << left << it.first << ": " << it.second << "\n";
    }
    for(auto it : m_otherFlag){
        cout << "  -" << setw(20) << left << it.first + " <...>" << ": " << it.second << "\n";
    }
}

bool ParamHdl::check_is_single_flag(string flag){
    return (m_singleFlag.find(flag) != m_singleFlag.end());
}