#include "../../include/wal/record.h"
#include <cstddef>
#include <cstring>

Record Record::createRecord(uint64_t tranc_id) {
  Record record;
  record.tranc_id_ = tranc_id;
  record.op_type_ = OperationType::Create;
  record.record_len_ = sizeof(record_len_) + sizeof(tranc_id_) + sizeof(Record);
  return record;
}

Record Record::putRecord(uint64_t tranc_id, const std::string &key,
                         const std::string &value) {
  Record record;
  record.tranc_id_ = tranc_id;
  record.op_type_ = OperationType::Put;
  record.key_ = key;
  record.value_ = value;
  record.record_len_ = sizeof(record_len_) + sizeof(tranc_id_) +
                       sizeof(Record) + sizeof(uint16_t) + key.size() +
                       sizeof(uint16_t) + value.size();
}

Record Record::deleteRecord(uint64_t tranc_id, const std::string &key) {
  Record record;
  record.tranc_id_ = tranc_id;
  record.op_type_ = OperationType::Delete;
  record.key_ = key;
  record.record_len_ = sizeof(record_len_) + sizeof(tranc_id_) +
                       sizeof(Record) + sizeof(uint16_t) + key.size();
  return record;
}

Record Record::rollbackRecord(uint64_t tranc_id) {
  Record record;
  record.tranc_id_ = tranc_id;
  record.op_type_ = OperationType::Rollback;
  record.record_len_ = sizeof(record_len_) + sizeof(tranc_id_) + sizeof(Record);
  return record;
}

Record Record::commitRecord(uint64_t tranc_id) {
  Record record;
  record.tranc_id_ = tranc_id;
  record.op_type_ = OperationType::Commit;
  record.record_len_ = sizeof(record_len_) + sizeof(tranc_id_) + sizeof(Record);
  return record;
}

// |  record_len_ | tranc_id_  | op_type_ | key_len_ (op) | key_ (op)|
// value_len_ (op)| value_ (op)|
std::vector<uint8_t> Record::encode() const {
  std::vector<uint8_t> encoded_record;

  size_t key_offset =
      sizeof(record_len_) + sizeof(tranc_id_) + sizeof(op_type_);

  encoded_record.resize(record_len_, 0);

  // record_len_
  std::memcpy(encoded_record.data(), &record_len_, sizeof(record_len_));

  // tranc_id_
  std::memcpy(encoded_record.data() + sizeof(record_len_), &tranc_id_,
              sizeof(tranc_id_));

  // op_type_
  std::memcpy(encoded_record.data() + sizeof(record_len_) + sizeof(tranc_id_),
              &op_type_, sizeof(op_type_));
  if (op_type_ == OperationType::Put) {
    uint16_t key_len = key_.size();
    std::memcpy(encoded_record.data() + key_offset, &key_len, sizeof(key_len));
    std::memcpy(encoded_record.data() + key_offset + sizeof(key_len),
                key_.data(), key_len);

    uint16_t value_len = value_.size();
    std::memcpy(encoded_record.data() + key_offset + sizeof(key_len) + key_len,
                &value_len, sizeof(value_len));
    std::memcpy(encoded_record.data() + key_offset + sizeof(key_len) + key_len +
                    sizeof(value_len),
                &value_, value_len);
  } else if (op_type_ == OperationType::Delete) {
    uint16_t key_len = key_.size();
    std::memcpy(encoded_record.data() + key_offset, &key_len, sizeof(key_len));
    std::memcpy(encoded_record.data() + key_offset + sizeof(key_len),
                key_.data(), key_len);
  }

  return encoded_record;
}

std::vector<Record> Record::decode(const std::vector<uint8_t> &data) {
  if (data.size() < sizeof(uint16_t) + sizeof(uint64_t) + sizeof(uint8_t)) {
    return {};
  }

  std::vector<Record> records;
  size_t pos = 0;

  while (pos < data.size()) {
    // 读取 record_len_
    uint16_t record_len;
    std::memcpy(&record_len, data.data() + pos, sizeof(record_len));
    pos += sizeof(record_len);

    // 读取 tranc_id_
    uint64_t tranc_id;
    std::memcpy(&tranc_id, data.data() + pos, sizeof(tranc_id));
    pos += sizeof(tranc_id);

    // 读取 op_type_
    OperationType op_type;
    std::memcpy(&op_type, data.data() + pos, sizeof(op_type));
    pos += sizeof(op_type);

    Record record;
    record.record_len_ = record_len;
    record.tranc_id_ = tranc_id;
    record.op_type_ = op_type;

    if (op_type == OperationType::Put) {
      // 读取 key_len_
      uint16_t key_len;
      std::memcpy(&key_len, data.data() + pos, sizeof(key_len));
      pos += sizeof(key_len);

      // 读取 key_
      record.key_ = std::string(
          reinterpret_cast<const char *>(data.data() + pos), key_len);
      pos += key_len;

      // 读取 key_len_
      uint16_t value_len;
      std::memcpy(&value_len, data.data() + pos, sizeof(value_len));
      pos += sizeof(value_len);

      // 读取 value_
      record.value_ = std::string(
          reinterpret_cast<const char *>(data.data() + pos), value_len);
      pos += value_len;
    } else if (op_type == OperationType::Delete) {
      // 读取 key_len_
      uint16_t key_len;
      std::memcpy(&key_len, data.data() + pos, sizeof(key_len));
      pos += sizeof(key_len);

      // 读取 key_
      record.key_ = std::string(
          reinterpret_cast<const char *>(data.data() + pos), key_len);
      pos += key_len;
    }
    records.push_back(record);
  }
  return records;
}