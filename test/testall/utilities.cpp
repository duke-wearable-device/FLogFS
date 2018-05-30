#include <iostream>
#include <sstream>

#include <flogfs.h>
#include <flogfs_linux_mmap.h>

#include <gtest/gtest.h>

#include "utilities.h"

constexpr const char *Pattern = "abcdefgh";

static flog_initialize_params_t params { 32, 16 };

bool initialize_and_open(bool truncate) {
    EXPECT_TRUE(flogfs_linux_open("tests.bin", truncate, &params));
    EXPECT_TRUE(flogfs_initialize(&params));
    if (truncate) {
        EXPECT_TRUE(flogfs_format());
    }
    EXPECT_TRUE(flogfs_mount());

    std::cout << "Theoretical capacity: " << (params.number_of_blocks - 1) * params.pages_per_block * FS_SECTORS_PER_PAGE * FS_SECTOR_SIZE << std::endl;

    return true;
}

std::vector<std::string> generate_random_file_names(int32_t number) {
    std::vector<std::string> names;
    for (auto i = 0; i < number; ++i) {
        char file_name[32];
        snprintf(file_name, sizeof(file_name), "file-%02d.bin", i);
        names.emplace_back(file_name);
    }
    return names;
}

std::vector<std::string> get_file_listing() {
    std::vector<std::string> names;
    flogfs_ls_iterator_t iter;
    char file_name[32];
    flogfs_start_ls(&iter);
    while (flogfs_ls_iterate(&iter, file_name)) {
        names.emplace_back(file_name);
    }
    flogfs_stop_ls(&iter);
    return names;
}

std::vector<GeneratedFile> write_files_randomly(std::vector<std::string> &names, uint8_t number_of_iterations, uint32_t min_size, uint32_t max_size) {
    std::vector<GeneratedFile> files(names.size());
    for (auto i = 0; i < names.size(); ++i) {
        files[i].name = names[i];
    }

    for (auto j = 0; j < number_of_iterations; ++j) {
        auto total_written = 0;

        if (j > 0) {
            for (auto &file : files) {
                EXPECT_TRUE(flogfs_rm(file.name.c_str()));
            }
        }

        for (auto &file : files) {
            file.size = (random() % (max_size - min_size)) + min_size;
            file.written = 0;

            EXPECT_TRUE(flogfs_open_write(&file.file, file.name.c_str()));
        }

        while (true) {
            auto done = true;

            for (auto &file : files) {
                auto remaining = file.size - file.written;
                if (remaining > 0) {
                    auto random_block_size = (uint32_t)(random() % 4096) + 256;
                    auto to_write = std::min(random_block_size, (uint32_t)remaining);
                    while (to_write > 0) {
                        auto writing = std::min((uint32_t)to_write, (uint32_t)strlen(Pattern));
                        auto wrote = flogfs_write(&file.file, (uint8_t *)Pattern, writing);
                        if (wrote != writing) {
                            assert(wrote == writing);
                        }
                        to_write -= wrote;
                        file.written += wrote;
                        total_written += wrote;
                        done = false;
                    }
                }
            }

            if (done) {
                break;
            }
        }

        for (auto &file : files) {
            EXPECT_TRUE(flogfs_close_write(&file.file));
        }

        std::cout << "Total Written: " << total_written << " bytes" << std::endl;
    }

    return files;
}

void Analysis::append(BlockAnalysis &&ba) {
    blocks_.emplace_back(std::move(ba));
}

int32_t Analysis::number_of_blocks(uint8_t type) const {
    int32_t n = 0;
    for (auto &ba : blocks_) {
        if (ba.valid_block && ba.type_id == type) {
            n++;
        }
    }
    return n;
}

int32_t Analysis::number_of_inode_blocks() const {
    return number_of_blocks(FLOG_BLOCK_TYPE_INODE);
}

int32_t Analysis::number_of_file_blocks() const {
    return number_of_blocks(FLOG_BLOCK_TYPE_FILE);
}

inline void append(std::ostream& os, std::vector<std::string> &strings, const char *delimitter) {
    auto nf = false;
    for (auto &s : strings) {
        if (s.length() > 0) {
            if (nf) {
                os << delimitter;
            }
            os << s;
            nf = true;
        }
    }
}

