#!/bin/bash
OUTPUT_DIR="compiled"
mkdir -p "$OUTPUT_DIR"

for f in *.vert *.frag; do
    [ -e "$f" ] && glslc "$f" -o "$OUTPUT_DIR/$f.spv"
done

for f in *.comp; do
    [ -e "$f" ] && glslc "$f" -o "$OUTPUT_DIR/${f%.comp}.spv"
done
