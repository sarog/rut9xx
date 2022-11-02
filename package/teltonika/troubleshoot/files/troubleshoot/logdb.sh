#!/bin/sh

logdb_hook() {
	[ -f "/storage/log.db" ] && cp "/storage/log.db" "${PACK_DIR}log.db" && return 0

	[ -f "/log/log.db" ] && cp "/log/log.db" "${PACK_DIR}log.db"
}

troubleshoot_hook_init logdb_hook
