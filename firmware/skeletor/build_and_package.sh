#!/bin/bash
# Build and Package Script for Skeletor Firmware
# Builds the firmware, creates data filesystem, and packages everything

set -e  # Exit on error

echo "=== Skeletor Firmware Build & Package Script ==="

# Get current directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Get git commit hash for versioning
if git rev-parse --short HEAD > /dev/null 2>&1; then
    COMMIT_HASH=$(git rev-parse --short HEAD)
else
    COMMIT_HASH="unknown"
fi

echo "Building for commit: $COMMIT_HASH"

# Clean previous builds
echo "Cleaning previous build artifacts..."
rm -rf .pio/build
rm -f skeletor-firmware-*.zip
rm -f *.sha256

# Build the firmware
echo "Building firmware..."
pio run -e cardputer-adv

# Check if firmware was built successfully
if [ ! -f ".pio/build/cardputer-adv/firmware.bin" ]; then
    echo "ERROR: firmware.bin not found!"
    exit 1
fi

echo "Firmware build successful!"

# Build data filesystem image
echo "Building data filesystem..."
if pio run -e cardputer-adv -t buildfs 2>&1 | grep -q "success\|Successfully"; then
    echo "Filesystem build successful!"
else
    echo "Warning: Filesystem build may have issues, continuing..."
fi

# Create output directory for packaging
OUTPUT_DIR="build_output"
mkdir -p "$OUTPUT_DIR"

# Copy firmware binaries
echo "Collecting binaries..."
cp .pio/build/cardputer-adv/firmware.bin "$OUTPUT_DIR/" 2>/dev/null || true

# Copy partitions if they exist
if [ -f ".pio/build/cardputer-adv/partitions.bin" ]; then
    cp .pio/build/cardputer-adv/partitions.bin "$OUTPUT_DIR/"
    echo "  - firmware.bin"
    echo "  - partitions.bin"
else
    echo "  - firmware.bin"
    echo "  (partitions.bin not found, skipping)"
fi

# Copy bootloader if it exists
if [ -f ".pio/build/cardputer-adv/bootloader.bin" ]; then
    cp .pio/build/cardputer-adv/bootloader.bin "$OUTPUT_DIR/"
    echo "  - bootloader.bin"
fi

# Generate SHA256 checksums
echo "Generating checksums..."
cd "$OUTPUT_DIR"
for file in *.bin; do
    if [ -f "$file" ]; then
        sha256sum "$file" > "${file}.sha256"
        echo "  SHA256($file): $(cat ${file}.sha256 | cut -d' ' -f1)"
    fi
done

# Create zip package
PACKAGE_NAME="skeletor-firmware-${COMMIT_HASH}.zip"
echo "Creating package: $PACKAGE_NAME"
zip -r "../$PACKAGE_NAME" *.bin *.sha256

cd ..
rm -rf "$OUTPUT_DIR"

echo ""
echo "=== Build Complete ==="
echo "Package created: $PACKAGE_NAME"
echo "Contents:"
unzip -l "$PACKAGE_NAME"
echo ""
echo "To flash: pio run -e cardputer-adv -t upload"
