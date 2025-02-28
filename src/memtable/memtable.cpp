#include "../../include/memtable/memtable.h"
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>

Memtable::Memtable() : current_table(std::make_shared<SkipList>()), frozen_bytes(0) {}

void Memtable::put(const std::string &key, const std::string &value)
{
    std::unique_lock<std::shared_mutex> lock(rx_mutex);
    current_table->put(key, value);
}

void Memtable::remove(const std::string &key)
{
    std::unique_lock<std::shared_mutex> lock(rx_mutex);
    current_table->put(key, "");
}

void Memtable::clear()
{
    std::unique_lock<std::shared_mutex> lock(rx_mutex);
    frozen_tables.clear();
    frozen_bytes = 0;
}

std::optional<std::string> Memtable::get(const std::string &key)
{
    // current table中找
    std::shared_lock<std::shared_mutex> lock(rx_mutex);
    if(current_table->get(key).has_value()) 
    {
        return current_table->get(key).value();
    }
    
    // frozen table中找
    for(auto it = frozen_tables.begin(); it !=frozen_tables.end(); ++it)
    {
        if((*it)->get(key).has_value())
        {
            if((*it)->get(key).value().empty())
                return std::nullopt;
            else
                return (*it)->get(key).value();
        }
    }

    return std::nullopt;
}