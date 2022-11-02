# 1: destination variable
# 2: interface
# 3: path
# 4: separator
# 5: limit
__wireless_ifstatus() {
	local __tmp

	[ -z "$__WIRELESS_CACHE" ] && {
		__tmp="$(ubus call network.wireless status 2>&1)"
		case "$?" in
			4) : ;;
			0) export __WIRELESS_CACHE="$__tmp" ;;
			*) echo "$__tmp" >&2 ;;
		esac
	}

	__tmp="$(jsonfilter ${4:+-F "$4"} ${5:+-l "$5"} -s "${__WIRELESS_CACHE:-{}}" -e "$1=@.*.interfaces[@.config.wifi_id='$2']$3")"

	[ -z "$__tmp" ] && \
		unset "$1" && \
		return 1

	eval "$__tmp"
}

# 1: destination variable
# 2: interface name
wireless_get_ifname() { __wireless_ifstatus "$1" "$2" ".ifname"; }

# flush the internal value cache to force re-reading values from ubus
wireless_flush_cache() { unset __WIRELESS_CACHE; }