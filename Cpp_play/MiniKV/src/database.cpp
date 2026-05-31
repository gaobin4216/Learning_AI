#include "database.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database: " + std::string(sqlite3_errmsg(db_)));
    }
    spdlog::info("Database opened: {}", path);

    exec("PRAGMA journal_mode=WAL");
    exec(R"(
        CREATE TABLE IF NOT EXISTS kv_store (
            key        TEXT PRIMARY KEY,
            value      BLOB NOT NULL,
            compressed INTEGER DEFAULT 0,
            created_at INTEGER DEFAULT (strftime('%s','now')),
            updated_at INTEGER DEFAULT (strftime('%s','now'))
        )
    )");
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
        spdlog::debug("Database closed");
    }
}

void Database::exec(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw std::runtime_error("SQL error: " + msg);
    }
}

void Database::put(const std::string& key, const std::vector<uint8_t>& value, bool compressed) {
    const char* sql = R"(
        INSERT INTO kv_store (key, value, compressed, created_at, updated_at)
        VALUES (?, ?, ?, strftime('%s','now'), strftime('%s','now'))
        ON CONFLICT(key) DO UPDATE SET
            value = excluded.value,
            compressed = excluded.compressed,
            updated_at = strftime('%s','now')
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 2, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, compressed ? 1 : 0);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to put key: " + std::string(sqlite3_errmsg(db_)));
    }
    sqlite3_finalize(stmt);
    spdlog::debug("PUT key='{}' size={} compressed={}", key, value.size(), compressed);
}

std::optional<KVEntry> Database::get(const std::string& key) {
    const char* sql = "SELECT key, value, compressed, created_at, updated_at FROM kv_store WHERE key = ?";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<KVEntry> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        KVEntry entry;
        entry.key = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

        const void* blob = sqlite3_column_blob(stmt, 1);
        int blob_size = sqlite3_column_bytes(stmt, 1);
        entry.value.assign(
            static_cast<const uint8_t*>(blob),
            static_cast<const uint8_t*>(blob) + blob_size
        );

        entry.compressed = sqlite3_column_int(stmt, 2) != 0;
        entry.created_at = sqlite3_column_int64(stmt, 3);
        entry.updated_at = sqlite3_column_int64(stmt, 4);
        result = std::move(entry);
    }

    sqlite3_finalize(stmt);
    return result;
}

bool Database::remove(const std::string& key) {
    const char* sql = "DELETE FROM kv_store WHERE key = ?";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    int changes = sqlite3_changes(db_);
    sqlite3_finalize(stmt);

    spdlog::debug("DELETE key='{}' found={}", key, changes > 0);
    return changes > 0;
}

std::vector<std::string> Database::list_keys(const std::string& pattern) {
    const char* sql = "SELECT key FROM kv_store WHERE key LIKE ? ORDER BY key";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }

    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);

    std::vector<std::string> keys;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        keys.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    }

    sqlite3_finalize(stmt);
    return keys;
}

size_t Database::count() {
    const char* sql = "SELECT COUNT(*) FROM kv_store";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }

    size_t n = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        n = static_cast<size_t>(sqlite3_column_int64(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return n;
}

void Database::clear() {
    exec("DELETE FROM kv_store");
    spdlog::info("Database cleared");
}
