#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "usage: $0 OPEN_SYOBON_DIRECTORY OUTPUT_SOURCE_DIRECTORY" >&2
    exit 2
fi

upstream_directory=$1
output_directory=$2

mkdir -p "$output_directory/BGM" "$output_directory/SE"

for category in BGM SE; do
    for source in "$upstream_directory/$category"/*.ogg; do
        filename=$(basename "${source%.ogg}")
        ffmpeg -loglevel error -y -i "$source" \
            -ac 1 -ar 22050 -c:a adpcm_ima_wav \
            "$output_directory/$category/$filename.wav"
    done
done
