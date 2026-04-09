# ESP32 Web Robot Arm

A practice and portfolio project for controlling a **6-axis robotic arm via a web interface running on an ESP32**.

The project demonstrates how embedded hardware can be controlled through a browser interface using a built-in microcontroller web server.

---

# Goal

The goal of this project is to build a **browser-based control system for a robotic arm with multiple servo motors**.

The ESP32 acts as a **WiFi web server**, allowing the robotic arm to be controlled from any device with a browser.

---

# Project Structure

* `firmware/` ESP32 source code
* `firmware/data/` HTML, CSS and JavaScript files served from LittleFS on the ESP32
* `web/` browser prototypes and experiments
* `docs/` technical documentation
* `hardware/` wiring and hardware notes
* `images/` screenshots and photos

---



# Technologies

* ESP32
* C++
* Arduino Framework
* HTML / CSS / JavaScript
* LittleFS
* REST-style API

---

# Firmware Upload

The web interface is split into separate files in `firmware/data/` and is served from the ESP32 file system.

After flashing the firmware, upload the LittleFS data partition as well:

```powershell
cd firmware
pio run -t upload
pio run -t uploadfs
```

---

# Learning Goals

This project is intended to practice:

* Embedded systems programming
* Hardware control using microcontrollers
* Web servers on embedded devices
* Modular software architecture
* API-driven hardware control

---

# Planned Hardware

* ESP32 microcontroller
* 6 servo motors
* 3D printed robotic arm
* external 5V power supply

Optional:

* PCA9685 servo controller

---

# License

MIT License

