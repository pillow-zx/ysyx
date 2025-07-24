#include "Log.h"

void Log(const std::string &msg, const std::string &log_file) {
    std::ofstream log_stream(log_file);
    if (!log_stream) {
        std::cerr << "Failed to open log file: " << log_file << std::endl;
        return;
    }

    log_stream << msg << std::endl;
    log_stream.close();
}