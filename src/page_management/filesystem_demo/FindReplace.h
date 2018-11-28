#ifndef BUF_SEARCH
#define BUF_SEARCH

#include "MyLinkList.h"
#include "MyHashMap.h"

namespace watery {

/*
 * FindReplace
 * 提供替换算法接口，这里实现的是栈式LRU算法
 */
class FindReplace {
private:
    MyLinkList _list;

public:
    /*
     * @函数名free
     * @参数index:缓存页面数组中页面的下标
     * 功能:将缓存页面数组中第index个页面的缓存空间回收
     *           下一次通过find函数寻找替换页面时，直接返回index
     */
    void free(int index) {
        _list.insertFirst(0, index);
    }
    /*
     * @函数名access
     * @参数index:缓存页面数组中页面的下标
     * 功能:将缓存页面数组中第index个页面标记为访问
     */
    void access(int index) {
        _list.insert(0, index);
    }
    /*
     * @函数名find
     * 功能:根据替换算法返回缓存页面数组中要被替换页面的下标
     */
    int find() {
        int index = _list.getFirst(0);
        _list.remove(index);
        _list.insert(0, index);
        return index;
    }
    /*
     * 构造函数
     * @参数c:表示缓存页面的容量上限
     */
    explicit FindReplace(size_t c) : _list{c, 1} {
        for (int i = 0; i < c; ++i) {
            _list.insert(0, i);
        }
    }
};

}

#endif
