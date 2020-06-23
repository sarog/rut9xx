#!/usr/bin/env bash
shopt -s extglob
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. >/dev/null && pwd)"

FIRMWARE_FOLDER="${PROJECT_ROOT}/bin/ar71xx/tltFws"
SEADRIVE_PATH_1="/Shared with groups/GitLab_Upload/RUT9/"
SEADRIVE_PATH_2="/Shared with groups/GitLab_Upload/RUT9/GPL/"

seadrive_counter=0
seadrive_counter_max=10
sleep_seconds=300
upload_fw=0
upload_gpl=0
upload_base_dir=""
upload_dir=""
upload_dir_tmp=""
upload_dir_level=""
upload_dir_level_tmp=""
upload_dir_level_tmp2=""
upload_file=""
upload_file_tmp=""
artifact_file=""
file_index=0
upload_index=0
declare -a src_files
declare -a dst_files

while [ $# -ne 0 ]; do
	case "$1" in
		--upload_dir)
			if [ -d "${2}" ]; then
				upload_base_dir="${2}"
			else
				echo "SUPPLIED UPLOAD DIRECTORY DOES NOT EXIST!"
			fi
			shift
			shift
		;;
		--firmware)
			upload_fw=1
			shift
		;;
		--gpl)
			upload_gpl=1
			shift
		;;
		*)
			shift
		;;
	esac
done

while [ "${seadrive_counter}" -lt "${seadrive_counter_max}" ]; do
	if [ "${upload_fw}" = 1 ] && [ -d "${upload_base_dir}${SEADRIVE_PATH_1}" ]; then
		break;
	elif [ "${upload_gpl}" = 1 ] && [ -d "${upload_base_dir}${SEADRIVE_PATH_2}" ]; then
		break;
	else
		seadrive_counter=$((seadrive_counter+1))
		echo "SEADRIVE UPLOAD PATH DOES NOT EXIST! Waiting for SeaDrive to sync and checking again after 5 seconds (${seadrive_counter}/${seadrive_counter_max})"
		sleep 5
	fi
done

if [ "${upload_fw}" = 1 ] && [ -d "${upload_base_dir}${SEADRIVE_PATH_1}" ]; then
	mapfile -t files < <(ls -1 "${FIRMWARE_FOLDER}"/*.bin)

	if [ "${#files[@]}" -gt 0 ]; then
		for file in "${files[@]}"; do
			upload_file="${file##*/}"

			if [ -z "${upload_file##*UBOOT*}" ]; then
				upload_file_tmp="${upload_file%_UBOOT*}"
			elif [ -z "${upload_file##*WEBUI*}" ]; then
				upload_file_tmp="${upload_file%_WEBUI*}"
			else
				echo "Unknown file: '${upload_file}', skipping"
				continue
			fi

			src_files[file_index]="${file}"
			upload_dir_level_tmp="${upload_file%%.*}"
			upload_dir_level_tmp2="${upload_dir_level_tmp%_*}"
			upload_dir_level="${upload_file%%_*}_${upload_dir_level_tmp##*_}/${upload_dir_level_tmp2%%+([[:digit:]])}/"

			if [ -z "${upload_dir_level_tmp2##*+([[:digit:]])}" ]; then
				upload_dir_level="${upload_dir_level}${upload_dir_level_tmp2}/"
			fi

			upload_dir="${upload_base_dir}${SEADRIVE_PATH_1}${upload_dir_level}${upload_file_tmp}"

			if [ -d "${upload_dir}" ]; then
				for count in {000..999}; do
					upload_dir_tmp="${upload_dir}/${upload_file_tmp}_${count}"
					if [ ! -d "${upload_dir_tmp}" ]; then
						upload_dir="${upload_dir_tmp}"
						if [ -z "${upload_file##*UBOOT*}" ]; then
							dst_files[file_index]="${upload_dir}/${upload_file_tmp}_${count}_UBOOT${upload_file#*UBOOT}"
						elif [ -z "${upload_file##*WEBUI*}" ]; then
							dst_files[file_index]="${upload_dir}/${upload_file_tmp}_${count}_WEBUI${upload_file#*WEBUI}"
						fi
						break
					fi
				done
			else
				dst_files[file_index]="${upload_dir}/${upload_file}"
			fi

			file_index=$((file_index+1))
		done

		artifact_file=".gitlab/firmware_upload_path"
	else
		echo "NO FIRMWARE FILE TO UPLOAD!"
		exit 2
	fi
