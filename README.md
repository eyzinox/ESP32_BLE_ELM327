# ESP32_BLE_ELM327

**ESP32-based ELM327 emulator over Bluetooth Low Energy (BLE)**  

This project turns your ESP32 into a virtual ELM327 interface. It supports a minimal set of OBD-II commands, returns simulated CAN responses in ASCII format, and logs both received and transmitted data in HEX and ASCII. Ideal for testing OBD-II apps, learning automotive communication protocols, or debugging BLE-based automotive tools without a real car.  

**Compatible with iOS and Android via BLE.**

---

## Features

- Emulates **ELM327 v1.5**
- Supports basic **AT commands**:
  - `ATZ`, `ATI`, `ATE0`, `ATL0`, `ATS0`, `ATH1`, `ATDP`, `ATSP6`, `ATRV`, `ATDPN`
- Handles **OBD-II PID queries** like `0100`, `010C`, `010D`
- Logs all BLE traffic in **HEX** and **ASCII**
- Responds with default or custom simulated CAN frames
- Lightweight and easy to extend
- **BLE-compatible** for iOS and Android apps

---

## Installation

1. Install [PlatformIO](https://platformio.org/) in Visual Studio Code.
2. Clone this repository:

   ```bash
   git clone https://github.com/your-username/ESP32_BLE_ELM327.git
   cd ESP32_BLE_ELM327

    Open the project in VSCode.

    Connect your ESP32.

    Build and upload the project:

    `pio run --target upload`

    Or use the PlatformIO buttons in VSCode.

## Usage

    Open a BLE-capable OBD-II app (e.g., CarScanner, Torque Pro) on iOS or Android.

    Connect to the ESP32 device named ESP32_BLE_ELM327.

    Send standard ELM327 commands:
    ```
    ATZ
    ATI
    0100
    010C
    010D
    ```
    Observe the HEX and ASCII logs in the Serial Monitor.

    Modify main.cpp to extend or customize command responses as needed.

## Example Logs

```
=== BLE RX ===
HEX   : 41 54 5A 0D
ASCII : ATZ.
================
Command received: 'ATZ'
TX ASCII: ELM327 v1.5>

=== BLE RX ===
HEX   : 30 31 30 44 0D
ASCII : 010C.
================
Command received: '010C'
TX ASCII: 7E8 04 41 0C 0B B8>
```