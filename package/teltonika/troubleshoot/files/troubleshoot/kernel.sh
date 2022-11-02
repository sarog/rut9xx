#!/bin/sh

kernel_hook() {
	local log_file="${PACK_DIR}kernel.log"

	troubleshoot_init_log "Kernel modules" "$log_file"
	troubleshoot_add_log "$(lsmod)" "$log_file"

	troubleshoot_init_log "Device files" "$log_file"
	troubleshoot_add_log "$(ls -al /dev/)" "$log_file"
}

troubleshoot_hook_init kernel_hook
