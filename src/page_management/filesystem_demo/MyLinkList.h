#ifndef MY_LINK_LIST
#define MY_LINK_LIST

#include <vector>

class MyLinkList {
private:
    struct Node {
        int next;
        int prev;
    };
    size_t _capacity;
    size_t _list_count;
    std::vector<Node> _mem;
    
    void link(int prev, int next) {
        _mem[prev].next = next;
        _mem[next].prev = prev;
    }
    
public:
    void del(int index) {
        if (_mem[index].prev == index) {
            return;
        }
        link(_mem[index].prev, _mem[index].next);
        _mem[index].prev = index;
        _mem[index].next = index;
    }
    
    void insert(int listID, int ele) {
        del(ele);
        auto node = listID + _capacity;
        auto prev = _mem[node].prev;
        link(prev, ele);
        link(ele, static_cast<int>(node));
    }
    
    void insertFirst(int listID, int ele) {
        del(ele);
        auto node = listID + _capacity;
        auto next = _mem[node].next;
        link(static_cast<int>(node), ele);
        link(ele, next);
    }
    
    int getFirst(size_t listID) {
        return _mem[listID + _capacity].next;
    }
    
    int next(size_t index) {
        return _mem[index].next;
    }
    
    bool isHead(size_t index) {
        return index >= _capacity;
    }
    
    MyLinkList(size_t c, size_t n)
        : _capacity{c}, _list_count{n} {
        _mem.resize(n + c);
        for (int i = 0; i < _capacity + _list_count; ++i) {
            _mem[i].next = i;
            _mem[i].prev = i;
        }
    }
};
#endif
