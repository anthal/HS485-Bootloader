Bootloader for ELV HS485 bus
============================

test-bootloader
---------------
* Bootloader for Atmel AVR ATmega8/328
* Compiler: **WinAVR-20100110**
* IDE: **Programmer's Notepad** (from WinAVR)

test-app
--------
* Application example for ATmega with bootloader
* Compiler: **WinAVR-20100110**


hs485_flash_updater
-------------------
* WARNING! Errors and incomplete!
* Windows application for programming of ATmega
* Compiler: **Bloodshed Dev-C++ 5.6.3** (http://www.bloodshed.net/)

SW-Tools
--------
* Windows 8.1 with **Git GUI** and **Git Bash**
* Compiler for AVR : **WinAVR-20100110**
* IDE for AVR: **Programmer's Notepad** (from WinAVR)
* Compiler for hs485_flash_updater: **Bloodshed Dev-C++ 5.6.3** (http://www.bloodshed.net/)


HW-Tools
--------
* **AVRISP** mkII with AVR Studio 4.19


Git
---
 `git add .`
 
 `git commit -m "name of commit"`
 
 `git push -u origin master`
 
ToDo
----

- Programmierung über HS485 nach Schalten nicht mehr möglich (kein Sprung zum Bootloader ?!)
- in SendAck2 0x19 ersetzen
- in SendState 0xA1 ersetzen


