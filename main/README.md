This README provides a comprehensive overview of the **ESP32-S3 Development Board**, detailing its high-performance specifications, hardware layout, and configuration options based on the provided technical sources.

# ESP32-S3 Development Board (N16R8/N8R2)

This document provides a detailed technical overview of the ESP32-S3 development board, including its internal circuitry, hardware modifications, and interface options.

## 1. Core Technical Specifications
*   **Processor:** Xtensa® dual-core 32-bit LX7 (Revision v0.2), 240 MHz.
*   **External Flash:** 16 MB Boya Flash, 80 MHz, DIO Mode.
*   **External PSRAM:** 8 MB Octal PSRAM (AP Vendor), 80 MHz.
*   **Connectivity:** 2.4 GHz Wi-Fi (802.11 b/g/n) and Bluetooth 5 (LE) with Mesh support.
*   **Power:** AMS1117-3.3 LDO regulator for stable 3.3V power, supported by 10uF/0.1uF filtering capacitors for noise reduction.
*   **Protection:** Integrated protection diodes to prevent current backflow to USB ports when using external power.
*   **Console:** Hardware UART on GPIO 43/44.

---

## 2. Ports and Communication Modes
The board features two Type-C USB ports with distinct hardware pathways:

*   **USB Mode (Native USB & OTG):**
    *   Directly connected to the ESP32-S3 **USB Serial/JTAG Controller** (GPIO 19/20).
    *   **Best for:** Hardware-level debugging (JTAG) and high-speed data transfer.
*   **UART Mode (Hardware Serial):**
    *   Uses an onboard **CH343P USB-to-Serial chip** connected to GPIO 43 (TX) and 44 (RX).
    *   **Best for:** Standard programming, serial monitoring, and general debugging in Arduino or MicroPython.

---

## 3. Buttons and Status Indicators
### Control Buttons
*   **RST:** Triggers a system reset.
*   **BOOT (GPIO0):** Used for "standby download mode." Hold BOOT and press RST to enter the bootloader.

### Hardware LEDs
*   **PWR:** Red LED; indicates 3.3V rail activity.
*   **TX / RX:** Blinking LEDs connected to the CH343P chip; indicate serial traffic.
*   **RGB LED (WS2812):** Connected to **GPIO48** (Note: Some diagrams incorrectly label this as GPIO47).

---

## 4. Hardware Customization (Soldering Required)
*   **5V Power Output:** By default, the "5Vin" pin is an **input only**. To enable **5V Output** (to power displays/sensors via USB), you must **solder a short circuit** at the "IN-OUT" jumper position.
*   **External RGB Bypass:** To use an external LED strip instead of the onboard RGB, remove the **"0R" (zero-ohm) resistor** (R3) to disconnect the internal data signal.
*   **Auto-Program Circuit:** The schematic includes an automatic reset/boot circuit (managed via DTR/RTS) so you don't have to press buttons for every upload.

---

## 5. Peripheral Interfaces & External Displays
### Capacitive Touch Sensors
The board features **14 touch-sensing GPIOs** (TOUCH1 to TOUCH14) located on GPIOs 1–14.

### I2C Interface (Adafruit 7-Segment Support)
The ESP32-S3 can map I2C to any GPIO, but defaults are often **GPIO 8 (SDA)** and **GPIO 9 (SCL)**.
*   **Support for Adafruit 1.2" 4-Digit Display:**
    *   **Power:** Connect to the 5V rail (requires the 5V output mod or an external supply).
    *   **Revision Note:** Backpacks revised after June 30, 2023, include a **5V boost converter** and **level shifters**, allowing them to run on 3.3V power and logic while still reaching full LED brightness.
    *   **Address:** I2C 7-bit addresses are selectable between **0x70–0x77**.

---

## 6. Comprehensive Pin Mapping (44-Pin DIP)

| GPIO | Function | Touch | Special Features |
| :--- | :--- | :--- | :--- |
| **0** | BOOT | — | Strapping pin for bootloader |
| **1-2** | General IO | T1, T2 | — |
| **3** | JTAG | T3 | Hardware debugging |
| **4-7** | ADC1 | T4-T7 | — |
| **8-9** | **I2C (Default)** | T8, T9 | SDA (8) / SCL (9) |
| **10-14** | SPI / ADC | T10-T14 | — |
| **15-16** | XTAL_32K | — | Supports external 32kHz crystal |
| **17-18** | UART1 | — | TX (17) / RX (18) |
| **19-20** | **Native USB** | — | USB D- (19) / USB D+ (20) |
| **43-44** | **Serial Debug** | — | UART0 TX (43) / RX (44) via CH343P |
| **47** | General IO | — | Often confused with RGB LED |
| **48** | **RGB LED** | — | Drives the onboard WS2812 |

## 7. Pin Layout

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
