#pragma once
#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "database.h"
#include "compressor.h"

struct AppConfig {
    std::string db_path = "minikv.db";
    size_t max_value_size_mb = 16;
    bool compression_enabled = true;
    size_t compression_threshold = 256;
    std::string log_level = "info";
    std::string log_file = "minikv.log";
};

class App {
public:
    App();
    int run(int argc, char* argv[]);

private:
    void load_config(const std::string& config_path);
    void init_logging();

    // commands
    void cmd_put(const std::string& key, const std::string& value);
    void cmd_get(const std::string& key);
    void cmd_remove(const std::string& key);
    void cmd_list(const std::string& pattern);
    void cmd_search(const std::string& pattern);
    void cmd_export(const std::string& path);
    void cmd_import(const std::string& path);
    void cmd_stats();
    void cmd_config();

    AppConfig config_;
    std::unique_ptr<Database> db_;
};
