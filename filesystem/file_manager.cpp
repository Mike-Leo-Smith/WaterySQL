//
// Created by Mike Smith on 2018/11/4.
//

#include "file_manager.h"

namespace watery {

FileManager &FileManager::instance() {
    static FileManager file_manager;
    return file_manager;
}

}