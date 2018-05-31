#ifndef __UTILITIES_H_INCLUDED
#define __UTILITIES_H_INCLUDED

#include <vector>
#include <string>
#include <cstdint>

bool initialize_and_open(bool truncate = true);

bool flush_and_close();

std::vector<std::string> generate_random_file_names(int32_t number);

std::vector<std::string> get_file_listing();

struct GeneratedFile {
    flog_write_file_t file;
    std::string name;
    size_t size;
    size_t written;
};

std::vector<GeneratedFile> write_files_randomly(std::vector<std::string> &names, uint8_t number_of_iterations, uint32_t min_size, uint32_t max_size);

struct INodeSector {
    flog_sector_idx_t sector;
    bool valid;
    flog_file_id_t file_id;
    std::string name;
    flog_timestamp_t created_at;
    flog_timestamp_t deleted_at;
    flog_block_idx_t first_block;
    flog_block_idx_t last_block;
    bool deleted;

    INodeSector(const flogfs_walk_inode_block_state_t &s) :
        sector(s.sector), valid(s.valid), file_id(s.file_id), name(s.file_name), first_block(s.first_block), created_at(s.created_at),
        deleted_at(s.deleted_at), last_block(s.last_block), deleted(s.deleted) {
    }
};

struct FileSector {
    flog_sector_idx_t sector;
    bool valid;
    uint16_t size;

    FileSector(const flogfs_walk_file_block_state_t &s) :
        sector(s.sector), valid(s.valid), size(s.size) {
    }
};

struct BlockAnalysis {
    flog_block_idx_t block;
    bool valid_block;
    flog_block_idx_t next_block;
    flog_block_age_t age;
    uint8_t type_id;
    flog_file_id_t file_id;
    uint32_t bytes_in_block;
    std::vector<INodeSector> inodes;
    std::vector<FileSector> files;

    bool is_file() {
        return type_id == FLOG_BLOCK_TYPE_FILE;
    }

    bool is_inode() {
        return type_id == FLOG_BLOCK_TYPE_INODE;
    }

    bool has_more() {
        return next_block != FLOG_BLOCK_IDX_ERASED;
    }

    std::vector<INodeSector> file_entries() {
        std::vector<INodeSector> f;
        for (auto &in : inodes) {
            if (in.valid && !in.deleted) {
                f.push_back(in);
            }
        }
        return f;
    }

    BlockAnalysis(flogfs_walk_state_t *state) :
        block(state->block), valid_block(state->valid_block), next_block(state->next_block),
        age(state->age), type_id(state->type_id), file_id(state->file_id), bytes_in_block(state->bytes_in_block) {
    }

    friend std::ostream& operator<<(std::ostream& os, const BlockAnalysis& e);
};

class Analysis {
private:
    std::vector<BlockAnalysis> blocks_;

public:
    std::vector<BlockAnalysis> &blocks() {
        return blocks_;
    }

    void append(BlockAnalysis &&ba);

    friend std::ostream& operator<<(std::ostream& os, const Analysis& e);

    BlockAnalysis &operator[](int32_t index) {
        return blocks_[index];
    }

public:
    bool verify();

public:
    int32_t number_of_blocks(uint8_t type) const;
    int32_t number_of_inode_blocks() const;
    int32_t number_of_file_blocks() const;
    int32_t number_of_files() const;

};

Analysis analyze_file_system();

#endif
