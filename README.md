# ESP32-S3 TinyML Gesture-Controlled Lock

A bare-metal, offline gesture-recognition security system built using an ESP32-S3 (N16R8) and TensorFlow Lite for Microcontrollers.

## Overview
This system processes raw accelerometer and gyroscope data from an MPU6050 to detect gestures locally. It uses an **Event-Driven State Machine** to minimize power consumption and requires no cloud connectivity.

## Project Structure
* **`src/`**: Firmware logic.
    * `main.cpp`: Bare-metal I2C driver, state machine (Waiting/Recording/Thinking), and TFLite inference.
    * `model.h`: The neural network weights, optimized for ESP32-S3.
* **`tools/`**: Machine Learning pipeline.
    * `Neural_training.ipynb`: Python code for model training.
    * `data_collector.py`: Script used to capture gesture data from the sensor.
* **`platformio.ini`**: Hardware build configuration for the ESP32-S3.

## Hardware Setup
| Component | Connection |
| :--- | :--- |
| **MPU6050 VCC** | 3.3V |
| **MPU6050 GND** | GND |
| **MPU6050 SDA** | GPIO 8 |
| **MPU6050 SCL** | GPIO 9 |

## How It Works
1. **WAITING**: The system idles until the user triggers input via Serial.
2. **RECORDING**: Samples 40 data points (accelerometer/gyro) at 20Hz.
3. **THINKING**: The data is fed into the TFLite interpreter.
4. **ACTION**: If confidence > 90%, the system unlocks.

##  Citation & License
This project is licensed under the MIT License. If you use or build upon this code, please provide attribution by linking back to this repository.