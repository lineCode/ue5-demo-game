// Copyright (c) 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace blackbox {
namespace types {
struct client_desc {
    std::string build_id;
    std::string country;
    std::string computer_name;
    std::string user_name;
    std::string cpu;
    std::string game_version_id;
    std::string gpu;
    std::string gpu_driver;
    std::string locale;
    uint64_t memory;
    std::string operating_system;
    std::string operating_version;
    std::string renderer;
    std::string session_started_at;
    std::string system_architecture;
};

struct client_desc_editor {
    std::string country;
    std::string computer_name;
    std::string user_name;
    std::string cpu;
    std::string game_version_id;
    std::string gpu;
    std::string gpu_driver;
    std::string locale;
    uint64_t memory;
    std::string operating_system;
    std::string operating_version;
    std::string renderer;
    std::string session_started_at;
    std::string system_architecture;
};

struct session_info_update_desc {
    std::string computer_name;
    std::string user_name;
    std::string cpu;
    uint64_t memory;
    std::string operating_system;
    std::string operating_version;
    std::string system_architecture;
};

struct session_info_update_desc_playtest {
    std::string computer_name;
    std::string user_name;
    std::string cpu;
    uint64_t memory{};
    std::string operating_system;
    std::string operating_version;
    std::string system_architecture;
    std::string playtest_id;
};

struct web_configuration {
    uint32_t fps;
    uint32_t kps;
    uint32_t total_seconds;
    std::string subtitle;
    bool enable_microprofile;
    bool enable_crashreporter;
    bool store_dxdiag;
    bool store_crash_video;
    bool enable_cpu_profiling;
    bool enable_gpu_profiling;
    bool enable_memory_profiling;
    bool enable_battery_profiling;
};

struct game_symbol {
    std::string module_id;
    uint64_t relative_address;
    std::string symbol_name;
};

struct game_variable {
    uint64_t relative_address;
    uint64_t size;
    std::string variable_name;
};

struct session_created_info {
    web_configuration configurations;
    std::vector<game_symbol> game_symbols;
    std::vector<game_variable> game_variables;
    uint32_t pdb_age;
    std::string pdb_guid;
    std::string public_module;
    std::string session_id;
};

struct function_data_point {
    uint64_t tick_start;
    std::string function_name;
    double value;
};

struct variable_data_point {
    std::string value;
    std::string variable_name;
};

struct frame_data_point {
    uint8_t battery;
    float cpu;
    float frametime;
    float gpu;
    uint64_t memory;
    std::vector<variable_data_point> variables;
};

struct instrumentation_data {
    std::string session_id;
    std::string timestamp_start;
    uint64_t tick_start;
    std::vector<function_data_point> functions;
    std::vector<frame_data_point> frames;
};

struct release_json_file_data {
    std::string type;
    std::string filename;
    std::string url;
    std::string sha512;
    std::string size;
};

struct release_json_version_data {
    std::string version;
    std::string releaseNotesURL;
    std::string date;
    std::vector<release_json_file_data> files;
};

struct release_json_version {
    release_json_version_data latest;
    std::vector<release_json_version_data> other;
};

struct release_json {
    release_json_version sdk;
    release_json_version cli;
};

struct integration_test_result {
    std::string test_id;
    std::string map;
    int total_test;
    int failed;
    int success;
    std::string result;
};

struct playtest_build {
    std::string id;
    std::string pre_script;
    std::string post_script;
};

struct playtest_data {
    std::string id;
    std::string game_id;
    std::string name;
    std::string description;
    bool started{};
    std::string started_at;
    std::string stopped_at;
    std::string created_at;
    std::string updated_at;
    std::string deleted_at;
    std::vector<playtest_build> builds;
};

struct playtest_paging {
    std::string first;
    std::string last;
    std::string next;
    std::string previous;
};

struct get_list_playtest_response {
    std::vector<playtest_data> data;
    playtest_paging paging;
    uint64_t total_data{};
};

} // namespace types
} // namespace blackbox
