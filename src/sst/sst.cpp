#include "../../include/sst/sst.h"

SSTBuilder::SSTBuilder(size_t block_size) : block_size(block_size) {
    meta_entries.clear();
    data.clear();
    first_key.clear();
    last_key.clear();
}

void SSTBuilder::add(const std::string &key, const std::string &value)
{
    if(first_key.empty())
    {
        first_key = key;
    }

    if(block.add_entry(key, value))
    {
        last_key = key;
        return;
    }
    
    finish_block();

    block.add_entry(key, value);
    first_key = key;
    last_key = key;
}