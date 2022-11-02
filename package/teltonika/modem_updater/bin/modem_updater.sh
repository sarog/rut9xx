#!/bin/sh
. /usr/share/libubox/jshn.sh
DEBUG_LOG="1"
DEBUG_ECHO="0"
HOSTNAME="modemfota.teltonika-networks.com"
SSH_PASS="u3qQo99duKeaVWr7"
DEVICE=""
SKIP_VALIDATION="0"
FW_PATH="/tmp/firmware/"
FLASHER_PATH=""
SSHFS_PATH="/usr/bin/sshfs"
LEGACY_MODE="0"
TTY_PORT=""
modem_id=""
MEIG_FLASHER="meig_firehose"
QUECTEL_FLASHER="quectel_flash"
ASK_FLASHER="QDLoader"
JUST_LIST="0"
PRODUCT_NAME=`mnf_info -n | cut -b 1-4`
CPU_NAME=$(cat /proc/cpuinfo | grep -e model)
KERNEL_VERSION=$(uname -a | awk -F ' ' '{print $3}')
current_version=$(cat /etc/version | awk -F '.' '{print $2 "." $3}')
USER_PATH="0"
VERSION=""
ASK_MODEM="0"

validate_connection(){
    if ping -q -c 1 -W 5 8.8.8.8 >/dev/null; then
        debug "[INFO] internet connection is working"
    else
        echo "[ERROR] internet connection is not active"
        start_services_modems
        exit 1
    fi
}

parse_modems() {
    json_init
    json_load_file /etc/board.json
    FOUND_IN_BOARD=0
    
    if json_is_a modems array; then
        json_select modems
        local idx=1
        while json_is_a ${idx} object; do
            json_get_var section $idx
            json_select $idx
            json_get_var id id
            if [ "$id" = "$1" ]; then
                json_get_var num num
                json_get_var desc desc
                echo "Modem$num: $desc. ID: $id"
                FOUND_IN_BOARD=1
            fi
            
            local idx=$((idx + 1))
            json_select ".."
        done
    fi
    #echo "Parse done"
}

verify_modem_id() {
    if [ "$modem_id" = "" ]; then
        echo "[ERROR] modem_id is not set. Please use \"-i\" option."
        get_modems
        helpFunction
    fi
    
    AT_RESULT=$(gsmctl -O "$modem_id" -A AT) > /dev/null 2>&1
    case "$AT_RESULT" in
        *OK*)
            debug "[INFO] $modem_id is responding to AT commands."
        ;;
        *)
            echo "[ERROR] modem_id is wrong or modem is not responding."
            get_modems
            helpFunction
    esac
}

exec_sshfs() {
    SSHFS_RESULT=$(echo $SSH_PASS | $SSHFS_PATH -p 21 rut@$HOSTNAME:/"$1" "$2" -o password_stdin 2>&1)
    debug "[INFO] $SSHFS_RESULT"
    case "$SSHFS_RESULT" in
        *Timeout*)
            echo "[ERROR] Timeout while mounting with sshfs. Your ISP might be blocking ssh connections."
            exit 1
        ;;
    esac
}

get_compatible_fw_list() {
    validate_connection

    FW_LIST_PATH="/tmp/fwlist/"
    
    [ -f "$FW_LIST_PATH" ] || mkdir -p "$FW_LIST_PATH"
    chmod 755 "$FW_LIST_PATH"
    exec_sshfs "" "$FW_LIST_PATH"
    
    FOUND_FW=$(ls "$FW_LIST_PATH" | grep ^"$MODEM_SHORT")
    echo "Available versions:"
    echo "$FOUND_FW"
    
    umount "$FW_LIST_PATH"
}

setup_ssh() {
    if [ ! -f $SSHFS_PATH ]; then
        echo "[ERROR] SSHFS not found."
        echo "You can install SSHFS using this command: \"opkg update && opkg install sshfs\""
        echo "Or alternatively using \"-r\" option."
        exit 1
    fi
    
    SSH_PATH="/root/.ssh"
    TMP_SSH_PATH="/tmp/known_hosts"
    
    chmod 755 $SSHFS_PATH
    
    key=$(cat $SSH_PATH/known_hosts | grep -c $HOSTNAME) > /dev/null 2>&1
    if [ "$key" = "0" ]; then
        debug "[INFO] Downloading key"
        
        curl -s http://$HOSTNAME/download/$TMP_SSH_PATH \
        --output $TMP_SSH_PATH --connect-timeout 90
        [ -f $SSH_PATH ] || mkdir -p $SSH_PATH
        cat $TMP_SSH_PATH >>$SSH_PATH/known_hosts
    fi
}

