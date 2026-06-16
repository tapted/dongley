This README provides a comprehensive overview of the **ESP32-S3 Development Board**, detailing its high-performance specifications, hardware layout, and configuration options based on the provided technical sources.

# ESP32-S3 Development Board (N16R8/N8R2)

This development board is a high-performance, low-power MCU SoC designed for **IoT (Internet of Things)** and **AIoT** applications. It integrates **2.4 GHz Wi-Fi** and **Bluetooth 5 (LE)**, making it an ideal choice for smart homes, industrial automation, and image recognition.

## 1. Technical Specifications
*   **Core:** Xtensa® dual-core 32-bit LX7 CPU, running at up to **240 MHz**.
*   **Memory (Model Dependent):**
    *   **N16R8:** 16 MB External Flash, **8 MB External PSRAM**.
    *   **N8R2:** 8 MB External Flash, 2 MB External PSRAM.
*   **Wireless:** IEEE 802.11 b/g/n Wi-Fi and Bluetooth LE 5 with Mesh support.
*   **Operating Voltage:** 3.0V to 3.6V.
*   **Advanced Features:** AI acceleration instructions, hardware security (AES-XTS, RSA, Secure Boot), and 14 capacitive touch-sensing GPIOs.

---

## 2. Ports and Connectivity
The board features **dual USB Type-C ports**, offering two distinct modes for communication and programming:

*   **USB Mode (Native USB & OTG):** Connected directly to the ESP32-S3's internal **USB Serial/JTAG Controller**. This port supports high-speed data transfer, USB On-The-Go (OTG), and integrated hardware debugging (JTAG).
*   **UART Mode (USB-to-Serial):** Connected to an onboard **CH343P chip**. This is used for traditional serial communication and is often the standard choice for serial monitoring in environments like Arduino.

---

## 3. Physical Buttons and Indicators
### Buttons
*   **RST (Reset):** Manually restarts the system.
*   **BOOT:** Connected to **GPIO0**. This is a strapping pin used to enter **standby download mode** (bootloader) by holding it down while pressing RST.

### Status LEDs
*   **PWR:** A red LED indicating that the board is receiving 3.3V power.
*   **TX/RX:** Two LEDs that flash to indicate serial data activity via the CH343P chip.
* **RGB LED:** A programmable **WS2812 RGB LED** physically connected to **GPIO48** (Note: Often incorrectly documented by manufacturers as GPIO47).

---

## 4. Hardware Configuration & Modifications
The board includes several "solder jumpers" and component positions to customize hardware behavior:

*   **5V Output Solder Jumper:** By default, the 5V pin on the header is a **5V Input**. To use this pin as a **5V Output** for expansion boards (drawing power from the USB port), you must **solder a short circuit** at the designated position marked "IN-OUT" near the power supply chip.
*   **External RGB Light:** If you wish to use an external RGB strip instead of the onboard WS2812, you must remove the corresponding **"0R" capacitor/resistor** at the designated position to disconnect the onboard LED.

---

## 5. Pin Definitions Mapping
The board exposes 44 pins in a dual-inline package (DIP) layout.

| Pin Position | PCB Label | GPIO / Primary Function | Features |
| :--- | :--- | :--- | :--- |
| **Left 1-2** | 3V3 | Power | 3.3V Power Rail |
| **Left 3** | RST | Reset | System Reset |
| **Left 4-7** | 4-7 | GPIO4-7 | ADC1, TOUCH4-7, PWM |
| **Left 8-9** | 15, 16 | GPIO15, 16 | XTAL_32K, ADC2, PWM |
| **Left 10-11** | 17, 18 | GPIO17, 18 | U1TXD/RXD, ADC2, PWM |
| **Left 12** | 8 | GPIO8 | SDA (Default), ADC1, PWM |
| **Left 13** | 3 | GPIO3 | JTAG, ADC1, TOUCH3, PWM |
| **Left 14** | 46 | GPIO46 | LOG, PWM |
| **Left 15** | 9 | GPIO9 | SCL (Default), ADC1, TOUCH9 |
| **Left 16-20** | 10-14 | GPIO10-14 | SPI, ADC, TOUCH10-14, PWM |
| **Left 21** | 5Vin | Power | 5V Input (Default) / Output (Mod) |
| **Left 22** | GND | Ground | Common Ground |
| | | | |
| **Right 1** | GND | Ground | Common Ground |
| **Right 2-3** | TX, RX | GPIO43, 44 | U0TXD/RXD (Serial Debug) |
| **Right 4-5** | 1, 2 | GPIO1, 2 | ADC1, TOUCH1-2, PWM |
| **Right 6-13** | 42-35 | GPIO42-35 | MTMS, MTDI, MTCK, SPI, PWM |
| **Right 14** | 0 | GPIO0 | **BOOT** (Strapping Pin) |
| **Right 15-16** | 45, 48 | GPIO45, 48 | VSPI, PWM (Onboard WS2812 is on 48) |
| **Right 17** | 47 | GPIO47 | General GPIO (Often mislabeled as RGB LED) |
| **Right 18** | 21 | GPIO21 | RTC Power Domain |
| **Right 19-20** | 20, 19 | GPIO20, 19 | **USB D+ / D-** (Native USB) |
| **Right 21-22** | GND | Ground | Common Ground |
