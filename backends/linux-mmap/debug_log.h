#ifndef __FLOGFS_LINUX_MMAP_DEBUG_LOG_H_
#define __FLOGFS_LINUX_MMAP_DEBUG_LOG_H_

#include "flogfs.h"
#include "flogfs_private.h"

#include <cstdint>
#include <vector>
#include <list>

enum class OperationType {
    Opened,
    EraseBlock,
    WriteSector,
    WriteSpare
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

public:
    LogEntry(OperationType type,
             flog_block_idx_t block = FLOG_BLOCK_IDX_INVALID, flog_page_index_t page = FLOG_PAGE_IDX_INVALID,
             flog_sector_idx_t sector = FLOG_SECTOR_IDX_INVALID, void *ptr = nullptr, uint16_t offset = 0, uint16_t size = 0) :
        type_(type), block_(block), page_(page), sector_(sector), ptr_(ptr), offset_(offset), size_(size) {
    }

public:
    OperationType type() {
        return type_;
    }
};

class Log {
private:
    std::list<LogEntry> entries;

public:
    void append(LogEntry &&entry) {
        entries.emplace_back(entry);
    }

    uint32_t size() {
        return entries.size();
    }

    void clear() {
        entries.erase(entries.begin(), entries.end());
    }
};

#endif