struct rle_helper {
    int32_t counter{ 0 };
    std::string name;

    rle_helper(std::string name) : name(name) {
    }

    void add() {
        counter++;
    }

    std::string str() {
        std::ostringstream s;
        if (counter > 0) {
            s << "(" << counter << " " << name << ")";
            counter = 0;
        }
        return s.str();
    }
};

std::ostream& operator<<(std::ostream& os, const std::vector<INodeSector>& sectors) {
    std::vector<std::string> p;
    auto invalid = rle_helper{ "invalid" };
    auto deleted = rle_helper{ "deleted" };
    for (auto &inode : sectors) {
        std::ostringstream ss;
        if (inode.valid) {
            if (inode.deleted) {
                if (false) {
                    ss << "D(#" << inode.file_id;
                    ss << " " << inode.name;
                    ss << " " << inode.first_block << "/" << inode.last_block << ")";
                    p.push_back(ss.str());
                }
                else {
                    deleted.add();
                }
            }
            else {
                p.push_back(deleted.str());
                ss << "(#" << inode.file_id;
                ss << " " << inode.name;
                ss << " #" << inode.first_block << ")";
                p.push_back(ss.str());
            }
        }
        else {
            invalid.add();
        }
    }

    p.push_back(deleted.str());
    p.push_back(invalid.str());

    append(os, p, " ");

    return os ;
}

std::ostream& operator<<(std::ostream& os, const std::vector<FileSector>& sectors) {
    std::vector<std::string> p;
    auto invalid = rle_helper{ "invalid" };
    auto complete = rle_helper{ "complete" };
    for (auto &fb : sectors) {
        std::ostringstream ss;
        if (fb.valid) {
            if (fb.size == FS_SECTOR_SIZE) {
                complete.add();
            }
            else {
                p.push_back(complete.str());
                ss << fb.size;
                p.push_back(ss.str());
            }
        }
        else {
            invalid.add();
        }
    }

    p.push_back(complete.str());
    p.push_back(invalid.str());

    append(os, p, " ");

    return os;
}

std::ostream& operator<<(std::ostream& os, const BlockAnalysis& ba) {
    os << "Block<" << ba.block;
    if (ba.valid_block)  {
        os << ", ";

        switch (ba.type_id) {
        case FLOG_BLOCK_TYPE_ERROR: os << "Error"; break;
        case FLOG_BLOCK_TYPE_UNALLOCATED: os << "Unallocated"; break;
        case FLOG_BLOCK_TYPE_INODE: {
            os << "INode " << ba.inodes;
            if (ba.next_block != FLOG_BLOCK_IDX_INVALID) {
                os << " -> " << ba.next_block << "";
            }
            break;
        }
        case FLOG_BLOCK_TYPE_FILE: {
            os << "#" << ba.file_id << " (" << ba.files << ")";
            if (ba.next_block != FLOG_BLOCK_IDX_INVALID) {
                os << " " << ba.bytes_in_block << " -> " << ba.next_block << "";
            }
            break;
        }
        }
    }
    os << ">";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Analysis& a) {
    os << "Analysis<";
    os << "inodes=" << a.number_of_inode_blocks() << " ";
    os << "files=" << a.number_of_file_blocks();
    os << ">";

    return os;
}

static flog_result_t analysis_callback(flogfs_walk_state_t *state, void *arg) {
    Analysis *analysis = (Analysis *)arg;

    if (analysis->blocks().empty() || analysis->blocks().back().block != state->block) {
        analysis->append(BlockAnalysis{ state });
    }
    else {
        auto &ba = analysis->blocks().back();

        switch (state->type_id) {
        case FLOG_BLOCK_TYPE_INODE: {
            ba.inodes.emplace_back(INodeSector{ state->types.inode });
            break;
        }
        case FLOG_BLOCK_TYPE_FILE: {
            ba.files.emplace_back(FileSector{ state->types.file });
            break;
        }
        }
    }

    return FLOG_SUCCESS;
}

Analysis analyze_file_system() {
    Analysis analysis;

    EXPECT_TRUE(flog_walk(analysis_callback, &analysis));

    for (auto &b : analysis.blocks()) {
        std::cout << b << std::endl;
    }

    std::cout << analysis << std::endl;

    return analysis;
}
