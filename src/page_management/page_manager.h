//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_FILE_MANAGER_H
#define WATERYSQL_FILE_MANAGER_H

#include <set>
#include <vector>
#include <map>
#include <unordered_map>
#include <fstream>
#include <queue>

#include "page_handle.h"
#include "../utility/type/singleton.h"

namespace watery {

class FileManager : public Singleton<FileManager> {

public:
    struct CacheDescriptor {
        TimeStamp time_stamp{0};
        PageHandle page_handle{-1, -1};
        bool dirty{false};
    };

private:
    TimeStamp _current_time_stamp{0};
    std::map<TimeStamp, CacheHandle> _cache_usage;
    std::unordered_map<PageHandle, CacheHandle, PageHandle::Hash> _cached_pages;
    std::vector<CacheDescriptor> _cache_descriptors;
    std::vector<Byte> _cache;
    std::queue<CacheHandle> _available_cache_handles;
    
    std::vector<std::fstream> _open_files;
    std::vector<std::set<CacheHandle>> _file_associated_cache_sets;
    std::queue<FileHandle> _available_file_handles;
    
    Byte *_get_cache(CacheHandle h) noexcept;
    CacheHandle _assign_cache_handle(PageHandle page_handle);
    TimeStamp _assign_time_stamp() noexcept;
    void _flush_cache(CacheHandle h);
    Byte *_access_cache(CacheHandle h);

protected:
    FileManager();

public:
    ~FileManager();
    
    FileHandle open_file(const std::string &file_name);
    void close_file(FileHandle h);
    void create_file(const std::string &file_name);
    void delete_file(const std::string &file_name);
    
    CacheHandle allocate_page_cache(PageHandle page_handle);
    CacheHandle load_page_cache(PageHandle page_handle);
    
    Byte *access_cache_for_writing(CacheHandle h);
    const Byte *access_cache_for_reading(CacheHandle h);
    
};

}

#endif  // WATERYSQL_FILE_MANAGER_H
