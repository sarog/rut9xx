#!/bin/sh

# this is an rpcd plugin to expose
# the functionality of /usr/sbin/profile.sh
# as an ubus object.

. /usr/share/libubox/jshn.sh
. /lib/functions.sh


# used to fix trailing comma issue
# when listing profiles.
LIST_COUNT=0

# used when listing profiles
handle_profile () {
    config="$1"
    current="$2"

    config_get updated "$config" updated

    [ $LIST_COUNT -gt 0 ] && {
        echo ","
    }

    echo "{"
    echo "\"name\": \"$config\","
    echo "\"updated\": $updated,"
    
    if [ "$config" = "$current" ]; then
        echo "\"active\": 1"
    else
        echo "\"active\": 0"
    fi
    echo "}"

    LIST_COUNT=$((LIST_COUNT+1))
}

# used when looping over /etc/config/profiles
# to check if a profile already exists when creating
# exits out of the script upon condition met
check_profile_exists () {
    config="$1"
    new_name="$2"

    [ "$config" = "$new_name" ] && {
        echo "{\"status\": 1, \"error\": \"profile '$new_name' already exists\"}"
        exit 1
    }
}

# validate profile name
check_name() {
    name="$1"
    length=$(echo -n "$name" | wc -m)

    [ $length -lt 1 ] && {
        echo '{"status": 22, "error": "no argument provided"}'
        exit 1
    }

    # check name len
    # limit set to be the same as-is in the webUI.
    [ $length -gt 20 ] && {
        echo '{"status": 22, "error": "given profile name too long (limit: 20 characters)"}'
        exit 1
    }

    # Sanitize input
    sanitized=$(echo $name | sed 's/ /_/g; s/[^a-zA-Z0-9_]//g')

    # Breaks if name is invalid.
    # better to inform the user of their mistake, rather
    # than contiuing on after making changes to user input,
    # since this ubus object will likely only be called from
    # other scripts/programs
    [ "$sanitized" != "$name" ] && {
        echo '{"status": 22, "error": "invalid profile name"}'
        exit 1
    }
}

# Checks if a name matches the reserved names in /etc/profiles
check_default() {
    name="$1"

    # Check if names won't interfere with the default profile files in /etc/profiles
    [ "$name" = "default" ] || [ "$name" = "template" ] && {
        echo '{"status": 22, "error": "profile name cannot be '\''template'\'' or '\''default'\''"}'
        exit 1
    }
}

# Basic pseudo-random number generator. Do not use for critical applications.
basic_rand() {
    local r1="$(cat /proc/uptime | sed 's/\.//g' | cut -d ' ' -f 1 | tail -c4)"
    local r2="$(cat /proc/uptime | sed 's/\.//g' | cut -d ' ' -f 2 | tail -c4)"

    echo "${r1}${r2}"
}

# Handles the `create` call
call_handle_create() {
    profiles="$1"
    current="$2"
    now="$3"

    read input
    json_load "$input"
    json_get_var name name

    check_name "$name"
    check_default "$name"

    config_load profiles

    # breaks if profile w same name found
    config_foreach check_profile_exists profile "$name"

    # create new config in uci
    uci_add profiles profile "$name"
    uci_set profiles "$name" id "$(basic_rand)"
    uci_set profiles "$name" updated $now
    uci_set profiles "$name" archive "${name}_${now}.tar.gz"
    uci_set profiles "$name" md5file "${name}_${now}.md5" 

    uci_commit profiles

    profile.sh -b "${profiles}${name}_${now}.tar.gz"
    profile.sh -m "${profiles}${name}_${now}.md5"

    uci commit

    echo '{ "status": 0 }'
}

# Handles the `change` call
call_handle_change() {
    read input
    json_load "$input"
    json_get_var name name

    check_name "$name"

    archive="$(uci_get profiles $name archive)"

    if [ -n "$archive" ]; then
        profile.sh -c "$name" &>/dev/null
        echo '{ "status": 0 }'
    else
        echo '{ "status": 2, "error": "profile not found"}'
        exit 1
    fi
}

