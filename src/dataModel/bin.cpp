#include "bin.h"
#include <string>

bool BNet_C::is_cross_net() { // crossing die
        bool top = false;
        bool bot = false;
        for (Bin_C* bin : _vbin) {
            int bin_id = bin->get_id();
            if (bin_id < _bin_num_top) {
                top = true;
            } else {
                bot = true;
            }
        }
        if (top && bot) {
            return true;
        }
		return false;
}
void BNet_C::update_ballPos(){
    double min_x0 = INT_MAX, max_x0 = 0, min_y0 = INT_MAX, max_y0 = 0;
    double min_x1 = INT_MAX, max_x1 = 0, min_y1 = INT_MAX, max_y1 = 0;
    for (Bin_C* bin : _vbin) {
        if (bin->get_die() == 0) {
            if (bin->get_center().x < min_x0) {
                min_x0 = bin->get_center().x;
            } else if (bin->get_center().x > max_x0) {
                max_x0 = bin->get_center().x;
            }
            if (bin->get_center().y < min_y0) {
                min_x0 = bin->get_center().y;
            } else if (bin->get_center().y > max_y0) {
                max_x0 = bin->get_center().y;
            }
        } else {
            if (bin->get_center().x < min_x1) {
                min_x1 = bin->get_center().x;
            } else if (bin->get_center().x > max_x1) {
                max_x1 = bin->get_center().x;
            }
            if (bin->get_center().y < min_y1) {
                min_x1 = bin->get_center().y;
            } else if (bin->get_center().y > max_y1) {
                max_x1 = bin->get_center().y;
            }
        }
        
    }
    double bin_x, bin_y;
    if (min_x0 <= min_x1) {
        bin_x = min_x1;
    } else {
        bin_x = min_x0;
    }
    if (min_y0 <= min_y1) {
        bin_y = min_y1;
    } else {
        bin_y = min_y0;
    }
    _ballPos = Pos(bin_x, bin_y);
}
void BNet_C::update_bbox(){
    _vll.clear();
    _vur.clear();
    _vll.resize(2,Pos(INT_MAX,INT_MAX));
    _vur.resize(2,Pos(0,0));
    if(is_cross_net()){
        for(int dieId=0;dieId<2;++dieId){
            _vll[dieId].x = min(_vll[dieId].x, _ballPos.x);
            _vll[dieId].y = min(_vll[dieId].y, _ballPos.y);
            _vur[dieId].x = max(_vur[dieId].x, _ballPos.x);
            _vur[dieId].y = max(_vur[dieId].y, _ballPos.y);
        }
    }
    else{
        int dieId = _vbin[0]->get_die();
        _vll[1 - dieId] = Pos(0,0);
        _vur[1 - dieId] = Pos(0,0);
        
    }

    for (Bin_C* bin : _vbin) {
        int dieId = bin->get_die();
        _vll[dieId].x = min(_vll[dieId].x, bin->get_center().x);
        _vll[dieId].y = min(_vll[dieId].y, bin->get_center().y);
        _vur[dieId].x = max(_vur[dieId].x, bin->get_center().x);
        _vur[dieId].y = max(_vur[dieId].y, bin->get_center().y);
    }

}
int BNet_C::get_HPWL(int dieId){
    return abs(_vur[dieId].x - _vll[dieId].x) + abs(_vur[dieId].y - _vll[dieId].y) * _weight;
}
int BNet_C::get_total_HPWL(){
    return get_HPWL(0) + get_HPWL(1);
}
