#ifndef BUF_PAGE_MANAGER
#define BUF_PAGE_MANAGER

#include <vector>
#include <unordered_map>

#include "../utils/MyHashMap.h"
#include "../utils/MyBitMap.h"
#include "FindReplace.h"
#include "../utils/pagedef.h"
#include "../fileio/FileManager.h"
#include "../utils/MyLinkList.h"

#include "../../buffer_offset.h"

/*
 * BufferedPageManager
 * 实现了一个缓存的管理器
 */
struct BufferedPageManager {

public:
    class DualWayMap {
    public:
        struct Hash {
            uint64_t operator()(watery::BufferOffset offset) const {
                return std::hash<uint64_t>{}(
                    (static_cast<uint64_t>(offset.file_handle) << 32) |
                    static_cast<uint64_t>(offset.page_offset));
            }
        };
    
        struct Pred {
            bool operator()(watery::BufferOffset lhs, watery::BufferOffset rhs) const {
                return lhs.page_offset == rhs.page_offset && lhs.file_handle == rhs.file_handle;
            }
        };
    
        using OffsetToHandleMap = std::unordered_map<watery::BufferOffset, watery::BufferHandle, Hash, Pred>;
        using HandleToOffsetMap = std::vector<watery::BufferOffset>;

    private:
        OffsetToHandleMap _offset_to_handle_map;
        HandleToOffsetMap _handle_to_offset_map;

    public:
        explicit DualWayMap(size_t size) {
            _handle_to_offset_map.resize(size, {-1, -1});
        }
        
        watery::BufferHandle get_handle(watery::BufferOffset offset) {
            return _offset_to_handle_map[offset];
        }
        
        watery::BufferOffset get_offset(watery::BufferHandle handle) {
            return _handle_to_offset_map[handle];
        }
        
        void set(watery::BufferHandle handle, watery::BufferOffset offset) {
            _offset_to_handle_map[offset] = handle;
            _handle_to_offset_map[handle] = offset;
        }
        
        void remove(watery::BufferHandle handle) {
            _offset_to_handle_map.erase(_handle_to_offset_map[handle]);
            _handle_to_offset_map[handle] = {-1, -1};
        }
        
    };
    
    

private:
    int _last;
    FileManager *_fileManager;
    DualWayMap _map{MAX_BUFFERED_PAGE_COUNT};
    FindReplace *_replace;
    bool *_dirty;
    /*
     * 缓存页面数组
     */
    BufType *addr;
    BufType _allocate_memory() {
        return new unsigned int[(PAGE_SIZE >> 2)];
    }
    BufType _fetch_page(int typeID, int pageID, int &index) {
        BufType b;
        index = _replace->find();
        b = addr[index];
        if (b == NULL) {
            b = _allocate_memory();
            addr[index] = b;
        } else {
            if (_dirty[index]) {
                auto [f, p] = _map.get_offset(index);
                _fileManager->write_page(f, p, b, 0);
                _dirty[index] = false;
            }
        }
        _map.set(index, {typeID, pageID});
        return b;
    }
    /*
     * @函数名release
     * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
     * 功能:将index代表的缓存页面归还给缓存管理器，在归还前，缓存页面中的数据不标记写回
     */
    void _release(int index) {
        _dirty[index] = false;
        _replace->free(index);
        _map.remove(index);
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
    BufType allocate_page(int fileID, int pageID, int &index, bool ifRead = false) {
        BufType b = _fetch_page(fileID, pageID, index);
        if (ifRead) {
            _fileManager->read_page(fileID, pageID, b, 0);
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
    BufType get_page(int fileID, int pageID, int &index) {
        index = _map.get_handle({fileID, pageID});
        if (index != -1) {
            mark_access(index);
            return addr[index];
        } else {
            BufType b = _fetch_page(fileID, pageID, index);
            _fileManager->read_page(fileID, pageID, b, 0);
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
        _replace->access(index);
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
            auto [f, p] = _map.get_offset(index);
            _fileManager->write_page(f, p, addr[index], 0);
            _dirty[index] = false;
        }
        _replace->free(index);
        _map.remove(index);
    }
    /*
     * @函数名close
     * 功能:将所有缓存页面归还给缓存管理器，归还前需要根据脏页标记决定是否写到对应的文件页面中
     */
    void close() {
        for (int i = 0; i < MAX_BUFFERED_PAGE_COUNT; ++i) {
            write_back(i);
        }
    }
    /*
     * @函数名getKey
     * @参数index:缓存页面数组中的下标，用来指定一个缓存页面
     * @参数fileID:函数返回时，用于存储指定缓存页面所属的文件号
     * @参数pageID:函数返回时，用于存储指定缓存页面对应的文件页号
     */
    void get_key(int index, int &fileID, int &pageID) {
        auto [f, p] = _map.get_offset(index);
        fileID = f;
        pageID = p;
    }
    /*
     * 构造函数
     * @参数fm:文件管理器，缓存管理器需要利用文件管理器与磁盘进行交互
     */
    BufferedPageManager(FileManager *fm) {
        int c = MAX_BUFFERED_PAGE_COUNT;
        int m = MOD;
        _last = -1;
        _fileManager = fm;
        //bpl = new MyLinkList(MAX_BUFFERED_PAGE_COUNT, MAX_FILE_NUM);
        _dirty = new bool[MAX_BUFFERED_PAGE_COUNT];
        addr = new BufType[MAX_BUFFERED_PAGE_COUNT];
        _replace = new FindReplace(c);
        for (int i = 0; i < MAX_BUFFERED_PAGE_COUNT; ++i) {
            _dirty[i] = false;
            addr[i] = NULL;
        }
    }
};
#endif