mount_sshfs() {
    [ -f $FW_PATH ] || mkdir -p $FW_PATH
    
    debug "[INFO] Checking old mount"
    
    if [ -n "$(ls -A $FW_PATH)" ]; then
        echo "[ERROR] $FW_PATH Not Empty. Unmounting"
        umount "$FW_PATH"
        if [ -n "$(ls -A $FW_PATH)" ]; then
            echo "[ERROR] Unmounting fail. Exiting..."
            start_services_modems
            exit 1
        fi
    else
        debug "[INFO] Old mount: Empty"
    fi
    
    # Mounting remote partition
    exec_sshfs "$VERSION" "$FW_PATH"
}

debug() {
    if [ "$DEBUG_LOG" = "1" ]; then
        logger -t "modem_updater" "$1"
    fi
    if [ "$DEBUG_ECHO" = "1" ]; then
        echo "$1"
    fi
}

make_ram_symlinks() {
    debug "[INFO] making symlinks for all temp packages installed in /tmp/"
    
    ln -sf /tmp/usr/lib/* /usr/lib/ > /dev/null 2>&1
    ln -sf /tmp/usr/bin/* /usr/bin/ > /dev/null 2>&1
    ln -sf /tmp/usr/sbin/* /usr/sbin/ > /dev/null 2>&1
}

download_file_forced() {
    local architecture_name="$1"
    local file_path="$2"
    local file_name="$3"
    
    if [ ! -f "/tmp/$file_name" ]; then
        debug "[INFO] Downloading $file_name"
        curl -s http://$HOSTNAME/dl/"$current_version"/"$architecture_name"/"$file_name" \
        --output /tmp/"$file_name" --connect-timeout 90
    fi
    
    if grep -Fq '<head><title>404 Not Found</title></head>' /tmp/"$file_name"; then
        echo "[ERROR] forced $file_name installation failed. Package not found."
        exit 1
    else
        echo "[INFO] $file_name downloaded."
    fi
    
    if [ ! -f /"$file_path"/"$file_name" ]; then
        debug "[INFO] Creating symlink /$file_path/$file_name"
        ln -s /tmp/"$file_name" /"$file_path"/"$file_name"
    fi
}

download_file_opkg() {
    local pkg_name="$1"
    local result
    
    result=$(opkg install "$pkg_name" -d ram) > /dev/null 2>&1
    
    case "$result" in
        *Configuring*)
            debug "[INFO] $pkg_name installed with opkg"
            OPKG_RESULT=0
        ;;
        *"installed in"*)
            debug "[INFO] $pkg_name is already installed."
            OPKG_RESULT=0
        ;;
        *"Cannot install"* | *Unknown*)
            #fail
            OPKG_RESULT=1
            echo "$result"
        ;;
    esac
}

download_file_any() {
    local architecture_name="$1"
    local file_path="$2"
    local file_name="$3"
    local pkg_name="$4"
    
    #try opkg download
    download_file_opkg "$pkg_name"
    if [ $OPKG_RESULT -ne 0 ]; then
        #opkg download failed try to get the file from the web.
        download_file_forced "$architecture_name" "$file_path" "$file_name"
    fi
    
}

start_services_modems() {
    debug "[INFO] Starting services:"
    /etc/init.d/gsmd start
    /etc/init.d/ledman start
    /etc/init.d/modem_tracker start
    #in case device was in 9008 mode.
    debug "[INFO] Starting modem(s)"
    mctl -r > /dev/null 2>&1
    mctl -r -m modem2 > /dev/null 2>&1
}

check_if_tempdir_added() {
    local temp_dest="dest ram /tmp"
    
    if ! grep -Fxq "$temp_dest" /etc/opkg.conf; then
        echo "$temp_dest" >>/etc/opkg.conf
        debug "[INFO] written $temp_dest to /etc/opkg.conf"
    fi
}

helpFunction() {
    echo ""
    echo "Usage: $0 -i <modem_id> -v <version> <options>."
    echo -e "\t-h \t Print help."
    echo -e "\t-i \t <modem_id> modem ID."
    echo -e "\t-v \t <version> modem firmware version to install."
    echo -e "\t-p \t <path> Specify a custom firmware path. Remote mounting with sshfs will not be used by the script."
    echo -e "\t-f \t force upgrade start without extra validation. USE AT YOUR OWN RISK."
    echo -e "\t-g \t List availible firmware versions for your modem."
    echo -e "\t-l \t Legacy mode for quectel modems(Fastboot)."
    echo -e "\t-d \t <name> Manually set device Vendor (Quectel or MEIG)."
    echo -e "\t-t \t <ttyUSBx> ttyUSBx port(for legacy mode)."
    echo -e "\t-r \t <fwver> Install missing dependencies into tmp folder."
    echo -e "\t \t Use \"-r show\" to show available versions on the server."
    echo -e "\t-D \t debug"
    exit 1 # Exit script after printing help
}

nvresultcheck() {
    case "$NV_RESULT" in
        *"nvburs: 0"*)
            debug "[INFO] nvburs:0"
            NV_FAILED=0
        ;;
        *)
            echo "[ERROR] NVBURS failed!"
            echo "$NV_RESULT"
            NV_FAILED=1
        ;;
    esac
}

setDevice() {
    DEVICE=$(/usr/sbin/gsmctl -w ${modem_id:+-O "$modem_id"} | cut -b -7)
    if [ "$DEVICE" = "N/A" ]; then
        echo "[ERROR] Not sure what flasher to use. Exiting.."
        helpFunction
    fi
}

setFlasherPath() {
    #setdevice
    if [ "$DEVICE" = "Quectel" ]; then
        FLASHER_PATH="/usr/bin/$QUECTEL_FLASHER"
        elif [ "$DEVICE" = "QuectelASK" ]; then
        FLASHER_PATH="/usr/bin/$ASK_FLASHER"
        elif [ "$DEVICE" = "MEIG" ]; then
        FLASHER_PATH="/usr/bin/$MEIG_FLASHER"
    fi
}

backupnvr() {
    debug "[INFO] MEIG device lets backup NVRAM"
    NV_RESULT=$(gsmctl -O "$modem_id" -A AT+NVBURS=2)
    nvresultcheck
    #if no backup exists then we make one.
    if [ $NV_FAILED = 1 ]; then
        NV_RESULT=$(gsmctl -O "$modem_id" -A AT+NVBURS=0)
        nvresultcheck
        if [ $NV_FAILED = 1 ]; then
            #failed to make a backup.
            echo "[ERROR] Failed to make NV ram backup. Updating without a backup may cause you to lose your IMEI and SN. Exiting..."
            exit 1
        fi
    fi
}

get_modems() {
    echo "Modem List:"
    local get_modems_output=$(ubus call gsmd get_modems)
    
    modem_array=""
    json_init
    json_load "$get_modems_output"
    
    if json_is_a modems array; then
        json_select modems
        idd=1
        while json_is_a ${idd} object; do
            json_get_var section $idd
            json_select $idd
            json_get_var id id
            modem_array="$modem_array $id"
            
            idd=$((idd + 1))
            json_select ".."
        done
    fi
    
    for s in $modem_array ; do
        parse_modems "$s"
        if [ $FOUND_IN_BOARD = "0" ]; then
            echo "External modem. ID: $s"
        fi
    done
}

get_fw_list() {
    setDevice

    if [ "$modem_id" != "" ]; then
        verify_modem_id
    fi
 
    if [ "$DEVICE" = "Quectel" ]; then
        MODEM=$(gsmctl -y ${modem_id:+-O "$modem_id"})
        MODEM_SHORT=$(echo $MODEM | cut -b -7)
        elif [ "$DEVICE" = "MEIG" ]; then
        MODEM=$(gsmctl -y ${modem_id:+-O "$modem_id"})
        #Currently meiglink uses only 3 letter device names so this should be fine for now.
        MODEM_SHORT=$(echo $MODEM | cut -b -3)
        MODEM_SHORT="SLM$MODEM_SHORT"
    fi
    
    setup_ssh
    get_compatible_fw_list
    
    exit 1
}

file_check() {
    full_path="$1"
    found_in_path="0"
    if [ -L ${full_path} ] ; then
        if [ -e ${full_path} ] ; then
            echo "[INFO] $full_path Good link"
            found_in_path="1"
        else
            echo "[INFO] $full_path Broken link"
        fi
        elif [ -e ${full_path} ] ; then
        echo "[INFO] $full_path Not a link"
        found_in_path="1"
    else
        echo "[INFO] $full_path Missing"
    fi
    
}

verify_needed_files() {
    local architecture_name="$1"
    local file_path="usr/bin"
    file_name=""
    pkg_name=""
    full_path=""
    
    #Set the flasher name depending on the router.
    if [ "$DEVICE" = "" ]; then
        setDevice
    fi
    if [ "$DEVICE" = "Quectel" ]; then
        file_name="$QUECTEL_FLASHER"
        elif [ "$DEVICE" = "MEIG" ]; then
        file_name="$MEIG_FLASHER"
    fi
    pkg_name="modem_updater"
    full_path="/$file_path/$file_name"
    
    file_check "$full_path"
    if [ $found_in_path = "0" ]; then
        download_file_any "$architecture_name" "$file_path" "$file_name" "$pkg_name"
    fi
    setFlasherPath
    chmod 755 $FLASHER_PATH
    
    # sshfs depends
    # libstdc++.so.6
    file_path="usr/lib"
    file_name="libstdc++.so.6"
    pkg_name="libstdcpp6"
    full_path="/$file_path/$file_name"
    
    file_check "$full_path"
    if [ $found_in_path = "0" ]; then
        download_file_any "$architecture_name" "$file_path" "$file_name" "$pkg_name"
    fi
    
    # libglib-2.0.so.0
    file_path="usr/lib"
    file_name="libglib-2.0.so.0"
    pkg_name="glib2"
    full_path="/$file_path/$file_name"
    
    file_check "$full_path"
    if [ $found_in_path = "0" ]; then
        download_file_any "$architecture_name" "$file_path" "$file_name" "$pkg_name"
    fi
    
    # fuse.ko
    file_path="lib/modules/$KERNEL_VERSION"
    file_name="fuse.ko"
    pkg_name="kmod-fuse"
    full_path="/$file_path/$file_name"
    
    file_check "$full_path"
    if [ $found_in_path = "0" ]; then
        download_file_any "$architecture_name" "$file_path" "$file_name" "$pkg_name"
    fi
    #inserting module
    look_module=$(lsmod | grep -w fuse)
    if [ "$look_module" = "" ]; then
        debug "[INFO] Inserting $file_name module"
        insmod $file_name
    fi
    
    # libfuse.so.2
    file_path="usr/lib"
    file_name="libfuse.so.2"
    pkg_name="libfuse1"
    full_path="/$file_path/$file_name"
    
    file_check "$full_path"
    if [ $found_in_path = "0" ]; then
        download_file_any "$architecture_name" "$file_path" "$file_name" "$pkg_name"
    fi
    
    # libgthread-2.0.so.0
    file_path="usr/lib"
    file_name="libgthread-2.0.so.0"
    pkg_name=glib2
    full_path="/$file_path/$file_name"
    
    file_check "$full_path"
    if [ $found_in_path = "0" ]; then
        download_file_any "$architecture_name" "$file_path" "$file_name" "$pkg_name"
    fi
    
    # sshfs
    file_path="usr/bin"
    file_name="sshfs"
    pkg_name="$file_name"
    full_path="/$file_path/$file_name"
    
    file_check "$full_path"
    if [ $found_in_path = "0" ]; then
        download_file_any "$architecture_name" "$file_path" "$file_name" "$pkg_name"
    fi
}

get_requirements() {
    if [ "$GET_REQUIREMENTS_ARG" = "show" ]; then
        curl "$HOSTNAME/dl/ver.txt"
        exit 1
    fi
    
    if [ "$GET_REQUIREMENTS_ARG" != "" ]; then
        current_version="$GET_REQUIREMENTS_ARG"
        echo "[INFO] Firmware version forced to $GET_REQUIREMENTS_ARG"
    fi
    
    check_if_tempdir_added
    echo "[INFO] Updating OPKG"
    opkg update &>/dev/null
    
    case "$CPU_NAME" in
        *MIPSEL* | *mipsel* | *mips32el*)
            echo "[INFO] device is running mipsel architechture"
            verify_needed_files "mipsel"
        ;;
        *MIPS* | *mips*)
            echo "[INFO] device is running mips architechture"
            verify_needed_files "mips"
        ;;
        
        *ARM* | *arm*)
            echo "[INFO] device is running ARM architechture"
            verify_needed_files "arm"
        ;;
        *)
            echo "[ERROR] Cannot get architechture or unknown architechture.\n exiting.."
            exit 1
        ;;
    esac
    make_ram_symlinks
    exit 1
}

while getopts "hv:i:t:fDgd:lr:p:" opt; do
    case "$opt" in
        h)
            helpFunction
        ;;
        v)
            if [ "$OPTARG" != "" ]; then
                VERSION="$OPTARG"
                DEV_MODULE=$(echo "$VERSION" | cut -c 1-8)
                #DEV_VERSION=$(echo "$VERSION" | tail -c 6 | cut -c1-2)
                debug "[INFO] $VERSION firmware version selected."
            else
                echo "[ERROR] No firmware version specified! Exiting."
                helpFunction
            fi
        ;;
        i)
            if [ "$OPTARG" != "" ]; then
                modem_id="$OPTARG"
            else
                echo "[ERROR] Modem ID not specified."
                get_modems
                helpFunction
            fi
        ;;
        t)
            if [ "$OPTARG" != "" ]; then
                TTY_PORT="$OPTARG"
            else
                echo "[ERROR] tty Port option used but port not specified? This option is needed for legacy mode only."
                helpFunction
            fi
        ;;
        p)
            if [ "$OPTARG" != "" ]; then
                USER_PATH="1"
                FW_PATH="$OPTARG"
                
            else
                echo "[ERROR] path not specified."
                helpFunction
            fi
        ;;
        f)
            SKIP_VALIDATION="1"
        ;;
        
        D)
            DEBUG_LOG="1"
            DEBUG_ECHO="1"
        ;;
        
        g)
            JUST_LIST="1"
        ;;
        d)
            if [ "$OPTARG" != "" ]; then
                DEVICE="$OPTARG"
            fi
        ;;
        l)
            LEGACY_MODE="1"
        ;;
        r)
            GET_REQUIREMENTS_ARG="$OPTARG"
            JUST_REQUIREMENTS="1"
        ;;
        ?)
            helpFunction
        ;; # Print helpFunction in case parameter is non-existent
    esac
done

if [ "$JUST_REQUIREMENTS" = "1" ]; then
    get_requirements
fi

if [ "$JUST_LIST" = "1" ]; then
    get_fw_list
fi

generic_validation(){
    if [ "$SKIP_VALIDATION" = "0" ]; then
        case "$PRODUCT_NAME" in
            TRB* | trb* )
                echo "[ERROR] TRB modems are not supported. Please use DFOTA."
                exit 1
            ;;
        esac
    fi

    if [ "$SKIP_VALIDATION" = "0" ]; then
        verify_modem_id
    fi

    if [ "$VERSION" = "" ] &&
    [ "$USER_PATH" = "0" ]; then
        echo "[ERROR] Modem version and firmware path not specified. Please use either ""-p"" or ""-v""."
        helpFunction
    fi
    
    if [ "$VERSION" != "" ] &&
    [ "$USER_PATH" != "0" ]; then
        echo "[ERROR] Modem version and firmware path are both specified. Please use either ""-p"" or ""-v""."
        helpFunction
    fi
    
    if [ "$modem_id" = "" ]; then
        echo "[ERROR] Modem ID not specified."
        get_modems
        helpFunction
    fi
    
    if [ "$TTY_PORT" != "" ] &&
    [ "$LEGACY_MODE" = "0" ]; then
        echo "[WARN] Warning you specified ttyUSB port but it will not be used for non legacy(fastboot) flash!"
    fi
    
    if [ "$DEVICE" = "" ]; then
        setDevice
    fi
    
    if [ "$LEGACY_MODE" = "1" ] && [ "$DEVICE" != "Quectel" ]; then
        echo "[ERROR] Legacy mode only supported for Quectel routers."
        exit 1
    fi
    
    setFlasherPath
    
    if [ ! -f "$FLASHER_PATH" ]; then
        echo "[ERROR] Flasher not found. You need to use \"-r\" option to install missing dependencies."
        helpFunction
    fi
    
    if [ "$DEVICE" = "Quectel" ] || [ "$DEVICE" = "MEIG" ] || [ "$DEVICE" = "QuectelASK" ]; then
        debug "[INFO] DEVICE is compatible."
    else
        if [ "$SKIP_VALIDATION" = "0" ]; then
            echo "[ERROR] Not supported or unknown modem. Exiting.."
            exit 1
        fi
    fi
    
    if [ "$SKIP_VALIDATION" = "0" ] &&
    [ "$VERSION" != "" ]; then
        MODEM_FW=$(gsmctl -y ${modem_id:+-O "$modem_id"})
        case "$MODEM_FW" in
            *$VERSION* )
                echo "[ERROR] Specified firmware is already installed. Exiting.."
                exit 1
            ;;
        esac
        #Quectel
        if [ "$DEVICE" = "Quectel" ]; then
            DEV_MOD=$(/usr/sbin/gsmctl -y -O "$modem_id" | cut -c 1-8)
            if [ "$DEV_MODULE" != "$DEV_MOD" ]; then
                echo "$DEV_MODULE != $DEV_MOD"
                echo "[ERROR] Specified firmware is not intended for this module. Exiting.."
                exit 1
            fi
        fi
        #Meig
        if [ "$DEVICE" = "MEIG" ]; then
            DEV_MOD=$(/usr/sbin/gsmctl -y -O "$modem_id" | cut -c 1-4)
            DEV_MODULE=$(echo "$VERSION" | cut -c 4-8 | tr -d _)
            if [ "$DEV_MODULE" != "$DEV_MOD" ]; then
                echo "$DEV_MODULE != $DEV_MOD"
                echo "[ERROR] Specified firmware is not intended for this module. Exiting.."
                exit 1
            fi
        fi
    fi
}

generic_prep(){
    #no sneaky modem restarting
    /etc/init.d/modem_tracker stop
    
    #backup NVRAM
    #its in the flasher now, but just in case..
    if [ "$DEVICE" = "MEIG" ]; then
        backupnvr
    fi
    
    #go into edl/disable mobile connection.
    if [ $LEGACY_MODE = "0" ]; then
        if [ "$DEVICE" = "Quectel" ]; then
            $FLASHER_PATH qfirehose -x ${modem_id:+-s /sys/bus/usb/devices/"$modem_id"}
            elif [ "$DEVICE" = "MEIG" ]; then
            $FLASHER_PATH -x ${modem_id:+-s /sys/bus/usb/devices/"$modem_id"}
        fi
    else
        gsmctl ${modem_id:+-O "$modem_id"} -A "AT+CFUN=0"
    fi
    
    #Wait for backup connection to kick in.
    sleep 10
    
    #check if connection is working
    if [ "$USER_PATH" = "0" ]; then
        validate_connection
    fi
    
    
    if [ $USER_PATH = "0" ]; then
        setup_ssh
        mount_sshfs
    fi
    debug "[INFO] Checking firmware path"
    
    if [ -z "$(ls -A $FW_PATH)" ]; then
        echo "[ERROR] firmware directory is empty. Mount failed? Exiting..."
        start_services_modems
        exit 1
    fi
    
    #some firmware sanity checks
    if [ "$DEVICE" = "Quectel" ] &&
    [ $LEGACY_MODE = "0" ] &&
    [ -z "$(ls -A $FW_PATH/update/firehose/)" ]; then
        echo "[ERROR] No firehose folder found. Either the firmware is not right or you must use legacy(fastboot) mode"
        start_services_modems
        helpFunction
        if [ "$USER_PATH" = "0" ]; then
            umount "$FW_PATH"
        fi
    fi
    
    echo "Stopping services.."
    /etc/init.d/ledman stop
    /etc/init.d/gsmd stop
}

generic_flash(){
    if [ "$DEVICE" = "MEIG" ]; then
        $FLASHER_PATH -d -f "$FW_PATH" ${modem_id:+-s /sys/bus/usb/devices/"$modem_id"}
    fi
    
    if [ "$DEVICE" = "Quectel" ]; then
        if [ $LEGACY_MODE = "1" ]; then
            #fastboot
            $FLASHER_PATH -v -m2 -f "$FW_PATH" ${TTY_PORT:+-p "$TTY_PORT"}
        else
            #firehose
            $FLASHER_PATH qfirehose -n -f "$FW_PATH/update/firehose" ${modem_id:+-s /sys/bus/usb/devices/"$modem_id"}
        fi
    fi
    
    sleep 10
    
    start_services_modems
    
    if [ "$USER_PATH" = "0" ]; then
        umount "$FW_PATH"
    fi
}

generic_validation
generic_prep

echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
echo " DO NOT TURN OFF YOUR DEVICE DURING THE UPDATE PROCESS!"
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
echo "Starting flasher..."

generic_flash

echo "Script finished"
