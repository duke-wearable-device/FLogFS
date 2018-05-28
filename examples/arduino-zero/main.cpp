#include <cstdarg>

#include <Arduino.h>
#include <SPI.h>

#include <flogfs.h>
#include <flogfs_arduino_sd.h>

constexpr const char *Pattern = "abcdefgh";

extern "C" void debugfln(const char *f, ...) {
    constexpr uint16_t DebugLineMax = 256;
    char buffer[DebugLineMax];
    va_list args;

    va_start(args, f);
    auto w = vsnprintf(buffer, DebugLineMax, f, args);
    va_end(args);

    buffer[w] = '\r';
    buffer[w + 1] = '\n';
    buffer[w + 2] = 0;

    Serial.print(buffer);
}

void flog_assertion_fail(const char *expression, const char *file, uint32_t line) {
    debugfln("Failed: %s:%d : %s", file, line, expression);
    while (true) {
    }
}

#define FLOG_CHECK(EX) (void)((EX) || (flog_assertion_fail(#EX, __FILE__, __LINE__), 0))

size_t write_file(const char *path) {
    if (!flogfs_check_exists(path)) {
        size_t size = 0;
        flog_write_file_t file;
        debugfln("Creating %s", path);
        FLOG_CHECK(flogfs_open_write(&file, path));
        debugfln("Writing %s", path);
        for (auto i = 0; i < 256; ++i) {
            size += flogfs_write(&file, (uint8_t *)Pattern, strlen(Pattern));
        }
        if (size != file.file_size) {
            debugfln("Size is wrong %d != %d", size, file.file_size);
        }

        debugfln("Closing size=%d block=%d sector=%d", file.file_size, file.block, file.sector);

        FLOG_CHECK(flogfs_close_write(&file));

        return size;
    }

    return 0;
}

void reopen_file(const char *path, size_t expected_size) {
    flog_write_file_t file;
    FLOG_CHECK(flogfs_open_write(&file, path));
    debugfln("Opened %s size=%d block=%d sector=%d", path, file.file_size, file.block, file.sector);
    if (expected_size != file.file_size) {
        debugfln("Size is wrong %d != %d", expected_size, file.file_size);
    }
    FLOG_CHECK(flogfs_close_write(&file));
}

void read_records(flog_read_file_t *file, size_t expected_size) {
    size_t actually_read = 0;

    while (true) {
        uint8_t buffer[strlen(Pattern) + 1];
        uint32_t bytes_read = flogfs_read(file, buffer, sizeof(buffer) - 1);
        if (bytes_read == 0) {
            break;
        }
        actually_read += bytes_read;
        buffer[sizeof(buffer) - 1] = 0;
        if (strcmp((const char *)buffer, Pattern) != 0) {
            debugfln("Error, garbiled read!");
        }
    }

    if (actually_read != expected_size) {
        debugfln("Read failed %d != %d", expected_size, actually_read);
    }
}

void read_file(const char *path, size_t expected_size) {
    flog_read_file_t file;
    debugfln("Reading %s", path);
    FLOG_CHECK(flogfs_open_read(&file, path));
    if (expected_size != file.file_size) {
        debugfln("Size is wrong %d != %d", expected_size, file.file_size);
    }

    read_records(&file, expected_size);

    FLOG_CHECK(flogfs_close_read(&file));
}

void seek_file(const char *path, size_t expected_size) {
    flog_read_file_t file;
    debugfln("Opening to test seek %s", path);

    FLOG_CHECK(flogfs_open_read(&file, path));

    size_t expected_bytes_to_be_read = strlen(Pattern) * 2;

    FLOG_CHECK(flogfs_read_seek(&file, expected_size - expected_bytes_to_be_read));

    read_records(&file, expected_bytes_to_be_read);

    FLOG_CHECK(flogfs_close_read(&file));
}

void ls_files() {
    flogfs_ls_iterator_t iter;
    char fname[256];

    flogfs_start_ls(&iter);

    while (flogfs_ls_iterate(&iter, fname)) {
        debugfln("LS: %s", fname);
    }

    flogfs_stop_ls(&iter);
}

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(10);
    }

    debugfln("Starting");

    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);

    SPI.begin();

    debugfln("Initialize");

    flog_init_params_t params {
        .number_of_blocks = 10,
        .pages_per_block = 64,
    };
    FLOG_CHECK(flogfs_arduino_sd_open(12, &params));

    FLOG_CHECK(flogfs_init(&params));

    debugfln("Mounting");

    if (flogfs_mount() == FLOG_FAILURE) {
        debugfln("Formatting");
        FLOG_CHECK(flogfs_format());
    }

    debugfln("Mounting");
    FLOG_CHECK(flogfs_mount());

    auto size1 = write_file("data-1.bin");
    auto size2 = write_file("data-2.bin");
    auto size3 = write_file("data-3.bin");

    read_file("data-1.bin", size1);
    read_file("data-2.bin", size2);
    read_file("data-3.bin", size3);

    reopen_file("data-1.bin", size1);
    reopen_file("data-2.bin", size2);
    reopen_file("data-3.bin", size3);

    seek_file("data-1.bin", size1);

    ls_files();

    debugfln("Done");
}

void loop() {
}
