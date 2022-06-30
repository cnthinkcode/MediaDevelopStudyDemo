//
//  FileStream.cpp
//  H264Coder
//
//  Created by hejianyuan on 2022/6/28.
//

#include "FileStream.hpp"
#include <iostream>
#include <cmath>

using namespace std;
 
FileStream::FileStream(const char * fileName):_streamMaxCacheSize(2*1024*1024), _streamCacheIndex(0), _streamCacheOffsetInFile(0), _streamCacheDataSize(0){
    _streamCache =(uint8_t *)malloc(_streamMaxCacheSize);
    if (_streamCache == nullptr) {
        file_error(501);
    }
    _fileName = fileName;
    file_info();
    _fileHandle = fopen(fileName, "rb");
    if (_fileHandle == nullptr) {
        file_error(0);
    }
}

FileStream::~FileStream(){
    if (_fileHandle) {
        fclose(_fileHandle);
        _fileHandle = nullptr;
    }
    
    if (_streamCache) {
        free(_streamCache);
    }
}


#pragma mark - FILE操作

void FileStream::file_info(){
    if (_fileName) {
        printf("创建FileStream：%s \n", _fileName);
    }
}

void FileStream::file_error(int code){
    switch (code) {
        case 0:
            printf("打开文件 %s 失败 \n", _fileName);
            break;
        case 1:
            printf("读取文件 %s 错误 \n", _fileName);
            break;
            
        case 501:
            printf("内存不足 \n");
            break;
            
        default:
            break;
    }
}

int FileStream::file_getc(){
    int result = EOF;
    
    // 如果缓存里面的读完了，开始读取文件填充缓存
    if (_streamCacheIndex >= _streamCacheDataSize) {
        long last = ftell(_fileHandle);
        _streamCacheDataSize = fread(_streamCache, 1, _streamMaxCacheSize, _fileHandle);
        if (_streamCacheDataSize) {
            _streamCacheOffsetInFile = last;
            _streamCacheIndex = 0;
        }
    }
    
    if (_streamCacheIndex < _streamCacheDataSize) {
        result = _streamCache[_streamCacheIndex];
        _streamCacheIndex++;
    }
        
    return result;
}

bool FileStream::file_eof(){
    return feof(_fileHandle) && (_streamCacheIndex >= _streamCacheDataSize);
}

long FileStream::file_tell(){
    return _streamCacheIndex + _streamCacheOffsetInFile;
}

size_t FileStream::file_read(uint8_t *buffer, long location, size_t size){
    if (fseek(_fileHandle, location, SEEK_SET)) {
        return 0;
    }
    
    return fread(buffer, 1, size, _fileHandle);
}



#pragma mark - NAL PARSE

#if 1
int FileStream::parse_next_nal(long &next_pos, long &previous_end_pos){
    // NAL 开始是0x 00 00 00 01 或者 0x 00 00 01
    int get_prefix_flag = 0;
    int leading_zero_num = 0;
    
    while (!file_eof()) {
        int byte = file_getc();
        if (byte == EOF) {
            break;
        };
    
        if (byte == 0) {
            ++leading_zero_num;
        }else if (byte == 1 && leading_zero_num >= 2){
            // 找到了 NAL Starting
            if (leading_zero_num == 2) {
                get_prefix_flag = 1;
            }else if(leading_zero_num == 3){
                get_prefix_flag = 2;
            }else{
                // 可能出错了
                get_prefix_flag = 3;
            }
            next_pos = file_tell();
            previous_end_pos = fmax(next_pos - leading_zero_num - 2, 0);
            break;
        }else{
            leading_zero_num = 0;
        }
    }
    
    if (!get_prefix_flag) {
        previous_end_pos = 0;
    }
    
    if (file_eof()) {
        next_pos = 0;
        previous_end_pos = file_tell();
    }
    
    return get_prefix_flag;
}

#else

int FileStream::parse_next_nal(long &next_pos, long &previous_end_pos){
    // NAL 开始是0x 00 00 00 01 或者 0x 00 00 01
    int get_prefix_flag = 0;
    int leading_zero_num = 0;
    
    while (!feof(_fileHandle)) {
        int byte = fgetc(_fileHandle);
        if (byte == EOF) break;
        
        if (byte == 0) {
            ++leading_zero_num;
        }else if (byte == 1 && leading_zero_num >= 2){
            // 找到了 NAL Starting
            if (leading_zero_num == 2) {
                get_prefix_flag = 1;
            }else if(leading_zero_num == 3){
                get_prefix_flag = 2;
            }else{
                // 可能出错了
                get_prefix_flag = 3;
            }
            next_pos = ftell(_fileHandle);
            
            previous_end_pos = fmax(next_pos - leading_zero_num - 2, 0);
            break;
        }else{
            leading_zero_num = 0;
        }
    }
    
    if (!get_prefix_flag) {
        previous_end_pos = 0;
    }
    
    if (feof(_fileHandle)) {
        next_pos = 0;
        previous_end_pos = ftell(_fileHandle);
    }
    
    return get_prefix_flag;
}

#endif

std::vector<NALUintRange> FileStream::parse_h264_bit_stream(){
    std::vector<NALUintRange> result;
    
    NALUintRange *lastNal = nullptr;
    
    while (1) {
        long pos = 0;
        long previous_end_pos = 0;
        bool ret = parse_next_nal(pos, previous_end_pos);
        
        if (previous_end_pos) {
            if (lastNal) {
                lastNal->size = previous_end_pos - lastNal->location;
            }
        }
        
        if (ret > 0 && pos) {
            result.push_back(NALUintRange(pos, 0));
            lastNal = &(result[result.size() - 1]);
        }else{
            break;
        }
    }
        
    return result;
}

void testFileStream(){
    FileStream stream = FileStream( "/Users/1905-developer-ios/Desktop/我的研究/当前资料/ffmpeg/output1.h264");
    std::vector<NALUintRange> array = stream.parse_h264_bit_stream();
    
    FileStream nalReader = FileStream( "/Users/1905-developer-ios/Desktop/我的研究/当前资料/ffmpeg/output1.h264");

    uint8_t prefix = 0;
    
    for (int i = 0; i < array.size(); i++) {
        NALUintRange range = array[i];
        
        if (nalReader.file_read(&prefix, range.location, 1) == 0) {
            printf("");
        }
        
        u_int8_t forbidden_bit = prefix & 0x200;
        u_int8_t nal_ref_idc = prefix & 0x140;
        u_int8_t nal_unit_type = prefix & 0x37;
        

        printf("NAL LOC:%06ld  LENGTH:%05ld PREFIX:%02x TYPE:%02d\n", range.location, range.size, prefix, nal_unit_type);
    }
    
    
    
}
