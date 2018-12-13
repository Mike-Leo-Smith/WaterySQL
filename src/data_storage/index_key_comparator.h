//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_INDEX_KEY_COMPARATOR_H
#define WATERYSQL_INDEX_KEY_COMPARATOR_H

#include "field_descriptor.h"
#include "data_comparator.h"

namespace watery {

class IndexKeyComparator {

private:
    bool _unique;
    DataComparator _data_cmp;

public:
    IndexKeyComparator() = default;
    
    explicit IndexKeyComparator(const FieldDescriptor &fd)
        : _unique{fd.constraints.unique()}, _data_cmp{fd.data_descriptor} {}
    
    bool less(const Byte *lhs, const Byte *rhs) const noexcept {
        if (_unique) {
            return _data_cmp.less(lhs, rhs);
        }
        auto data_cmp_result = _data_cmp.compare(&lhs[sizeof(RecordOffset)], &rhs[sizeof(RecordOffset)]);
        return (data_cmp_result < 0) ||
               (data_cmp_result == 0 &&
                MemoryMapper::map_memory<int64_t>(lhs) < MemoryMapper::map_memory<int64_t>(rhs));
    }
    
};

}

#endif  // WATERYSQL_INDEX_KEY_COMPARATOR_H
