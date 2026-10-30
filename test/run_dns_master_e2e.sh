#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "usage: $0 /path/to/ve-tos-cpp-sdk-test" >&2
    exit 2
fi

test_binary=$(realpath "$1")
test_dir=$(cd "$(dirname "$0")" && pwd)

exec timeout 120s unshare -m -n --propagation private -- bash -c '
    ip link set lo up
    mount --bind "$1" /etc/resolv.conf
    mount -t tmpfs -o size=1m tmpfs /run/nscd
    exec env -u LD_PRELOAD -u http_proxy -u https_proxy -u all_proxy \
        -u HTTP_PROXY -u HTTPS_PROXY -u ALL_PROXY FSX_DNS_REAL_TEST=1 "$2" \
        --gtest_filter=DnsFallbackEndToEndTest.*
' _ "$test_dir/resources/dns-master-e2e-resolv.conf" "$test_binary"