elif [ "${upload_gpl}" = 1 ] && [ -d "${upload_base_dir}${SEADRIVE_PATH_2}" ]; then
	mapfile -t files < <(ls -1 "${PROJECT_ROOT}"/*.tar.gz)
	if [ "${#files[@]}" -gt 0 ]; then
		src_files[file_index]="${files[-1]}"
		upload_file="${files[-1]##*/}"
		upload_file_tmp="${upload_file%.tar*}"
		upload_dir="${upload_base_dir}${SEADRIVE_PATH_2}${upload_file_tmp}"
	else
		echo "NO GPL ARCHIVE TO UPLOAD!"
		exit 2
	fi

	if [ -d "${upload_dir}" ]; then
		for count in {000..999}; do
			upload_dir_tmp="${upload_dir}/${upload_file_tmp}_${count}"
			if [ ! -d "${upload_dir_tmp}" ]; then
				upload_dir="${upload_dir_tmp}"
				dst_files[file_index]="${upload_dir}/${upload_file_tmp}_${count}.tar.gz"
				break
			fi
		done
	else
		dst_files[file_index]="${upload_dir}/${upload_file}"
	fi

	file_index=$((file_index+1))
	artifact_file=".gitlab/gpl_upload_path"
	sleep_seconds=600
else
	echo "SEADRIVE UPLOAD PATH DOES NOT EXIST! NOT UPLOADING FIRMWARE TO SEADRIVE!"
	exit 1
fi

src_files[file_index]="${PROJECT_ROOT}/changelog"
dst_files[file_index]="${upload_dir}/changelog.txt"
file_index=$((file_index+1))

if [ -n "$CI_COMMIT_REF_NAME" ]; then
	branch_name="$CI_COMMIT_REF_NAME"
else
	branch_name=$(git rev-parse --abbrev-ref HEAD)
fi

echo "SEADRIVE UPLOAD PATH: '${upload_dir}'"
mkdir -p "${upload_dir}"

while [ "${upload_index}" -lt "${file_index}" ]; do
	if [ -z "${src_files[upload_index]##*UBOOT*}" ] && [ -n "${branch_name##master*}" ]; then
		upload_index=$((upload_index+1))
		continue
	fi

	if [ -z "${src_files[upload_index]##*changelog*}" ]; then
		if [ -n "${branch_name##master*}" ] && [ -n "${branch_name##hotfix*}" ] && [ -n "${branch_name##release*}" ]; then
			upload_index=$((upload_index+1))
			continue
		fi
	fi

	md5_checksum="$(md5sum "${src_files[upload_index]}" | cut -d' ' -f1)"
	sha256_checksum="$(sha256sum "${src_files[upload_index]}" | cut -d' ' -f1)"
	echo "Uploading file: '${dst_files[upload_index]##*/}', MD5 checksum: '${md5_checksum}', SHA256 checksum: '${sha256_checksum}'"
	cp "${src_files[upload_index]}" "${dst_files[upload_index]}"
	upload_index=$((upload_index+1))
done

echo "Waiting for Seafile to sync files (${sleep_seconds} seconds)"
sync
sleep "${sleep_seconds}"

if [ -e "${upload_dir}" ]; then
	echo "Upload SUCCESSFUL: ${upload_dir}"
	echo "${upload_dir#*GitLab_Upload}" > "${artifact_file}"
else
	echo "Upload FAILED!"
fi
