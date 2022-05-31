// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_BLACKBOX_H
#define ACCELBYTE_BLACKBOX_H

#include "accelbyte/blackbox_common.h"
#include "accelbyte/cpp/blackbox_types_cpp.h"
#include "accelbyte/cpp/utils/error_codes.h"
#include "accelbyte/cpp/utils/graphics_api_list.h"
#include "accelbyte/cpp/utils/log_severity.h"
#include "accelbyte/cpp/blackbox_http.h"

#include <cstdint>
#include <functional>
#if BLACKBOX_LINUX_PLATFORM
#    include <ucontext.h>
#    include <csignal>
#endif

namespace blackbox {

using session_callback_t = void (*)(const callback_http_response&, const char*);
using info_gather_callback_t = void (*)();

struct ucontext_buffer {
    // Size of ucontext is 936 bytes in unreal's clang toolchain, this is based on old Glibc.
    char value[936];
};

// Main Functions

/**
 * @brief Initiate BlackBox SDK
 *
 * @param api The currently used Graphics API
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code init_module(graphics_api api = graphics_api::DX11);

/**
 * @brief Shutdown BlackBox SDK
 *
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code shutdown_module();

// Main Functions - Sessions

/**
 * @brief Starting new BlackBox session
 *
 * This command will in effect start the crash video recorder and profiling
 * enabled components are customizeable from the website
 *
 * @param callback Callback on completion
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code start_new_session(session_callback_t callback);

/**
 * @brief Same as start new session but used inside the editor
 *
 * @param callback Callback on completion
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code start_new_session_on_editor(session_callback_t callback);

/**
 * @brief Get the session id or empty if the session haven't been started
 *
 * @return const char* reference to the session id string
 */
BLACKBOX_API const char* get_session_id();

/**
 * @brief Set the http proxy to use
 *
 * @param addr proxy address
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code set_http_proxy(const char* addr);

/**
 * @brief Tick the SDK, executing the callback
 *
 * @param dt delta time between calls
 */
BLACKBOX_API void tick(float dt);

/**
 * @brief Tick the SDK using engine tick (not game tick), executing the callback
 *
 * @param dt delta time between calls
 */
BLACKBOX_API void engine_tick(float dt);

/**
 * @brief Start gather hardware and system info
 *
 * @param callback Callback on gather information complete
 */
BLACKBOX_API void start_gather_device_info(info_gather_callback_t callback);

/**
 * @brief Dump gathered hardware and system info to a file
 *
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code dump_device_info_to_file(const char* file_path);

/**
 * @brief Clear all pending callbacks
 *
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code clear_pending_tasks();

// Main Functions - Key Input

/**
 * @brief Set the up key input from the game engine
 *
 * @param action_names names of key actions
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code setup_key_input(const char** action_names, size_t length);

/**
 * @brief Update the state of registered keys
 *
 * @param key_idx index of registered key
 * @param pressed state of the key
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code update_key_input(uint8_t key_idx, bool pressed);

// Main Functions - Additional Info

/**
 * @brief Additional info from the game to be shown in the website
 *
 * Currently the only supported data type is string, this function will add
 * if the field is non yet exist and update if it's already exist.
 *
 * @param field_name The info name
 * @param value The info value
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code update_additional_info(const char* field_name, const char* value);

/**
 * @brief Delete a field in Additional info list
 *
 * @param field_name The info name
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code delete_additional_info_field(const char* field_name);

/**
 * @brief Get the additional info value object
 *
 * @param field_name The info name
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API const char* get_additional_info_value(const char* field_name);

/**
 * @brief Empties the additional info list
 *
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code empty_additional_info();

// Main Functions - Recorder
#if BLACKBOX_WINDOWS_PLATFORM
/**
 * @brief Update the blackbox video recorder texture
 *
 * This function initiate the internal texture buffer if it's not yet exist, effectively starting
 * the recording
 *
 * @param backbuffer_tex Backbuffer texture from the game
 * @return error::code Describe any errors encountered
 */
template<typename TDXResource>
BLACKBOX_API error::code update_backbuffer_texture(TDXResource backbuffer_tex);

#else
/**
 * @brief Update the blackbox video recorder texture
 *
 * This function initiate the internal texture buffer if it's not yet exist, effectively starting
 * the recording
 *
 * @param backbuffer_tex Backbuffer texture from the game
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code update_backbuffer_texture(void* backbuffer_tex);
#endif

/**
 * @brief Stop video recording
 *
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code stop_recording();

// Main Functions - Profiler

/**
 * @brief Start the profiling module
 *
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code start_profiling();

/**
 * @brief Stop the profiling module
 *
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code stop_profiling();

/**
 * @brief Suspend the shared mem usage
 *
 * Effectively will shutdown the helper on windows
 *
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code suspend_shared_mem();

/**
 * @brief Continue the shared mem usage
 *
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code continue_shared_mem();

/**
 * @brief Register that the game has crashed
 *
 * @return error::code 
 */
BLACKBOX_API error::code blackbox_set_game_has_crashed();

/**
 * @brief Feed the data from game engine to send as basic profiling
 *
 * @param dt frame time
 * @param gpu_render_time GPU render time
 * @param game_render_time Game thread render time
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code update_profiling_basic_data(float dt, float gpu_render_time, float game_thread_time);

/**
 * @brief Store dynamic profiling data from debug symbol file
 *
 * @param module_name the name of the main module in symbol file
 * @param symbols symbols to profile
 * @param variables variables to profile
 * @return error::code Describe any errors encountered
 */

BLACKBOX_API error::code store_profiling_data(
    const char* module_name,
    types::game_symbol* symbols,
    size_t symbols_len,
    types::game_variable* variables,
    size_t variables_len);

// Logs

/**
 * @brief Set the log severity for log callback
 *
 * Please check log_severity.h to see available severity levels
 *
 * @param enabled_sev severity levels to enable
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code set_log_severity(uint8_t enabled_sev);

/**
 * @brief Set the log callback
 *
 * @param callback function to call when there's a log available
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code set_log_callback(void (*callback)(log::severity, const char*));

/**
 * @brief Set the file compression function object
 *
 * @param tgt_fn function pointer to compression function
 */
BLACKBOX_API void set_file_compression_function(bool (*tgt_fn)(void*, signed int&, const void*, signed int));

/**
 * @brief Compress data using zlib compression scheme
 *
 * @param compressed_data buffer to compressed data, this is the output of this function
 * @param compressed_size size of the compressed data, this value must be set with buffer size when calling this
 * function
 * @param uncompressed_data buffer to uncompressed data, this is the input of this function
 * @param compressed_size size of the uncompressed data
 */
BLACKBOX_API error::code compress_data(
    void* compressed_data, int32_t& compressed_size, const void* uncompressed_data, int32_t uncompressed_size);

// Configs

/**
 * @brief get base url
 *
 * @return const char* base url
 */
BLACKBOX_API const char* config_get_base_url();

/**
 * @brief get Identity and Access Management (IAM) url
 *
 * @return const char* IAM url
 */
BLACKBOX_API const char* config_get_iam_url();

/**
 * @brief get SDK download url
 *
 * @return std::string SDK download url
 */
BLACKBOX_API const char* config_get_sdk_download_url();

/**
 * @brief get releason json url
 *
 * @return std::string release json url
 */
BLACKBOX_API const char* config_get_release_json_url();

/**
 * @brief get game version id
 *
 * @return const char* game version id
 */
BLACKBOX_API const char* config_get_game_version_id();

/**
 * @brief get build id
 *
 * @return const char* build id
 */
BLACKBOX_API const char* config_get_build_id();

/**
 * @brief get namespace
 *
 * @return const char* namespace
 */
BLACKBOX_API const char* config_get_namespace();

/**
 * @brief get project id
 *
 * @return const char* project id
 */
BLACKBOX_API const char* config_get_project_id();

/**
 * @brief get API key
 *
 * @return const char* API key
 */
BLACKBOX_API const char* config_get_api_key();

/**
 * @brief get config path
 *
 * @return const char* config path
 */
BLACKBOX_API const char* config_get_config_path();

/**
 * @brief get webconfig fps
 *
 * @return uint32_t fps value
 */
BLACKBOX_API uint32_t config_get_fps();

/**
 * @brief get webconfig kps
 *
 * @return uint32_t kps value
 */
BLACKBOX_API uint32_t config_get_kps();

/**
 * @brief get webconfig recording second
 *
 * @return uint32_t recording second value
 */
BLACKBOX_API uint32_t config_get_total_second();

/**
 * @brief get webconfig substitle type
 *
 * @return const char* subtitle type value
 */
BLACKBOX_API const char* config_get_subtitle_type();

/**
 * @brief get webconfig enable crash reporter
 *
 * @return bool enable crash reporter value
 */
BLACKBOX_API bool config_get_enable_crash_reporter();

/**
 * @brief get webconfig store dxdiag
 *
 * @return bool is store dxdiag value
 */
BLACKBOX_API bool config_get_store_dxdiag();

/**
 * @brief get webconfig store crash video
 *
 * @return bool store crash video value
 */
BLACKBOX_API bool config_get_store_crash_video();

/**
 * @brief get webconfig enable basic profiling
 *
 * @return bool basic profiling value
 */
BLACKBOX_API bool config_get_enable_basic_profiling();

/**
 * @brief get webconfig enable cpu profiling
 *
 * @return bool cpu profiling value
 */
BLACKBOX_API bool config_get_enable_cpu_profiling();

/**
 * @brief get webconfig enable gpu profiling
 *
 * @return bool gpu profiling value
 */
BLACKBOX_API bool config_get_enable_gpu_profiling();

/**
 * @brief get webconfig enable memory profiling
 *
 * @return bool memory profiling value
 */
BLACKBOX_API bool config_get_enable_memory_profiling();

/**
 * @brief get crash folder
 *
 * @return const char* the crash folder path
 */
BLACKBOX_API const char* config_get_crash_folder();

/**
 * @brief get crash GUID
 *
 * @return const char* the crash GUID
 */
BLACKBOX_API const char* config_get_crash_guid();

/**
 * @brief get hardware information gathering switch value
 *
 * @return bool the switch value
 */
BLACKBOX_API bool config_get_enable_hardware_info_gathering();

/**
 * @brief set and store base url
 *
 * @param base_url new  blackbox base url
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_base_url(const char* base_url);

/**
 * @brief set IAM url
 *
 * @param iam_url new blackbox IAM url
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_iam_url(const char* iam_url);

/**
 * @brief set your game version id
 *
 * @param game_version_id new game version id
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_game_version_id(const char* game_version_id);

#if BLACKBOX_PS4_PLATFORM || BLACKBOX_PS5_PLATFORM
/**
 * @brief set your platform SDK version
 *
 * @param platform_sdk_version new platform SDK version
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_platform_sdk_version(const char* platform_sdk_version);
#endif

/**
 * @brief set latest sdk download url
 *
 * @param download_url new sdk download url
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_sdk_download_url(const char* download_url);

/**
 * @brief set latest release json url
 *
 * @param release_json_url new release json url
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_release_json_url(const char* release_json_url);

/**
 * @brief set your game build id
 *
 * @param build_id new build id
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_build_id(const char* build_id);

/**
 * @brief set your game namespace
 *
 * @param namespace_ new namespace
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_namespace(const char* namespace_);

/**
 * @brief set your game project id
 *
 * @param project_id new project id
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_project_id(const char* project_id);

/**
 * @brief set API key
 *
 * @param api_key new API key
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_api_key(const char* api_key);

/**
 * @brief set game client pid
 *
 * @param pid new pid
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_pid(uint32_t pid);

/**
 * @brief set crash folder
 *
 * The location of crash folder is dependent on the game engine and
 * so this information needs to be fed from the engine
 *
 * @param crash_folder new crash folder
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_crash_folder(const char* crash_folder);

/**
 * @brief set is using editor
 *
 * The flag is the game currently using editor or package build
 *
 * @param is_using_editor is with editor flag
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_is_using_editor(const bool is_using_editor);
/**
 * @brief set blackbox helper path
 *
 * This function tells the SDK where to look for the helper
 * @param path new helper path
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_blackbox_helper_path(const char* path);

/**
 * @brief set blackbox issue reporter path
 *
 * This function tells the SDK where to look for issue reporter
 * @param path new issue reporter path
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_blackbox_issue_reporter_path(const char* path);

/**
 * @brief set crash guid
 *
 * @param crash_guid
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_crash_guid(const char* crash_guid);

/**
 * @brief set blackbox log source path
 *
 * This function tells the SDK where to look for the log source file
 * @param path new log source path
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_log_source_file_path(const char* path);

/**
 * @brief set rendering GPU name
 *
 * On dual GPU setup, the rendering GPU isn't always the primary GPU
 * This function is provided so that the game engine can tell the SDK which GPU is being used
 *
 * @param gpu_name new rendering GPU name
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_rendering_gpu_name(const char* gpu_name);

/**
 * @brief set gpu driver ver
 *
 * @param ver_str new driver version
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_gpu_ver(const char* ver_str);

/**
 * @brief set default config file location
 *
 * @param path file location
 * @return error::code Describe any errors encountered
 */
BLACKBOX_API error::code config_set_config_path(const char* path);

/**
 * @brief set engine version associated with crash handler
 *
 * @param ver_str engine version associated
 * @return error::code any errors encountered
 */
BLACKBOX_API error::code config_set_engine_version(const char* ver_str);

/**
 * @brief set the currently gpu device id so that the SDK will use the same id with the engine
 *
 * @param id the gpu device id currently used
 * @return error::code any errors encountered
 */
BLACKBOX_API error::code config_set_gpu_device_id(uint32_t id);

/**
 * @brief set engine type associated with crash handler
 *
 * @param engine_type engine type associated [UE4,UE5]
 * @return error::code any errors encountered
 */
BLACKBOX_API error::code config_set_engine(const char* engine_type);

/**
 * @brief set engine major version associated with crash handler
 *
 * @param major_version engine major version associated
 * @return error::code any errors encountered
 */
BLACKBOX_API error::code config_set_engine_major_version(uint32_t major_version);

/**
 * @brief set engine minor version associated with crash handler
 *
 * @param minor_version engine minor version associated
 * @return error::code any errors encountered
 */
BLACKBOX_API error::code config_set_engine_minor_version(uint32_t minor_version);

/**
 * @brief set engine patch version associated with crash handler
 *
 * @param patch_version engine patch version associated
 * @return error::code any errors encountered
 */
BLACKBOX_API error::code config_set_engine_patch_version(uint32_t patch_version);

/**
 * @brief enable or disable client's machice hardware information gathering
 *
 * @param enable the boolean switch to enable or disable information gathering
 * @return error::code any errors encountered
 */
BLACKBOX_API error::code config_set_enable_hardware_info_gathering(bool enable);

/**
 * @brief import initial config from .ini file
 *
 * @param path initial config file location
 * @return error::code any errors encountered
 */
BLACKBOX_API error::code import_default_config(const char* path);

// Informations

/**
 * @brief Get host OS architecture
 *
 * @return const char* architecture string
 */
BLACKBOX_API const char* info_get_os_architecture();

/**
 * @brief Get host OS name
 *
 * @return const char* OS name string
 */
BLACKBOX_API const char* info_get_os_name();

/**
 * @brief Get host OS version
 *
 * @return const char* OS version string
 */
BLACKBOX_API const char* info_get_os_version();

/**
 * @brief Get logged in host machine username
 *
 * @return const char* username string
 */
BLACKBOX_API const char* info_get_host_user_name();

/**
 * @brief Get logged in host machine name
 *
 * @return const char* machine name string
 */
BLACKBOX_API const char* info_get_computer_name();


/**
 * @brief Get SDK version
 *
 * @return const char* SDK version string
 */
BLACKBOX_API const char* info_get_version();

/**
 * @brief NEW! Issue reporter API
 */
BLACKBOX_API void capture_screenshot(const char* parent_path);

// Crash Handling
#if BLACKBOX_WINDOWS_PLATFORM
/**
 * @brief Handle the client application crash for windows platform
 */
BLACKBOX_API void handle_crash();

#elif BLACKBOX_XBOX_ONE_PLATFORM
BLACKBOX_API void handle_crash(
    const wchar_t* minidump_path,
    const wchar_t* log_path,
    void* runtime_xml_data,
    size_t runtime_xml_size,
    char* stacktrace,
    size_t stacktrace_size);

BLACKBOX_API void send_crash(const char* crashbin_dir, std::function<void(bool)> cb);

#elif BLACKBOX_XBOXGDK_PLATFORM
BLACKBOX_API void save_crash_video();
BLACKBOX_API void set_gamertag(const char* gamertag);
BLACKBOX_API void handle_crash(
    const wchar_t* minidump_path,
    const wchar_t* log_path,
    void* runtime_xml_data,
    size_t runtime_xml_size,
    char* stacktrace,
    size_t stacktrace_size);

BLACKBOX_API void send_crash(const char* crashbin_dir, std::function<void(bool)> cb, bool should_wait = false);

#elif BLACKBOX_PS4_PLATFORM || BLACKBOX_PS5_PLATFORM
BLACKBOX_API void init_crash_handler();

#elif BLACKBOX_LINUX_PLATFORM
BLACKBOX_API bool handle_crash(int signal, siginfo_t* info, ucontext_buffer* buff);

BLACKBOX_API void init_crash_handler(const char* crash_dir);

#elif BLACKBOX_MAC_PLATFORM

BLACKBOX_API void handle_crash();
BLACKBOX_API void init_crash_handler(const char* crash_dir);

#endif

/**
 * @brief Validate config for APIKey, game version ID, and name space.
 *
 * @return bool Return true if validation success
 */
BLACKBOX_API bool validate_config();

/**
 * @brief Notify sdk that the game changes the resolution
 */

BLACKBOX_API void notify_change_game_resolution();

/**
 * @brief Create blackbox temp directory.
 *
 * @return const char* Created directory path
 */
BLACKBOX_API const char* generate_temp_dir();

/**
 * @brief Clean generated temp directory
 */
BLACKBOX_API void remove_temp_dir();

} // namespace blackbox

#endif // ACCELBYTE_BLACKBOX_H
