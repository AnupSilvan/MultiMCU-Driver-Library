MultiMCU Driver Library

A structured collection of reusable embedded driver modules for multiple microcontroller platforms including ATSAM4, STM, Arduino, and other ARM-based MCUs. This repository is designed to serve as a centralized driver library that can be easily integrated into embedded firmware projects.

📂 Repository Structure
Microchip_ATSAM4_Driver_Files/
│
├── Digital Input Driver
├── Digital_Output_LED
├── Ext EEPROM Files
├── Ext Flash Files
├── I2C_driver
├── RS485 Files
├── RTC_driver
├── SRAM_Driver
├── TIMER Files
├── UART Files
├── USB Files
└── User_SPI


Each folder contains standalone driver modules with initialization, configuration, and functional APIs.

🎯 Purpose

This repository was created to:

Maintain a reusable driver codebase

Reduce development time in embedded projects

Standardize driver architecture

Provide plug-and-play modules

Keep tested drivers organized by peripheral type

⚙️ Supported Peripherals

Current drivers include:

GPIO Digital Input

LED Output

External EEPROM

External Flash Memory

I²C Communication

RS485 Communication

Real Time Clock (RTC)

External SRAM

Timers

UART

USB

SPI (User-defined)

🧠 Design Philosophy

Modular architecture

Hardware abstraction friendly

Minimal dependencies

Lightweight implementation

Easy portability across MCUs

🚀 Usage

Copy required driver folder into your project.

Include header file in your source:

#include "uart_driver.h"


Initialize driver:

UART_Init();


Call APIs as required.

📌 Notes

Drivers are written in Embedded C.

Designed primarily for bare-metal firmware.

Can be adapted for RTOS environments.

👨‍💻 Author

Anup Silvan
Embedded Systems Engineer
Specialized in ARM Microcontrollers & Firmware Development
