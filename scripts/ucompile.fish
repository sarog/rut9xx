#!/bin/fish

if ! test -z $argv
        set PKG_NAME "$argv"
else
        set PKG_NAME "base-files"
end

make "package/$PKG_NAME/toolchain-dump"

set files (string split ";" -- (cat tmp/.tc))

for f in $files; 
        if string match -q -- "TOOLCHAIN_DIR*" "$f"
                set TC_PATH (string replace "TOOLCHAIN_DIR=" "" "$f")
                if not string match -q -- "$TC_PATH/*" "$PATH"
                        set -x PATH "$TC_PATH/bin:" $PATH
                end
        else
                set -x (string split --max 1 "=" $f)
        end
end

