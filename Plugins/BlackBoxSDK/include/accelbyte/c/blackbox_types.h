// Copyright (c) 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_BLACKBOX_TYPES_H
#define ACCELBYTE_BLACKBOX_TYPES_H
#include <stdint.h>

typedef struct blackbox_http_header {
    const char* field_name;
    const char* value;
    blackbox_http_header* next;
} blackbox_http_header_t;

typedef struct blackbox_http_response {
    uint32_t resp_code;
    int socket_err_code;
    const char* resp_url;
    blackbox_http_header_t* resp_headers;
    const char* resp_body_str;
} blackbox_http_response_t;

typedef struct blackbox_type_client_desc {
    const char* build_id;
    const char* country;
    const char* cpu;
    const char* game_version_id;
    const char* gpu;
    const char* gpu_driver;
    const char* locale;
    const char* memory;
    const char* operating_system;
    const char* operating_version;
    const char* renderer;
    const char* session_started_at;
    const char* system_architecture;
} blackbox_type_client_desc_t;

typedef struct blackbox_create_new_session_response {
    blackbox_http_response_t* http_response;
    blackbox_type_client_desc_t* client_desc;
} blackbox_create_new_session_response_t;

typedef struct blackbox_type_client_desc_editor {
    const char* country;
    const char* cpu;
    const char* game_version_id;
    const char* gpu;
    const char* gpu_driver;
    const char* locale;
    const char* memory;
    const char* operating_system;
    const char* operating_version;
    const char* renderer;
    const char* session_started_at;
    const char* system_architecture;
} blackbox_type_client_desc_editor_t;

typedef struct blackbox_create_new_session_on_editor_response {
    blackbox_http_response_t* http_response;
    blackbox_type_client_desc_editor_t* client_desc;
} blackbox_create_new_session_on_editor_response;

#endif // !ACCELBYTE_TYPES_H
