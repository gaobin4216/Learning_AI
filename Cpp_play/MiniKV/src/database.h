#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <sqlite3.h>

struct KVEntry {
    std::string key;
    std::vector<uint8_t> value;
    bool compressed;
    int64_t created_at;
    int64_t updated_at;
};

class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void put(const std::string& key, const std::vector<uint8_t>& value, bool compressed);
    std::optional<KVEntry> get(const std::string& key);
    bool remove(const std::string& key);
    std::vector<std::string> list_keys(const std::string& pattern = "%");
    size_t count();
    void clear();

private:
    void exec(const std::string& sql);
    sqlite3* db_ = nullptr;
};
