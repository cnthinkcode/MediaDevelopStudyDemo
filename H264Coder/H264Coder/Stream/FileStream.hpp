//
//  FileStream.hpp
//  H264Coder
//
//  Created by hejianyuan on 2022/6/28.
//

#ifndef FileStream_hpp
#define FileStream_hpp

#include <stdio.h>
#include <vector>
#include "NALUnit.hpp"

class FileStream {
public:
    FileStream(const char * fileName);
    ~FileStream();
    
    
    /// 遍历下一个NAL
    /// @param next_nal_pos  [out] 下一个NAL的偏移量
    /// @param previous_nal_end_pos [out] 上一个NAL的尾部
    int parse_next_nal(long &next_nal_pos, long &previous_nal_end_pos);
    
    std::vector<NALUintRange> parse_h264_bit_stream();
    
    size_t file_read(uint8_t *buffer, long location, size_t size);

    
private:
    void file_info();
    void file_error(int code);
    
    int file_getc();
    bool file_eof();
    long file_tell();
    
    
private:
    // 文件Handle
    FILE *_fileHandle;
    // 文件Path
    const char *_fileName;
    
    // 缓存地址
    uint8_t *_streamCache;
    // 缓存最大长度
    long _streamMaxCacheSize;
    // 当前读取位置
    long _streamCacheIndex;
    // 缓存在文件中的位置
    long _streamCacheOffsetInFile;
    // 缓存有效数据
    long _streamCacheDataSize;
};


void testFileStream();

#endif /* FileStream_hpp */
