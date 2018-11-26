#ifndef MY_HASH_MAP
#define MY_HASH_MAP

#include <utility>
#include "pagedef.h"
#include "MyLinkList.h"

/*
 * hash表的键
 */
struct DataNode {
    /*
     * 第一个键
     */
    int key1;
    /*
     * 第二个键
     */
    int key2;
};
/*
 * 两个键的hash表
 * hash表的value是自然数，在缓存管理器中，hash表的value用来表示缓存页面数组的下标
 */
class MyHashMap {
private:
    size_t _capacity, _mod;
    MyLinkList list;
    std::vector<DataNode> a;
    /*
     * hash函数
     */
    uint64_t hash(int k1, int k2) {
        return std::hash<uint64_t>{}((static_cast<uint64_t>(k1) << 32) | static_cast<uint64_t>(k2)) % _mod;
    }

public:
    /*
     * @函数名findIndex
     * @参数k1:第一个键
     * @参数k2:第二个键
     * 返回:根据k1和k2，找到hash表中对应的value
     *           这里的value是自然数，如果没有找到，则返回-1
     */
    int findIndex(int k1, int k2) {
        auto p = list.getFirst(hash(k1, k2));
        while (!list.isHead(p)) {
            if (a[p].key1 == k1 && a[p].key2 == k2) {
                return p;
            }
            p = list.next(p);
        }
        return -1;
    }
    /*
     * @函数名replace
     * @参数index:指定的value
     * @参数k1:指定的第一个key
     * @参数k2:指定的第二个key
     * 功能:在hash表中，将指定value对应的两个key设置为k1和k2
     */
    void replace(int index, int k1, int k2) {
        list.insertFirst(hash(k1, k2), index);
        a[index].key1 = k1;
        a[index].key2 = k2;
    }
    /*
     * @函数名remove
     * @参数index:指定的value
     * 功能:在hash表中，将指定的value删掉
     */
    void remove(int index) {
        list.del(index);
        a[index].key1 = -1;
        a[index].key2 = -1;
    }
    /*
     * @函数名getKeys
     * @参数index:指定的value
     * @参数k1:存储指定value对应的第一个key
     * @参数k2:存储指定value对应的第二个key
     */
    void getKeys(int index, int &k1, int &k2) {
        k1 = a[index].key1;
        k2 = a[index].key2;
    }
    /*
     * 构造函数
     * @参数c:hash表的容量上限
     * @参数m:hash函数的mod
     */
    MyHashMap(size_t c, size_t m)
        : _capacity{c}, _mod{m}, list{c, m} {
        a.resize(c, {-1, -1});
    }
};
#endif
