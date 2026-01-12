# Skeletor Firmware

PlatformIO-based firmware project for Cardputer ADV (M5Stack ESP32-S3).

## Description

Skeletor is a firmware application for the M5Stack Cardputer ADV that displays random catchphrases with a simple skull animation. The firmware reads catchphrases from SPIFFS storage and cycles through them with an animated skull display.

## Features

- Random catchphrase display from SPIFFS or built-in fallback list
- Simple two-frame skull animation
- Serial logging for debugging
- Optional OTA (Over-The-Air) updates when WiFi credentials are provided

## Quick Build & Flash

### Prerequisites

- [PlatformIO](https://platformio.org/) installed
- M5Stack Cardputer ADV hardware

### Build Instructions

1. Clone this repository
2. Navigate to the firmware directory:
   ```bash
   cd firmware/skeletor
   ```

3. Build the firmware:
   ```bash
   pio run -e cardputer-adv
   ```

4. Flash to device:
   ```bash
   pio run -e cardputer-adv -t upload
   ```

5. Monitor serial output:
   ```bash
   pio device monitor
   ```

### Building Data Filesystem (Optional)

To upload the SPIFFS data (catchphrases and assets):

```bash
pio run -e cardputer-adv -t uploadfs
```

## Enabling OTA Updates

To enable Over-The-Air (OTA) firmware updates:

1. Copy the secrets example file:
   ```bash
   cp src/secrets.h.example src/secrets.h
   ```

2. Edit `src/secrets.h` and add your WiFi credentials:
   ```cpp
   #define WIFI_SSID "YourNetworkName"
   #define WIFI_PASSWORD "YourPassword"
   ```

3. Rebuild and flash the firmware

**Note:** The firmware compiles without `src/secrets.h`. OTA is only enabled when valid WiFi credentials are provided.

## Build Script

The `build_and_package.sh` script automates building and packaging:

```bash
./build_and_package.sh
```

This script:
- Builds the firmware with PlatformIO
- Builds the data filesystem image
- Collects firmware.bin and partitions.bin
- Computes SHA256 checksums
- Creates a zip package: `skeletor-firmware-<commit>.zip`

## CI Artifacts

GitHub Actions automatically builds firmware on every push and pull request. 

### Downloading CI Artifacts

1. Go to the [Actions tab](https://github.com/Crashkeyz/What-me/actions)
2. Select the workflow run
3. Download the firmware artifacts (firmware.bin, partitions.bin)

The CI workflow produces:
- `firmware.bin` - Main firmware binary
- `partitions.bin` - Partition table (if present)

These can be flashed using esptool or PlatformIO.

## Project Structure

```
firmware/skeletor/
├── platformio.ini          # PlatformIO configuration
├── src/
│   ├── main.cpp           # Main firmware code
│   └── secrets.h.example  # WiFi credentials template
├── data/
│   ├── catchphrases.txt   # Sample catchphrases
│   └── assets/            # Skull animation BMP files
├── build_and_package.sh   # Build and packaging script
└── README.md              # This file
```

## License

MIT License - See LICENSE file for details

## Links

- Issue: https://github.com/Crashkeyz/What-me/issues/1
- Repository: https://github.com/Crashkeyz/What-me
