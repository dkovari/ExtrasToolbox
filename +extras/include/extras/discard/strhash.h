/*----------------------------------------------------------------------
cstring hash function, based on https://stackoverflow.com/questions/2111667/compile-time-string-hashing
and here https://stackoverflow.com/questions/16388510/evaluate-a-string-with-a-switch-in-c/16388594
-----------------------------------------------------------------------*/
#pragma once

/**Hash function returning integer from char array
*/
constexpr unsigned int strhash(const char* str, int h = 0) {
    return !str[h] ? 5381 : (strhash(str, h + 1) * 33) ^ str[h]; //if end of string, return 5381, otherwise return hash value: Sum( (char*33)XOR(char) )
}