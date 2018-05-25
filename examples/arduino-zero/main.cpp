#include <cstdarg>

#include <Arduino.h>
#include <SPI.h>

#include <flogfs.h>
#include <flogfs_arduino_sd.h>

static void flog_check(flog_result_t fr) {
    if (fr != FLOG_SUCCESS) {
        Serial.println("Fail!");
        while (true) {
        }
    }
}

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

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(10);
    }

    debugfln("Starting");

    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);

    SPI.begin();

    flog_check(flogfs_arduino_sd_open(12));

    debugfln("Initialize");

    flog_check(flogfs_init());

    debugfln("Formatting");

    flog_check(flogfs_format());

    debugfln("Mounting");

    flog_check(flogfs_mount());

    const char *pattern = "asdfasdf";
    const char *path = "data-1.bin";
    if (!flogfs_check_exists(path)) {
        size_t size = 0;
        flog_write_file_t file;

        debugfln("Creating %s...", path);

        flog_check(flogfs_open_write(&file, path));
        for (auto i = 0; i < 1024 * 26; ++i) {
            size += flogfs_write(&file, (uint8_t *)pattern, strlen(pattern));
        }
        flog_check(flogfs_close_write(&file));


        debugfln("Wrote file (%d bytes)", size);
    }

    if (flogfs_check_exists(path)) {
        flog_read_file_t file;

        debugfln("Opening %s", path);

        flog_check(flogfs_open_read(&file, path));

        debugfln("File size: %d", flogfs_read_file_size(&file));

        flog_check(flogfs_close_read(&file));
    }

    debugfln("Done");
}

void loop() {
}
