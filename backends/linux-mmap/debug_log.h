#ifndef __FLOGFS_LINUX_MMAP_DEBUG_LOG_H_
#define __FLOGFS_LINUX_MMAP_DEBUG_LOG_H_

#include "flogfs.h"
#include "flogfs_private.h"

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#include <iostream>
#include <vector>
#include <list>

enum class OperationType {
    Opened,
    EraseBlock,
    WriteSector,
    WriteSpare,
    FormatBegin,
    FormatEnd,
    PrimeBegin,
    PrimeEnd,
};

class LogEntry {
private:
    OperationType type_;
    flog_block_idx_t block_;
    flog_page_index_t page_;
    flog_sector_idx_t sector_;
    void *ptr_;
    uint16_t offset_;
    uint16_t size_;
    void *copy_;

public:
    LogEntry(OperationType type,
             flog_block_idx_t block = FLOG_BLOCK_IDX_INVALID, flog_page_index_t page = FLOG_PAGE_IDX_INVALID,
             flog_sector_idx_t sector = FLOG_SECTOR_IDX_INVALID, void *ptr = nullptr, uint16_t offset = 0, uint16_t size = 0) :
        type_(type), block_(block), page_(page), sector_(sector), ptr_(ptr), offset_(offset), size_(size), copy_(nullptr) {
    }

    virtual ~LogEntry() {
        if (copy_ != nullptr) {
            free(copy_);
            copy_ = nullptr;
        }
    }

public:
    OperationType type() {
        return type_;
    }

    void backup() {
        assert(copy_ == nullptr);

        if (ptr_ != nullptr) {
            copy_ = malloc(size_);
            memcpy(copy_, ptr_, size_);
        }
    }

    void undo() {
        assert(copy_ != nullptr);

        if (ptr_ != nullptr) {
            memcpy(ptr_, copy_, size_);
        }
    }

public:
    friend std::ostream& operator<<(std::ostream& os, const LogEntry& e);

};

class Log {
private:
    bool copy_on_write_{ false };
    std::list<LogEntry> entries_;

public:
    void append(LogEntry &&entry) {
        entries_.emplace_back(std::move(entry));
        if (copy_on_write_) {
            entries_.back().backup();
        }
    }

    uint32_t size() {
        return entries_.size();
    }

    void clear() {
        entries_.erase(entries_.begin(), entries_.end());
    }

    void copy_on_write(bool enabled) {
        copy_on_write_ = enabled;
    }

public:
    friend std::ostream& operator<<(std::ostream& os, const Log& e);

};

#endif
