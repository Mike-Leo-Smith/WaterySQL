#ifndef FILE_MANAGER
#define FILE_MANAGER

#include <array>
#include <string>
#include <bitset>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "pagedef.h"

class FileManager {

private:
    std::array<int, MAX_FILE_NUM> _fd{};
    std::bitset<MAX_FILE_NUM> _fm;
    
    int _create_file(const char *name) {
        FILE *f = fopen(name, "a+");
        if (f == nullptr) {
            std::cout << "fail" << std::endl;
            return -1;
        }
        fclose(f);
        return 0;
    }
    
    int _open_file(const char *name, int fileID) {
        int f = open(name, O_RDWR);
        if (f == -1) {
            return -1;
        }
        _fd[fileID] = f;
        return 0;
    }
    
public:
    /*
     * FilManager构造函数
     */
    FileManager() {
        _fd.fill(-1);
    };
    
    /*
     * @函数名writePage
     * @参数fileID:文件id，用于区别已经打开的文件
     * @参数pageID:文件的页号
     * @参数buf:存储信息的缓存(4字节无符号整数数组)
     * @参数off:偏移量
     * 功能:将buf+off开始的2048个四字节整数(8kb信息)写入fileID和pageID指定的文件页中
     * 返回:成功操作返回0
     */
    int write_page(int fileID, int pageID, Buffer buf, int off) {
        int f = _fd[fileID];
        off_t offset = pageID;
        offset = (offset << PAGE_SIZE_IDX);
        off_t error = lseek(f, offset, SEEK_SET);
        if (error != offset) {
            return -1;
        }
        Buffer b = buf + off;
        write(f, (void *)b, PAGE_SIZE);
        return 0;
    }
    /*
     * @函数名readPage
     * @参数fileID:文件id，用于区别已经打开的文件
     * @参数pageID:文件页号
     * @参数buf:存储信息的缓存(4字节无符号整数数组)
     * @参数off:偏移量
     * 功能:将fileID和pageID指定的文件页中2048个四字节整数(8kb)读入到buf+off开始的内存中
     * 返回:成功操作返回0
     */
    int read_page(int fileID, int pageID, Buffer buf, int off) {
        int f = _fd[fileID];
        off_t offset = pageID;
        offset = (offset << PAGE_SIZE_IDX);
        off_t error = lseek(f, offset, SEEK_SET);
        if (error != offset) {
            return -1;
        }
        Buffer b = buf + off;
        read(f, (void *)b, PAGE_SIZE);
        return 0;
    }
    /*
     * @函数名closeFile
     * @参数fileID:用于区别已经打开的文件
     * 功能:关闭文件
     * 返回:操作成功，返回0
     */
    int close_file(int fileID) {
        _fm[fileID] = false;
        int f = _fd[fileID];
        close(f);
        return 0;
    }
    /*
     * @函数名createFile
     * @参数name:文件名
     * 功能:新建name指定的文件名
     * 返回:操作成功，返回true
     */
    bool create_file(const char *name) {
        _create_file(name);
        return true;
    }
    /*
     * @函数名openFile
     * @参数name:文件名
     * @参数fileID:函数返回时，如果成功打开文件，那么为该文件分配一个id，记录在fileID中
     * 功能:打开文件
     * 返回:如果成功打开，在fileID中存储为该文件分配的id，返回true，否则返回false
     */
    bool open_file(const char *name, int &fileID) {
        for (auto i = 0; i < MAX_FILE_NUM; i++) {
            if (!_fm[i]) {
                if (_open_file(name, fileID) == 0) {
                    _fm[i] = true;
                    return true;
                }
            }
        }
        return false;
    }
    
    ~FileManager() = default;
};

#endif
