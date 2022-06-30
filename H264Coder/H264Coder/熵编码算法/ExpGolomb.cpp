//
//  ExpGolomb.cpp
//  H264Coder
//
//  Created by hejianyuan on 2022/6/27.
//

#include "ExpGolomb.hpp"
#include <assert.h>

typedef unsigned char uint8;

int read_bit(uint8 *buffer, uint8 &byte_pos, uint8 &bit_pos){
    uint8 mask = 0, val = 0;
    mask = 1 << (7 - bit_pos);
    val =  ((buffer[byte_pos] & mask) != 0);
    
    if ((bit_pos = ++bit_pos%8) == 0) {
        byte_pos++;
    }
    
    return val;
}

int get_uev_code_num(uint8 *buffer, uint8 &byte_pos, uint8 &bit_pos){
    assert(bit_pos < 8);
    uint8 val = 0, leading_zero_bits = 0;
    int prefix = 0, suffix = 0;
    while (1) {
        val = read_bit(buffer, byte_pos, bit_pos);
        if (val == 0) {
            ++leading_zero_bits;
        }else{
            break;
        }
    }
    prefix = (1 << leading_zero_bits) - 1;
    for (int i = 0; i < leading_zero_bits; i++) {
        val = read_bit(buffer, byte_pos, bit_pos);
        suffix += val * (1 << (leading_zero_bits - i - 1));
    }
    
    prefix += suffix;
    return prefix;
}

void textExpGolomb(){
    uint8 bytesArray[6] = {0xa6, 0x42, 0x98, 0xe2, 0x04, 0x8a};
    uint8 byte_pos = 0, bit_pos = 0;
    int bytes_size = sizeof(bytesArray) * 8;
    
    int code_num = 0;
    while ((byte_pos * 8 + bit_pos) < bytes_size) {
        code_num = get_uev_code_num(bytesArray, byte_pos, bit_pos);
        printf("Exp_Columb Code Num = %d \n", code_num);
    }
}

