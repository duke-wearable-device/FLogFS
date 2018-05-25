#include <cstdarg>

#include <Arduino.h>

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

    Serial.println("Starting");

    flog_check(flogfs_arduino_sd_open(12));

    flog_check(flogfs_init());

    flog_check(flogfs_format());

    flog_check(flogfs_mount());
}

void loop() {
}
