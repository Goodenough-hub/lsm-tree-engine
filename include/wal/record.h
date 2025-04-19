#pragma once

#include <cstdint>
#include <string>
enum class OperationType
{
    Put,
    Delete,
    Commit,
    Rollback,
    Create
};

class Record
{
private:
    uint64_t tranc_id_;
    OperationType op_type_;
    std::string key_;
    std::string value_;
    uint16_t record_len_;
};