# ESP32-S3 TinyML Gesture-Security-System

A bare-metal, offline gesture-recognition security system built using an ESP32-S3 (N16R8) and TensorFlow Lite for Microcontrollers.

## Overview
This system processes raw accelerometer and gyroscope data from an MPU6050 to detect gestures locally. It uses an **Event-Driven State Machine** to minimize power consumption and requires no cloud connectivity.

## Project Structure
* **`src/`**: Firmware logic.
    * `main.cpp`: Bare-metal I2C driver, state machine (Waiting/Recording/Thinking), and TFLite inference.
    * `model.h`: The neural network weights, optimized for ESP32-S3.
* **`tools/`**: Machine Learning pipeline.
    * `data_collector.py`: The interface script that connects to the MPU6050 and captures raw sensor streams for training.
    * `sensor_data_preprocessing.ipynb`: Cleans, scales, and labels the raw CSV sensor data into a format suitable for neural network input.
    * `kinematic_dataset.npz`: The final, structured dataset used to train the model.
    * `Neural_training.ipynb`: The Keras/TensorFlow training environment where the neural network architecture was defined and trained.
    * `Converting_for_Esp32.ipynb`: Handles the conversion from a standard TensorFlow model to a quantized **TensorFlow Lite (`.tflite`)** file, optimized for memory constraints on the ESP32-S3.
    * `Gesture_model.tflite`: The final, high-performance model artifact used for real-time inference on the device.
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
