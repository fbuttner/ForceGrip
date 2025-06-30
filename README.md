# ForceGrip

ForceGrip is a device designed to measure and track finger and hand strength. It uses a force-sensitive load cell connected to an ESP32 to provide real-time data. You can connect to the ForceGrip via Bluetooth Low Energy (BLE)  or a local WiFi network to view your strength data.

## ✨ Features

- **Real-time Finger Strength Measurement**: Get instant feedback on your finger strenght.
- **Dual-Mode Connectivity**:
    - **Bluetooth LE**: Connects as a standard BLE Weight Scale device.
    - **WiFi Access Point**: Creates its own WiFi network to serve a web-based dashboard.
- **Web Dashboard**: View live data and configure the device through a simple web interface when in WiFi mode.
- **Easy Mode Switching**: A long press on the button (2 seconds) toggles between BLE and WiFi modes.
- **SPIFFS Filesystem**: Hosts the web interface files directly on the ESP32.

## ⚙️ Hardware Components

- ESP32-based board : e.g., LOLIN C3 PICO
- An HX711 load cell amplifier module
- A load cell : salvage from a WH-C06 digital scale
- A momentary push button : salvage from a WH-C06 digital scale
- A battery : salvage from a WH-C06 digital scale

## 🚀 Usage

### BLE Mode
By default, the ForceGrip starts in Bluetooth LE mode. You can connect to it using any application that supports the standard BLE Weight Scale service to read the real-time force data.

### WiFi Mode

1.  To switch to WiFi mode, press and hold the button for 2 seconds.
2.  The device will create a WiFi access point. Connect to it with the following credentials:
    - **SSID**: `ForceGrip`
    - **Password**: `ForceGripPWD`
3.  Once connected, open a web browser and navigate to `http://192.168.4.1`.
4.  The web page will display real-time data and provide options for device configuration.


## 🛠️ Development Setup

### 1. Prerequisites
- **Visual Studio Code**: Download and install.
- **PlatformIO IDE Extension**: Open VSCode, go to the Extensions view (`Ctrl+Shift+X`), search for `PlatformIO IDE`, and install it.

### 2. Clone the Repository
```bash
git clone https://github.com/fbuttner/ForceGrip.git
cd ForceGrip
```

### 3. Open the Project with PlatformIO

- Launch VSCode.
- Open the PlatformIO Home by clicking the alien icon in the sidebar or using Ctrl+Alt+P → “PlatformIO: Home”.
- Go to Open Project and select the folder you just cloned.
- PlatformIO will automatically load the environment and dependencies.

### 4. Build and Upload Firmware

Use the PlatformIO toolbar at the bottom of the VSCode window:
- Click the **Build** button (✔️ icon) to compile the firmware.
- Click the **Upload** button (➡️ icon) to flash the firmware to your ESP32.

### 5. Upload Web Interface (SPIFFS)
The web interface files are located in the `data` directory and need to be uploaded to the ESP32's SPIFFS (SPI Flash File System).

You can do this in two ways:

- **Via the PlatformIO UI:**
    1.  Click the PlatformIO icon in the VSCode sidebar.
    2.  Expand your project environment (e.g., `lolin_c3_mini`).
    3.  Expand the `Platform` menu.
    4.  Click **Upload Filesystem Image**.

- **Via the PlatformIO CLI:**
    1.  Open a new PlatformIO CLI terminal in VSCode.
    2.  Run the command:
        ```bash
        pio run --target uploadfs
        ```
