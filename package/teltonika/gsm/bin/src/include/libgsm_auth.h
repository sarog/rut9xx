#pragma once

typedef enum {
    GSM_AUTH_SUCCESS,
    GSM_AUTH_FAIL
} gsm_auth_stat;

int check_number_in_group(char *senderNr, struct uci_section *section);
int lookup_group_in_config(char *senderNr, const char *group_name);
int check_phone_authorization(struct uci_context *uci, struct uci_section *section, char *senderNr);

char *get_system_param(char *rule_path, int cut_new_line);
int authorized_by_serial(char *sms_text);

/**
 * @brief parse password from SMS text and make shadow authorization
 * 
 * @param sms_text SMS text
 * @retval GSM_AUTH_SUCCESS on success
 * @retval GSM_AUTH_FAIL on fail
 * @return gsm_auth_stat 
 */
gsm_auth_stat authorized_by_password(const char *sms_text);
