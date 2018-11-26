#ifndef BUFFERED_PAGE_MANAGER
#define BUFFERED_PAGE_MANAGER

#include <array>
#include <bitset>
#include <vector>
#include <unordered_map>

#include "MyHashMap.h"
#include "pagedef.h"
#include "FileManager.h"
#include "MyLinkList.h"

#include "FindReplace.h"

/*
 * BufferedPageManager
 * 实现了一个缓存的管理器
 */
struct BufferedPageManager {
    
private:
    int _last;
    FileManager &_file_manager;
    MyHashMap _map{MAX_BUFFERED_PAGE_COUNT, MAX_FILE_NUM};
    FindReplace _replace{MAX_BUFFERED_PAGE_COUNT};
    std::bitset<MAX_BUFFERED_PAGE_COUNT> _dirty;
    std::array<Buffer, MAX_BUFFERED_PAGE_COUNT> _addr{};
    /*
     * 缓存页面数组
     */
    Buffer _allocate_memory() {
        return reinterpret_cast<Buffer>(new uint8_t[PAGE_SIZE]);
    }
    
    Buffer _fetch_page(int typeID, int pageID, int &index) {
        Buffer b;
        index = _replace.find();
        b = _addr[index];
        if (b == nullptr) {
            b = _allocate_memory();
            _addr[index] = b;
        } else {
            if (_dirty[index]) {
                int f = 0, p = 0;
                _map.getKeys(index, f, p);
                _file_manager.write_page(f, p, b, 0);
                _dirty[index] = false;
            }
        }
        _map.replace(index, typeID, pageID);
        return b;
    }

public:
    /*
     * @函数名allocPage
     * @参数fileID:文件id，数据库程序在运行时，用文件id来区分正在打开的不同的文件
     * @参数pageID:文件页号，表示在fileID指定的文件中，第几个文件页
     * @参数index:函数返回时，用来记录缓存页面数组中的下标
     * @参数ifRead:是否要将文件页中的内容读到缓存中
     * 返回:缓存页面的首地址
     * 功能:为文件中的某一个页面获取一个缓存中的页面
     *           缓存中的页面在缓存页面数组中的下标记录在index中
     *           并根据ifRead是否为true决定是否将文件中的内容写到获取的缓存页面中
     * 注意:在调用函数allocPage之前，调用者必须确信(fileID,pageID)指定的文件页面不存在缓存中
     *           如果确信指定的文件页面不在缓存中，那么就不用在hash表中进行查找，直接调用替换算法，节省时间
     */
    Buffer allocate_page(int fileID, int pageID, int &index, bool ifRead = false) {
        Buffer b = _fetch_page(fileID, pageID, index);
        if (ifRead) {
            _file_manager.read_page(fileID, pageID, b, 0);
        }
        return b;
    }
    /*
     * @函数名getPage
     * @参数fileID:文件id
     * @参数pageID:文件页号
     * @参数index:函数返回时，用来记录缓存页面数组中的下标
     * 返回:缓存页面的首地址
     * 功能:为文件中的某一个页面在缓存中找到对应的缓存页面
     *           文件页面由(fileID,pageID)指定
     *           缓存中的页面在缓存页面数组中的下标记录在index中
     *           首先，在hash表中查找(fileID,pageID)对应的缓存页面，
     *           如果能找到，那么表示文件页面在缓存中
     *           如果没有找到，那么就利用替换算法获取一个页面
     */
    Buffer get_page(int fileID, int pageID, int &index) {
        index = _map.findIndex(fileID, pageID);
        if (index != -1) {
            mark_access(index);
            return _addr[index];
        } else {
            Buffer b = _fetch_page(fileID, pageID, index);
            _file_manager.read_page(fileID, pageID, b, 0);
            return b;
        }
    }
    /*
     * @函数名access
     * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
     * 功能:标记index代表的缓存页面被访问过，为替换算法提供信息
     */
    void mark_access(int index) {
        if (index == _last) {
            return;
        }
        _replace.access(index);
        _last = index;
    }
    /*
     * @函数名markDirty
     * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
     * 功能:标记index代表的缓存页面被写过，保证替换算法在执行时能进行必要的写回操作，
     *           保证数据的正确性
     */
    void mark_dirty(int index) {
        _dirty[index] = true;
        mark_access(index);
    }
    /*
     * @函数名writeBack
     * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
     * 功能:将index代表的缓存页面归还给缓存管理器，在归还前，缓存页面中的数据需要根据脏页标记决定是否写到对应的文件页面中
     */
    void write_back(int index) {
        if (_dirty[index]) {
            int f = 0, p = 0;
            _map.getKeys(index, f, p);
            _file_manager.write_page(f, p, _addr[index], 0);
            _dirty[index] = false;
        }
        _replace.free(index);
        _map.remove(index);
    }
    
    /*
     * @函数名getKey
     * @参数index:缓存页面数组中的下标，用来指定一个缓存页面
     * @参数fileID:函数返回时，用于存储指定缓存页面所属的文件号
     * @参数pageID:函数返回时，用于存储指定缓存页面对应的文件页号
     */
    void get_key(int index, int &fileID, int &pageID) {
        _map.getKeys(index, fileID, pageID);
    }
    /*
     * 构造函数
     * @参数fm:文件管理器，缓存管理器需要利用文件管理器与磁盘进行交互
     */
    explicit BufferedPageManager(FileManager &fm)
        : _file_manager{fm} {
        _last = -1;
        _addr.fill(nullptr);
    }
    
    ~BufferedPageManager() {
        for (int i = 0; i < MAX_BUFFERED_PAGE_COUNT; ++i) {
            write_back(i);
            if (_addr[i] != nullptr) {
                delete[] _addr[i];
            }
        }
    }
    
};

#endif
