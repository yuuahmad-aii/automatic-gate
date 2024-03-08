
#include <Arduino.h>
#include "FastAccelStepper.h"
#include "AVRStepperPins.h" // Only required for AVR controllers
#define dirPinStepper 5
#define enablePinStepper 6
#define stepPinStepper 9
// If using an AVR device use the definitons provided in AVRStepperPins
//    stepPinStepper1A
//
// or even shorter (for 2560 the correct pin on the chosen timer is selected):
//    stepPinStepperA
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

#define ledPin 13             // led notifikasi deccel state
#define ledGerbangTerbuka 12  // led notif gerbang terbuka
#define ledGerbangTertutup 11 // led notif gerbang tertutup
#define limitPin A1
#define receiverPin 2

unsigned long debounceDuration = 1000; // millis
unsigned long lastTimeButtonStateChanged = 0;
// int i = 0;

byte lastlimitState = LOW;
byte lastReceiverState = LOW;
bool decelState = false;
bool selesaiBukaTutup = true;

void setup()
{
  // Serial.begin(115200);
  // Serial.println("START");

  engine.init();
  stepper = engine.stepperConnectToPin(stepPinStepper);
  if (stepper)
  {
    // Serial.println("HAVE STEPPER");
    stepper->setDirectionPin(dirPinStepper);
    stepper->setEnablePin(enablePinStepper);
    stepper->setAutoEnable(true);

    // If auto enable/disable need delays, just add (one or both):
    // stepper->setDelayToEnable(50);
    // stepper->setDelayToDisable(1000);

    // speed up in ~0.025s, which needs 625 steps without linear mode
    stepper->setSpeedInHz(2000); // step/s
    // stepper->setAcceleration(8000); // step/s2 //ini tidak bisa mengerem, sehingga menabrak
    stepper->setAcceleration(1400);
    // stepper->set
  }
  else
  {
    while (true)
    {
      // Serial.println("NO STEPPER");
      delay(1000);
    }
  }

  pinMode(ledPin, OUTPUT);
  pinMode(ledGerbangTerbuka, OUTPUT);
  pinMode(ledGerbangTertutup, OUTPUT);
  pinMode(limitPin, INPUT_PULLUP);
  pinMode(receiverPin, INPUT_PULLUP);
}

void loop()
{
  // debounce sekaligus setting trigger jadi rising edge
  byte receiverState = digitalRead(receiverPin);
  if (millis() - lastTimeButtonStateChanged > debounceDuration)
  {
    byte limitState = digitalRead(limitPin);
    if (limitState != lastlimitState)
    {
      lastTimeButtonStateChanged = millis();
      lastlimitState = limitState;
      if (limitState == HIGH)
      {
        decelState = true;
        // trigger = true;
        // do an action, for example print on Serial
        // Serial.print(i);
        // Serial.println("Button released");
        // i++;
      }
    }
  }

  // limit decelState hanya selama interval
  // if (currentMillis - previousMillis >= interval) {
  //   // save the last time you blinked the LED
  //   previousMillis = currentMillis;
  //   decelState = false;
  // }
  // digitalWrite(ledPin, decelState);

  // perintahkan buka / tutup hanya jika relay tertrigger
  if (receiverState != lastReceiverState)
  {
    lastReceiverState = receiverState;
    selesaiBukaTutup = false;
  }

  if (receiverState)
  {
    stepper->moveTo(31168);
    digitalWrite(ledGerbangTerbuka, HIGH);
    digitalWrite(ledGerbangTertutup, LOW);
  }
  else if (!receiverState)
  {
    stepper->moveTo(0);
    digitalWrite(ledGerbangTerbuka, LOW);
    digitalWrite(ledGerbangTertutup, HIGH);
  }

  if (decelState && !selesaiBukaTutup)
  {
    if (receiverState)
      stepper->setCurrentPosition(31168);
    else if (!receiverState)
      stepper->setCurrentPosition(0);
    // Serial.println("selesai!");
    selesaiBukaTutup = true;
    decelState = false;
  }
}