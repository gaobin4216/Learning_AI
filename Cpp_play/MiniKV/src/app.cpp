#include "app.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <fmt/color.h>

App::App() = default;

void App::load_config(const std::string& config_path) {
    std::ifstream f(config_path);
    if (!f.is_open()) {
        spdlog::warn("Config file not found: {}, using defaults", config_path);
        return;
    }

    try {
        nlohmann::json j = nlohmann::json::parse(f);

        if (j.contains("database")) {
            auto& db = j["database"];
            if (db.contains("path")) config_.db_path = db["path"];
            if (db.contains("max_value_size_mb")) config_.max_value_size_mb = db["max_value_size_mb"];
        }
        if (j.contains("compression")) {
            auto& c = j["compression"];
            if (c.contains("enabled")) config_.compression_enabled = c["enabled"];
            if (c.contains("threshold_bytes")) config_.compression_threshold = c["threshold_bytes"];
        }
        if (j.contains("logging")) {
            auto& l = j["logging"];
            if (l.contains("level")) config_.log_level = l["level"];
            if (l.contains("file")) config_.log_file = l["file"];
        }
        spdlog::info("Config loaded from {}", config_path);
    } catch (const std::exception& e) {
        spdlog::error("Failed to parse config: {}", e.what());
    }
}

void App::init_logging() {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(config_.log_file));

    auto logger = std::make_shared<spdlog::logger>("minikv", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::from_str(config_.log_level));
    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
}

int App::run(int argc, char* argv[]) {
    cxxopts::Options options("minikv", "A lightweight CLI key-value database");

    options.add_options()
        ("c,config", "Config file path", cxxopts::value<std::string>()->default_value("config/default.json"))
        ("h,help", "Print help")
        ("command", "Command to execute", cxxopts::value<std::string>())
        ("args", "Command arguments", cxxopts::value<std::vector<std::string>>())
        ;

    options.parse_positional({"command", "args"});

    try {
        auto result = options.parse(argc, argv);

        bool show_help = result.count("help") || !result.count("command");
        if (!show_help && result.count("command")) {
            std::string cmd_check = result["command"].as<std::string>();
            if (cmd_check == "help" || cmd_check == "--help" || cmd_check == "-h") {
                show_help = true;
            }
        }
        if (show_help) {
            fmt::print(fg(fmt::color::cyan), "MiniKV v1.0.0");
            fmt::print(" - Lightweight CLI Key-Value Database\n\n");
            std::cout << options.help() << "\n";
            fmt::print(fg(fmt::color::yellow), "Usage:\n");
            fmt::print("  minikv put <key> <value>\n");
            fmt::print("  minikv get <key>\n");
            fmt::print("  minikv rm <key>\n");
            fmt::print("  minikv list [pattern]\n");
            fmt::print("  minikv search <pattern>\n");
            fmt::print("  minikv export <file>\n");
            fmt::print("  minikv import <file>\n");
            fmt::print("  minikv stats\n");
            fmt::print("  minikv clear\n");
            fmt::print("  minikv config\n");
            return 0;
        }

        // Load config and init
        std::string config_path = result["config"].as<std::string>();
        load_config(config_path);
        init_logging();

        db_ = std::make_unique<Database>(config_.db_path);

        std::string cmd = result["command"].as<std::string>();
        std::vector<std::string> args;
        if (result.count("args")) {
            args = result["args"].as<std::vector<std::string>>();
        }

        // Dispatch commands
        if (cmd == "put") {
            if (args.size() < 2) {
                fmt::print(fg(fmt::color::red), "Usage: minikv put <key> <value>\n");
                return 1;
            }
            cmd_put(args[0], args[1]);
        } else if (cmd == "get") {
            if (args.empty()) {
                fmt::print(fg(fmt::color::red), "Usage: minikv get <key>\n");
                return 1;
            }
            cmd_get(args[0]);
        } else if (cmd == "rm") {
            if (args.empty()) {
                fmt::print(fg(fmt::color::red), "Usage: minikv rm <key>\n");
                return 1;
            }
            cmd_remove(args[0]);
        } else if (cmd == "list") {
            cmd_list(args.empty() ? "*" : args[0]);
        } else if (cmd == "search") {
            if (args.empty()) {
                fmt::print(fg(fmt::color::red), "Usage: minikv search <pattern>\n");
                return 1;
            }
            cmd_search(args[0]);
        } else if (cmd == "export") {
            if (args.empty()) {
                fmt::print(fg(fmt::color::red), "Usage: minikv export <file>\n");
                return 1;
            }
            cmd_export(args[0]);
        } else if (cmd == "import") {
            if (args.empty()) {
                fmt::print(fg(fmt::color::red), "Usage: minikv import <file>\n");
                return 1;
            }
            cmd_import(args[0]);
        } else if (cmd == "stats") {
            cmd_stats();
        } else if (cmd == "clear") {
            db_->clear();
            fmt::print(fg(fmt::color::green), "Database cleared.\n");
        } else if (cmd == "config") {
            cmd_config();
        } else {
            fmt::print(fg(fmt::color::red), "Unknown command: {}\n", cmd);
            fmt::print("Use --help for usage.\n");
            return 1;
        }

    } catch (const cxxopts::exceptions::exception& e) {
        fmt::print(fg(fmt::color::red), "Argument error: {}\n", e.what());
        return 1;
    } catch (const std::exception& e) {
        spdlog::error("Fatal: {}", e.what());
        fmt::print(fg(fmt::color::red), "Error: {}\n", e.what());
        return 1;
    }

    return 0;
}

