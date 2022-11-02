#!/bin/sh

# Enable IPv6 on RUTOS when upgrading from legacy FW
sed -i 's/net\.ipv6\.conf\.all\.disable_ipv6=1/net\.ipv6\.conf\.all\.disable_ipv6=0/' /etc/sysctl.conf
