# ForceGrip

ForceGrip is a device designed to measure and track finger and hand strength. It uses a force-sensitive load cell connected to an ESP32 to provide real-time data. You can connect to the ForceGrip via Bluetooth Low Energy (BLE)  or a local WiFi network to view your strength data.

## ✨ Features

- **Real-time Finger Strength Measurement**: Get instant feedback on your finger strenght.
- **Dual-Mode Connectivity**:
    - **Bluetooth LE**: Connects as a standard BLE Weight Scale device.
    - **WiFi Client & Access Point**: Connects to an existing WiFi network or creates its own Access Point for direct connection.
- **Web Dashboard**: View live data and configure the device through a simple web interface.
- **Easy Mode Switching**: A long press on the button (2 seconds) toggles between BLE and WiFi modes.
- **SPIFFS Filesystem**: Hosts the web interface files directly on the ESP32.

## ⚙️ Hardware Components

- ESP32-based board : e.g., LOLIN C3 PICO
- An HX711 load cell amplifier module
- A load cell : salvage from a WH-C06 digital scale
- A momentary push button : salvage from a WH-C06 digital scale
- A battery : salvage from a WH-C06 digital scale

## 🕹️ Usage

### BLE Mode
By default, the ForceGrip starts in Bluetooth LE mode. You can connect to it using any application that supports the standard BLE Weight Scale service to read the real-time force data.

### WiFi Mode

To switch to WiFi mode, press and hold the button for 2 seconds. The device supports two WiFi modes: Client (STA) mode and Access Point (AP) mode.

#### Client Mode (Connecting to an existing network)
When configured with your WiFi credentials, the device will connect to your existing network.
1.  Ensure your computer or phone is on the same network.
2.  Open a web browser and navigate to `http://forcgrip.local`.
3.  The web page will display real-time data and provide options for device configuration, including setting up your WiFi credentials .

If the device cannot connect to a configured network, it will automatically fall back into Access Point mode.

#### Access Point Mode (Initial Setup)
If the device has not been configured to connect to your WiFi, or if the connection fails, it will create its own Access Point. This mode is typically used for initial setup.
1.  Connect to the device's WiFi network:
    - **SSID**: `ForceGrip`
    - **Password**: `ForceGripPWD`
2.  Once connected, open a web browser and navigate to `http://192.168.4.1` or `http://forcgrip.local`.
3.  The web page will display real-time data and provide options for device configuration, including setting up your WiFi credentials .



## 🚀 Flashing The Firmware

There are two ways to get the firmware onto your device.

### 1. Web Flasher (Easy Way, Recommended)

If you don't want to compile the code, you can flash the device directly from your browser.

1.  Use a **Google Chrome** or **Microsoft Edge** browser.
2.  Connect your ForceGrip device to your computer via USB.
3.  Go to the **ForceGrip Web Flasher**.
4.  Follow the on-screen instructions to connect to your device and flash the latest firmware.

### 2. Building from Source (For Developers)

This method is for users who want to modify the code and compile it themselves.

#### 1.Prerequisites

- **Visual Studio Code**: Download and install.
- **PlatformIO IDE Extension**: Open VSCode, go to the Extensions view (`Ctrl+Shift+X`), search for `PlatformIO IDE`, and install it.

#### 2. Clone the Repository
```bash
git clone https://github.com/fbuttner/ForceGrip.git
cd ForceGrip
```

#### 3. Open the Project with PlatformIO

- Launch VSCode.
- Open the PlatformIO Home by clicking the alien icon in the sidebar or using Ctrl+Alt+P → “PlatformIO: Home”.
- Go to Open Project and select the folder you just cloned.
- PlatformIO will automatically load the environment and dependencies.

#### 4. Build and Upload Firmware

Use the PlatformIO toolbar at the bottom of the VSCode window:
- Click the **Build** button (✔️ icon) to compile the firmware.
- Click the **Upload** button (➡️ icon) to flash the firmware to your ESP32.

#### 5. Upload Web Interface (SPIFFS)
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
