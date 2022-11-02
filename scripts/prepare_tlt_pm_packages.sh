#!/usr/bin/env bash

[ "$#" -ne 4 ] && {
	echo "Usage prepare_tlt_pm_package.sh <packages> <top_dir> <arch> <package_dir>"
	exit 1
}

packages=$1
top_dir=$2
arch=$3
target_package_dir=$4
pm_packages="$top_dir/bin/packages/$arch/pm_packages"
zipped_packages="$top_dir/bin/packages/$arch/zipped_packages"
base_dir="$top_dir/bin/packages/$arch/base"
vuci_dir="$top_dir/bin/packages/$arch/vuci"
packages_dir="$top_dir/bin/packages/$arch/packages"
json_file="$top_dir/ipk_packages.json"

cd "$pm_packages"

prepare_control_f() {

	local package=$1
	local name="$2"
	local list="Firmware tlt_name Router Package Version pkg_reboot"
	
	tar -xvf "$package"_* --get "./control.tar.gz" || return 0

	tar -xvf control.tar.gz --get "./control" && rm control.tar.gz || return 0

	touch main
	for i in $list; do
		grep "$i" control >> main
	done
	rm control

	tar -uf "$zipped_packages/$name.tar" ./main
	rm main
}

for p in $packages; do
	case "$p" in
		hs_theme*|vuci-i18n-*)
			mv "$vuci_dir/$p"_* "$pm_packages" 2>/dev/null
			;;
		iptables-*)
			mv "$target_package_dir/$p"_* "$pm_packages" 2>/dev/null
			;;
		*)
			mv "$base_dir/$p"_* "$pm_packages" 2>/dev/null
			;;
	esac

	[ $? -eq 0 ] || [ -f "$pm_packages/$p"_* ] || continue
	name=$(jq -r ".\"$p\".name" "$json_file")
	[ "$name" = "null" ] && name="$p"
	tar -cf "$zipped_packages/$name.tar" ./"$p"_*

	prepare_control_f "$p" "$name"
	
	for d in $(jq -r ".\"$p\".base_dep" "$json_file"); do
		[ "$d" = "null" ] && break
		mv "$base_dir/$d"_* "$pm_packages" 2>/dev/null
		tar -uf "$zipped_packages/$name.tar" ./"$d"_*
	done
	for d in $(jq -r ".\"$p\".vuci_dep" "$json_file"); do
		[ "$d" = "null" ] && break
		mv "$vuci_dir/$d"_* "$pm_packages" 2>/dev/null
		tar -uf "$zipped_packages/$name.tar" ./"$d"_*
	done
	for d in $(jq -r ".\"$p\".packages_dep" "$json_file"); do
		[ "$d" = "null" ] && break
		mv "$packages_dir/$d"_* "$pm_packages" 2>/dev/null
		tar -uf "$zipped_packages/$name.tar" ./"$d"_*
	done
	for d in $(jq -r ".\"$p\".kmod_dep" "$json_file"); do
		[ "$d" = "null" ] && break
		mv "$target_package_dir/$d"_* "$pm_packages" 2>/dev/null
		tar -uf "$zipped_packages/$name.tar" ./"$d"_*
	done

	gzip "$zipped_packages/$name.tar"
done

