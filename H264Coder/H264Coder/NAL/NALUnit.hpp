//
//  NAL.hpp
//  H264Coder
//
//  Created by hejianyuan on 2022/6/28.
//

#ifndef NAL_hpp
#define NAL_hpp

#include <stdio.h>
#include <iostream>

struct NALUintRange{
    long location;
    size_t size;
    
    NALUintRange(long loc, size_t size){
        this->location = loc;
        this->size = size;
    }
};

class NALUnit {
    
public:
    NALUnit(uint8_t *payload, size_t size);
    ~NALUnit();
    
    
    uint8_t nal_ref_idc;
    uint8_t nal_unit_type;
    
    uint8_t *data;

private:
    // 加载数据
    void load_data();
    // 去掉0x03
    void ebsp_to_sodb();
    
private:

    
};

#endif /* NAL_hpp */
