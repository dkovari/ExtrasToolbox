#pragma once

template<typename T>
inline T vmax(T a, T b){
    if(a<b){
        return b;
    }
    return a;
}

template<typename T>
inline T vmin(T a, T b){
    if(a<b){
        return a;
    }
    return a;
}
