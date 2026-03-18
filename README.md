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
* `web/` browser interface
* `docs/` technical documentation
* `hardware/` wiring and hardware notes
* `images/` screenshots and photos

---

# Development Roadmap

## Phase 1 – Basic System

* [X] Setup PlatformIO project
* [X] Connect ESP32 to WiFi
* [ ] Start ESP32 Web Server
* [ ] Serve a simple HTML test page

---

## Phase 2 – Servo Control

* [ ] Control **1 servo motor**
* [ ] Add web slider for servo angle
* [ ] Send angle from browser to ESP32
* [ ] Move servo from browser

---

## Phase 3 – Full Robot Control

* [ ] Add control for **6 servo motors**
* [ ] Implement Robotic Arm class
* [ ] Create Servo Controller module
* [ ] Add safety limits for angles

---

## Phase 4 – Web Interface

* [ ] Create UI with sliders for all axes
* [ ] Display current servo angles
* [ ] Add "Home Position" button
* [ ] Improve layout and styling

---

## Phase 5 – Position Storage

* [ ] Save current arm position
* [ ] List stored positions
* [ ] Load stored position
* [ ] Delete stored position

---

## Phase 6 – Motion Sequences

* [ ] Create motion sequences
* [ ] Add multiple positions to sequence
* [ ] Play motion sequence
* [ ] Stop running sequence

---

## Phase 7 – Improvements

* [ ] Smooth motion interpolation
* [ ] Mobile-friendly UI
* [ ] API documentation
* [ ] Add architecture diagram

---

# Technologies

* ESP32
* C++
* Arduino Framework
* HTML / CSS / JavaScript
* REST-style API

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

