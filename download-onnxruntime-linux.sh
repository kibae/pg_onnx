#!/usr/bin/env bash

cd "$(dirname "$0")" || exit

onnxruntime-server/download-onnxruntime-linux.sh "$@"