void App::cmd_put(const std::string& key, const std::string& value) {
    std::vector<uint8_t> data(value.begin(), value.end());
    bool compressed = false;

    if (config_.compression_enabled && data.size() >= config_.compression_threshold) {
        data = Compressor::compress(data);
        compressed = true;
    }

    db_->put(key, data, compressed);

    fmt::print(fg(fmt::color::green), "OK");
    fmt::print(" key='{}' size={}", key, value.size());
    if (compressed) {
        fmt::print(fg(fmt::color::yellow), " (compressed)");
    }
    fmt::print("\n");
    spdlog::info("PUT key='{}' original_size={} compressed={}", key, value.size(), compressed);
}

void App::cmd_get(const std::string& key) {
    auto entry = db_->get(key);
    if (!entry) {
        fmt::print(fg(fmt::color::red), "Key not found: {}\n", key);
        return;
    }

    std::vector<uint8_t> data = entry->value;
    if (entry->compressed) {
        data = Compressor::decompress(data);
    }

    std::string value(data.begin(), data.end());
    fmt::print(fg(fmt::color::cyan), "{}", value);
    fmt::print("\n");
    spdlog::info("GET key='{}' size={}", key, data.size());
}

void App::cmd_remove(const std::string& key) {
    if (db_->remove(key)) {
        fmt::print(fg(fmt::color::green), "Deleted: {}\n", key);
    } else {
        fmt::print(fg(fmt::color::red), "Key not found: {}\n", key);
    }
}

void App::cmd_list(const std::string& pattern) {
    std::string sql_pattern = pattern;
    if (pattern == "*") sql_pattern = "%";

    auto keys = db_->list_keys(sql_pattern);
    if (keys.empty()) {
        fmt::print(fg(fmt::color::yellow), "No keys found.\n");
        return;
    }

    fmt::print(fg(fmt::color::cyan), "Keys ({}):\n", keys.size());
    for (const auto& k : keys) {
        fmt::print("  {}\n", k);
    }
}

void App::cmd_search(const std::string& pattern) {
    std::string sql_pattern = "%" + pattern + "%";
    auto keys = db_->list_keys(sql_pattern);

    if (keys.empty()) {
        fmt::print(fg(fmt::color::yellow), "No matches for '{}'.\n", pattern);
        return;
    }

    fmt::print(fg(fmt::color::cyan), "Matches for '{}' ({}):\n", pattern, keys.size());
    for (const auto& k : keys) {
        fmt::print("  {}\n", k);
    }
}

void App::cmd_export(const std::string& path) {
    auto keys = db_->list_keys();
    nlohmann::json j;

    for (const auto& key : keys) {
        auto entry = db_->get(key);
        if (!entry) continue;

        std::vector<uint8_t> data = entry->value;
        if (entry->compressed) {
            data = Compressor::decompress(data);
        }

        j[key] = {
            {"value", std::string(data.begin(), data.end())},
            {"compressed", entry->compressed},
            {"created_at", entry->created_at},
            {"updated_at", entry->updated_at}
        };
    }

    std::ofstream f(path);
    f << j.dump(2);

    fmt::print(fg(fmt::color::green), "Exported {} entries to {}\n", j.size(), path);
    spdlog::info("Exported {} entries to {}", j.size(), path);
}

void App::cmd_import(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        fmt::print(fg(fmt::color::red), "Cannot open file: {}\n", path);
        return;
    }

    nlohmann::json j = nlohmann::json::parse(f);
    size_t imported = 0;

    for (auto& [key, val] : j.items()) {
        std::string value_str = val["value"];
        std::vector<uint8_t> data(value_str.begin(), value_str.end());
        bool compressed = false;

        if (config_.compression_enabled && data.size() >= config_.compression_threshold) {
            data = Compressor::compress(data);
            compressed = true;
        }

        db_->put(key, data, compressed);
        imported++;
    }

    fmt::print(fg(fmt::color::green), "Imported {} entries from {}\n", imported, path);
    spdlog::info("Imported {} entries from {}", imported, path);
}

void App::cmd_stats() {
    size_t count = db_->count();
    auto keys = db_->list_keys();

    size_t total_raw = 0, total_stored = 0, compressed_count = 0;

    for (const auto& key : keys) {
        auto entry = db_->get(key);
        if (!entry) continue;

        total_stored += entry->value.size();
        compressed_count += entry->compressed ? 1 : 0;

        if (entry->compressed) {
            auto decompressed = Compressor::decompress(entry->value);
            total_raw += decompressed.size();
        } else {
            total_raw += entry->value.size();
        }
    }

    fmt::print(fg(fmt::color::cyan), "=== MiniKV Statistics ===\n");
    fmt::print("  Entries:          {}\n", count);
    fmt::print("  Compressed:       {}\n", compressed_count);
    fmt::print("  Raw data size:    {} bytes\n", total_raw);
    fmt::print("  Stored size:      {} bytes\n", total_stored);
    if (total_raw > 0) {
        fmt::print("  Compression ratio: {:.1f}%\n",
                    (1.0 - static_cast<double>(total_stored) / total_raw) * 100);
    }
    fmt::print("  Database file:    {}\n", config_.db_path);
}

void App::cmd_config() {
    nlohmann::json j;
    j["database"]["path"] = config_.db_path;
    j["database"]["max_value_size_mb"] = config_.max_value_size_mb;
    j["compression"]["enabled"] = config_.compression_enabled;
    j["compression"]["threshold_bytes"] = config_.compression_threshold;
    j["logging"]["level"] = config_.log_level;
    j["logging"]["file"] = config_.log_file;

    fmt::print(fg(fmt::color::cyan), "Current configuration:\n");
    std::cout << j.dump(2) << "\n";
}
