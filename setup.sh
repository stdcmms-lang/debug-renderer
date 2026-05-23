#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

git submodule update --init --recursive
./patches/apply-patches.sh
make setup-skia-tools
make build-skia
