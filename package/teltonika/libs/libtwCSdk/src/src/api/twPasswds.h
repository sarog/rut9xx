/*
 *  Copyright 2018, PTC, Inc.
 */

#ifndef TW_C_SDK_TWPASSWDS_H
#define TW_C_SDK_TWPASSWDS_H
#define TW_MAX_PASSWORD_LENGTH 256
typedef void (*twPasswdCallbackFunction)(char * passwdBuffer, unsigned int maxPasswdSize);
/**
 * When given a password callback function, twPasswdCallbackFunction, resolves it and returns the
 * provided app_key. The returned key is owned by the caller and must be deleted by calling twFreePasswd()
 * to ensure the password's removal from memory as soon as possible.
 * @param callback A pointer to a function of the type twPasswdCallbackFunction
 */
char* twConvertCallbackToPasswd(twPasswdCallbackFunction callback);
/**
 * Used to delete any passwd returned by twConvertCallbackToPasswd. Ensures not only deletion but the overwriting
 * of the memory used to store the password with zeros. Call this funciton as soon as possible after retrieving
 * a password.
 * @param twPasswd
 */
void twFreePasswd(char* twPasswd);
#endif //TW_C_SDK_TWPASSWDS_H