#include "debug_log.h"

std::ostream& operator<<(std::ostream& os, const LogEntry& e) {
    switch (e.type_) {
    case OperationType::Opened:
        os << "Opened()";
        break;
    case OperationType::EraseBlock:
        os << "EraseBlock(" << e.block_ << ")";
        break;
    case OperationType::WriteSector:
        os << "WriteSector(" << e.block_ << "." << e.page_ << "." << e.sector_ << ")";
        break;
    case OperationType::WriteSpare:
        os << "WriteSpare(" << e.block_ << "." << e.page_ << "." << e.sector_ << ")";
        break;
    case OperationType::FormatBegin:
        os << "FormatBegin()";
        break;
    case OperationType::FormatEnd:
        os << "FormatEnd()";
        break;
    case OperationType::PrimeBegin:
        os << "PrimeBegin()";
        break;
    case OperationType::PrimeEnd:
        os << "PrimeEnd()";
        break;
    default:
        os << "Unknown";
        break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Log& l) {
    for (auto &e : l.entries_) {
        os << e << std::endl;
    }
    return os;
}
