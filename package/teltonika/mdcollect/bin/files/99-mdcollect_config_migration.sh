#!/bin/sh

if [ -z "$(uci -q get mdcollectd.config.ignore)" ]; then
  uci -q add_list mdcollectd.config.ignore=lo
else
  uci -q del_list mdcollectd.config.ignore=wwan0
fi
