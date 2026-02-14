#pragma once

#include <vector>
#include "catalog/column.h"

namespace mydb {

class Schema {
public:
    explicit Schema(const std::vector<Column>& columns) : columns_(columns) {}

    const std::vector<Column>& GetColumns() const { return columns_; }
    
    const Column& GetColumn(uint32_t col_idx) const {
        return columns_[col_idx];
    }

    uint32_t GetColumnCount() const { return static_cast<uint32_t>(columns_.size()); }

private:
    std::vector<Column> columns_;
};

} // namespace mydb
