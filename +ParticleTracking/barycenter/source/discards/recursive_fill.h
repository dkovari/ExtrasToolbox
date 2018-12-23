#pragma once

#include <cstddef>

void recursive_fill(const bool* src, bool* dst,int r, int c, size_t H,size_t W, bool EightConnect){
    if(r<0){return;}
    if(c<0){return;}
    if(r>=H){return;}
    if(c>=W){return;}

    size_t i = r+c*H;
    if(dst[i]) {return;} //already set in dst
    if(!src[i]) {return;} //not present in src
    dst[i] = true;

    recursive_fill(src,dst,r+1,c,H,W,EightConnect);//try north
    recursive_fill(src,dst,r-1,c,H,W,EightConnect);//try south
    recursive_fill(src,dst,r,c+1,H,W,EightConnect);//try east
    recursive_fill(src,dst,r,c-1,H,W,EightConnect);//try west

    if(EightConnect){
        recursive_fill(src,dst,r+1,c+1,H,W,EightConnect);//try northeast
        recursive_fill(src,dst,r-1,c+1,H,W,EightConnect);//try southeast
        recursive_fill(src,dst,r+1,c-1,H,W,EightConnect);//try northwest
        recursive_fill(src,dst,r-1,c-1,H,W,EightConnect);//try southwest
    }

}
