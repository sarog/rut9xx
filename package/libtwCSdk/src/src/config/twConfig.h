/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twConfig.h
 * \brief General configuration overrides for ThingWorx C SDK
 *
 * Add any overrides to the CommonSettings file in this file.
 * Note that the settings here will apply to ALL of your projects
 * that use the SDK, so it is recommended that application specific
 * settings use the CommonSettings file to change configuration.
 * Configuration affects the code footprint and RAM usage of the SDK 
 * and consequently any application built using the SDK.
 * 
 * This file only needs to be used if your are NOT building with either
 * one of the provided Windows Solution or gcc Makefiles which allow you
 * to do per project configuration in the respective CommonSettings.targets
 * or example application Makefiles respectively.  If you are using one 
 * of the above mentioned files, it is possible to override those settings
 * in this file using preprocessor #defines.
 *
 * Current Settings and their defaults are:<br>
 * ENABLE_TASKER = 1   -> Built in tasker is enabled<br>
 * ENABLE_FILE_XFER = 1  -> File transfer is enabled<br>
 * ENABLE_TUNNELING = 1 - > Tunneling is enabled<br>
 * OFFLINE_MSG_STORE = 2  -> Offline message store is enabled and set to persist to a file<br>
 * ENABLE_HTTP_PROXY_SUPPORT = 1 -> Enable/Disable support for connecting through HTTP Proxies. Default is enabled.<br>
 * USE_NTLM_PROXY = 1 -> Enable/Disable support for NTLM Authenitcating HTTP Proxies. Default is enabled.<br>
 * TW_LEAN_AND_MEAN - > disabled.  Used to Minimize memory footprint (disabling functionality)<br>
 *
 * Refer to twDefaultSettings.h for details on these settings.
*/

#ifndef TW_CONFIG_H
#define TW_CONFIG_H

/* Configuration settings go here. */

#endif
