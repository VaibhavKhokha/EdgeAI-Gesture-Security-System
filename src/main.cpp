#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <Arduino.h>

#include <Wire.h>
#include <iostream>
#include <string>
#include "model.h"

#define I2C_SDA_PIN 8 
#define I2C_SCL_PIN 9
#define MPU_ADDR 0x68

enum SystemState { WAITING, RECORDING, THINKING };
SystemState currentState = WAITING;
bool promptPrinted = false;

unsigned long prevMillis = 0;
int current_sample = 0;
const unsigned long eventInterval = 50;

//Globals and memory

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
tflite::ErrorReporter* error_reporter = nullptr;

TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// 8KB memory
constexpr int kTensorArenaSize = 8 * 1024;
uint8_t* tensor_arena = nullptr;

void setup()
{
    Serial.begin(115200);

    delay(3000);

    tensor_arena = new uint8_t[kTensorArenaSize];
    
    //mpu
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(400000);
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);

    if(Wire.endTransmission() != 0)
    {
        Serial.println("MPU connection Failed..");
        while(1) yield();
    }

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1C);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1B);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1A);
    Wire.write(0x04);
    Wire.endTransmission();

    Serial.println("MPU ready...");

    // setting logging
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    //Mapping model to cpp array
    model = tflite::GetModel(Gesture_model_tflite);
    if(model->version() != TFLITE_SCHEMA_VERSION)
    {
        Serial.println("Model schema mismatch!");
        return;
    }

    //pull in all the math ops
    static tflite::AllOpsResolver resolver;

    //brain
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    //claiming mem
    interpreter->AllocateTensors();

    //assigning input and output params
    input = interpreter->input(0);
    output = interpreter->output(0);

    Serial.println("Model Booted :)");


}

void loop()
{
    switch(currentState)
    {
        case WAITING:
            if(!promptPrinted)
            {
                Serial.println("\n---------------------------------------");
                Serial.println("Press [Enter] to start: ");
                Serial.println("-----------------------------------------");
                promptPrinted = true;
            }

            if(Serial.available() > 0)
            {
                while(Serial.available() > 0)
                {
                    Serial.read();
                }

                Serial.print("[Recording Started] Perform Gesture: ");

                currentState = RECORDING;
                current_sample = 0;
                promptPrinted = false;
                prevMillis = millis();
            }
            break;



        case RECORDING:
            if(millis() - prevMillis >= eventInterval)
            {
                prevMillis = millis();

                Wire.beginTransmission(MPU_ADDR);
                Wire.write(0x3B);
                Wire.endTransmission(false);

                Wire.requestFrom(MPU_ADDR, 14, true);

                if(Wire.available() == 14)
                {
                    int16_t raw_Ax = Wire.read() << 8 | Wire.read();
                    int16_t raw_Ay = Wire.read() << 8 | Wire.read();
                    int16_t raw_Az = Wire.read() << 8 | Wire.read();
                    int16_t raw_temp = Wire.read() << 8 | Wire.read();
                    int16_t raw_Gx = Wire.read() << 8 | Wire.read();
                    int16_t raw_Gy = Wire.read() << 8 | Wire.read();
                    int16_t raw_Gz = Wire.read() << 8 | Wire.read();

                    float Ax = raw_Ax / 16384.0;
                    float Ay = raw_Ay / 16384.0;
                    float Az = raw_Az / 16384.0;

                    float Gx = (raw_Gx / 131.0) * 0.0174533;
                    float Gy = (raw_Gy / 131.0) * 0.0174533;
                    float Gz = (raw_Gz / 131.0) * 0.0174533;
        

                    //writing to input memory of model(in 1D)
                    input->data.f[current_sample * 6 + 0] = Ax;
                    input->data.f[current_sample * 6 + 1] = Ay;
                    input->data.f[current_sample * 6 + 2] = Az;
                    input->data.f[current_sample * 6 + 3] = Gx;
                    input->data.f[current_sample * 6 + 4] = Gy;
                    input->data.f[current_sample * 6 + 5] = Gz;

                    current_sample++;
                    Serial.print(".");

                    if(current_sample == 40)
                    {
                        Serial.println(" [DONE] ");
                        currentState = THINKING;
                    }
                }
                else
                {
                    Serial.println("\nI2C ERROR...");
                    currentState = WAITING;
                }
            }
            break;

        case THINKING:
            TfLiteStatus invoke_status = interpreter->Invoke();

            if (invoke_status != kTfLiteOk)
            {
                Serial.println("AI Invoke Failed...");
                currentState = WAITING;
                break;
            }

            float confidence = output->data.f[0];

            if(confidence > 0.90)
            {
                Serial.print(" ACCESS GRANTED... Confidence:  ");
                Serial.println(confidence);

                Serial.println(" Unlocked... Auto-locking in 5 seconds...");
                delay(5000);
            }
            else
            {
                Serial.print(" ACCESS DENIED... Confidence: ");
                Serial.println(confidence);
                Serial.println(" Try Again... ");
                delay(1000);
            }

            currentState = WAITING;
            break;
    }
}