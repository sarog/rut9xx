#!/bin/sh

troubleshoot_hook_init() {
	local hook="${1}_hook"

	export -n "H_STACK_LIST=${H_STACK_LIST:+$H_STACK_LIST }$hook"
	export -n "$hook=$1"
}

troubleshoot_run_hook() {
	local hook="${1}_hook"
	local func
	eval "func=\$$hook"
	local ran
	eval "ran=\$H_RAN_$hook"

	[ -n "$ran" ] || {
		export -n "H_RAN_$hook=1"
		shift
		$func "$@"
	}
}

troubleshoot_run_hook_all() {
	local hook name

	for hook in $H_STACK_LIST; do
		eval "name=\$$hook"
		troubleshoot_run_hook "$name"
	done
}

troubleshoot_init_log() {
	local title="$1" log_file="$2"

	echo -e "\n[${title}]" >>"$log_file"
}

troubleshoot_add_log() {
	local data="$1" log_file="$2"

	echo -e "$data" >>"$log_file"
}

troubleshoot_add_file_log() {
	local file="$1" log_file="$2"

	cat "$file" >>"$log_file" 2>/dev/null
}

troubleshoot_add_log_ext() {
	local cmd="$1" param="$2" log_file="$3"

	[ -n "$cmd" ] && [ -n "$(which "$cmd")" ] && {
		troubleshoot_add_log "$(eval "$cmd" $param)" "$log_file"
	}
}
