#!/bin/bash

brand_prefix="RUT9XX"
brand_number="00"
branch_prefix=""
version_major=""
version_minor=""
version_patch=""
gpl=0
brand_firmware_name=""

while [ $# -gt 0 ]; do
	case "$1" in
		--brand-prefix)
			brand_prefix="$2"
			shift
			shift
		;;
		--brand-number)
			brand_number="$2"
			shift
			shift
		;;
		--branch-prefix)
			branch_prefix="$2"
			shift
			shift
		;;
		--gpl)
			gpl=1
			shift
		;;
		*)
			shift
		;;
	esac
done

if [ -n "$CI_COMMIT_REF_NAME" ]; then
	branch="$CI_COMMIT_REF_NAME"
else
	branch=$(git rev-parse --abbrev-ref HEAD)
fi

describe=$(git describe)
describe=${describe##*_}

IFS='.-' read -ra version_array <<< "$describe"

for i in ${!version_array[@]}; do
	case ${version_array[i]} in
		''|*[!0-9]*)
		;;
		*)
			if [ $i -eq 0 ]; then
				version_major=$(printf "%02d" ${version_array[i]})
			elif [ $i -eq 1 ]; then
				version_minor=$(printf "%02d" ${version_array[i]})
			else
				version_patch=${version_array[i]}
			fi
		;;
	esac
done

if [ -z "$branch_prefix" ]; then
	case "$branch" in 
		master*)
			if [ $gpl -eq 0 ]; then
				branch_prefix="R"
			else
				branch_prefix="R_GPL"
			fi
		;;
		develop*) 
			branch_prefix="T_DEV"
		;;
		feature*) 
			branch_prefix=${branch#*/}
			branch_prefix=${branch_prefix%%-*}
			branch_prefix="T_F${branch_prefix}"
		;;
		release*) 
			branch_prefix=${branch#*/}
			branch_prefix=$(echo $branch_prefix | tr -d '.')
			branch_prefix="T_R${branch_prefix}"
		;;
		hotfix*) 
			branch_prefix=${branch#*/}
			branch_prefix=$(echo $branch_prefix | tr -d '.')
			branch_prefix="T_H${branch_prefix}"
		;;
		*)
			pref=${branch%%-*}
			case $pref in
				''|*[!0-9]*) branch_prefix="T" ;;
				*)
					pref=$((pref+1000)) # Kad nesidubliuotu ID su feature/* sakom
					printf -v pref "%04d" $pref
					branch_prefix="T_F${pref}"
				;;
			esac
		;;
	esac
fi

brand_firmware_name="${brand_prefix}_${branch_prefix}_${brand_number}.${version_major}.${version_minor}"

if [ -n "$version_patch" ]; then
	brand_firmware_name="${brand_firmware_name}.${version_patch}"
fi

printf "$brand_firmware_name"