# Handles the `update` call
call_handle_update() {
    profiles="$1"
    current="$2"
    now="$3"

    # Check if names won't interfere with the default profile files in /etc/profiles
    # kept different `check_default` above
    # because this has no check for 'template' and a different err msg
    [ "$current" = "default" ] && {
        echo '{"status": 95, "error": "update not available for '\''default'\'' profile"}'
        exit 1
    }

    rm -f "${profiles}${current}"*

    config_load profiles
    uci_set profiles "$current" updated $now
    uci_set profiles "$current" archive "${current}_${now}.tar.gz"
    uci_set profiles "$current" md5file "${current}_${now}.md5"
    uci_commit profiles

    profile.sh -b "${profiles}${current}_${now}.tar.gz"
    profile.sh -m "${profiles}${current}_${now}.md5"
    uci commit

    echo '{ "status": 0 }'
}

# Handles the `remove` call
call_handle_remove() {
    profiles="$1"
    current="$2"

    read input
    json_load "$input"
    json_get_var name name

    config_load profiles

    check_name "$name"
    check_default "$name"

    err_chk=0

    # file existance null-checks.
    if [ -f "$profiles$(uci_get profiles $name archive \".\")" ]; then
        rm "$profiles$(uci_get profiles $name archive \".\")"
    else
        err_chk=1
    fi

    if [ -f "$profiles$(uci_get profiles $name md5file \".\")" ]; then
        rm "$profiles$(uci_get profiles $name md5file \".\")"
    else
        err_chk=1
    fi

    [ $err_chk -eq 1 ] && {
        echo '{"status": 5, "error": "encountered errors while removing profile"}'
        exit 1
    }

    uci_remove profiles "$name"

    # profile currently in use is being removed,
    # reset to default.
    [ "$name" = "$current" ] && {
        profile.sh -c "default" &>/dev/null
    }

    uci_commit profiles
    uci commit

    echo '{ "status": 0 }'
}

# Handles the `list` call
call_handle_list() {
    current="$1"
    LIST_COUNT=0

    echo '{ "profiles": ['
    config_load profiles
    config_foreach handle_profile profile "$current"
    echo ']}'

    #unset LIST_COUNT # keep the env vars clean
}

# Handles the `diff` call
call_handle_diff() {
    profiles="$1"

    read input
    json_load "$input"
    json_get_var name name

    check_name "$name"

    # edgecase for comparing w default
    [ "$name" = "default" ] && {
        echo '{ "status": 0, "diff": ['
        profile.sh -d "${profiles}${name}"*.md5 | sed 's/^/\"/; s/$/\",/'
        echo ']}'
        exit
    }

    if [ -f "$profiles$(uci_get profiles $name md5file \".\")" ]; then
        echo '{ "status": 0, "diff": ['
        profile.sh -d "${profiles}${name}"*.md5 | sed 's/^/\"/; s/$/\",/'
        echo ']}'
    else
        echo '{"status": 2, "error": "could not find profile"}'
        exit 1
    fi
}


main () {

    case "$1" in
        list)
            echo '{
                "create":{"name":"String"},
                "change":{"name":"String"},
                "remove":{"name":"String"},
                "diff":{"name":"String"},
                "update": {},
                "list": {},
            }'

        ;;
        call)
            profiles="$(uci_get profiles general path /etc/profiles)"
            current="$(uci_get profiles general profile default)"
            now="$(date +%s)" # unix time

            case "$2" in
                create)
                    call_handle_create "$profiles" "$current" "$now"
                ;;
                change)
                    call_handle_change
                ;;
                update)
                    call_handle_update "$profiles" "$current" "$now"
                ;;
                remove)
                    call_handle_remove "$profiles" "$current"
                ;;
                list)
                    call_handle_list "$current"
                ;;
                diff)
                    call_handle_diff "$profiles"
                ;;
            esac
            json_cleanup
        ;;
    esac
}

main "$@"
