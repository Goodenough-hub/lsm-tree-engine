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

public:
    static Record createRecord(uint64_t tranc_id);
    static Record putRecord(uint64_t tranc_id, const std::string &key, const std::string &value);
    static Record deleteRecord(uint64_t tranc_id, const std::string &key);
    static Record commitRecord(uint64_t tranc_id);
    static Record rollbackRecord(uint64_t tranc_id);
};