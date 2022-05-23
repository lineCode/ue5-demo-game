// Copyright (c) 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_BLACKBOX_H
#define ACCELBYTE_BLACKBOX_H

#include "accelbyte/blackbox_common.h"
#include "blackbox_error_codes.h"
#include "blackbox_log.h"
#include "blackbox_types.h"

#include <stdint.h>

blackbox_return blackbox_init_module();
blackbox_return blackbox_shutdown_module();
blackbox_return
blackbox_start_new_session(blackbox_type_client_desc* client_desc, void (*callback)(unsigned int, int, const char*));
blackbox_return blackbox_start_new_session_on_editor(void (*callback)(blackbox_type_client_desc_editor*));
blackbox_return blackbox_get_session_id();
void blackbox_tick(float dt);
blackbox_return blackbox_setup_keyboard_input();
blackbox_return blackbox_update_keyboard_input();
blackbox_return blackbox_update_additional_info(const char* field_name, const char* value);
blackbox_return blackbox_clear_additional_info_field(const char* field_name);
blackbox_return blackbox_reset_additional_info();
void blackbox_set_log_severity(blackbox_log_severity enabled_sev);
void blackbox_set_log_callback(void (*fn)(blackbox_log_severity, const char*));
blackbox_return blackbox_update_backbuffer_texture();

#endif // ACCELBYTE_BLACKBOX_H
