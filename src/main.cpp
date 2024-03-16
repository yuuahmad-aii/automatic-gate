
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
// unutk homing
bool runHoming = true;

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
    stepper->setAcceleration(850);
    // stepper->set
  }
  else
    while (true)
    {
      // Serial.println("NO STEPPER");
      delay(1000);
    }

  pinMode(ledPin, OUTPUT);
  pinMode(ledGerbangTerbuka, OUTPUT);
  pinMode(ledGerbangTertutup, OUTPUT);
  pinMode(limitPin, INPUT_PULLUP);
  pinMode(receiverPin, INPUT_PULLUP);
}

void loop()
{
  // perintah dari receiver radio
  byte receiverState = digitalRead(receiverPin);
  // debounce sekaligus setting trigger jadi rising edge
  if (millis() - lastTimeButtonStateChanged > debounceDuration)
  {
    byte limitState = digitalRead(limitPin);
    if (limitState != lastlimitState)
    {
      lastTimeButtonStateChanged = millis();
      lastlimitState = limitState;
      if (limitState == HIGH)
        decelState = true;
    }
  }

  // perintahkan buka / tutup hanya jika relay tertrigger
  // edge trigger (deteksi rising dan falling)
  if (receiverState != lastReceiverState)
  {
    lastReceiverState = receiverState;
    selesaiBukaTutup = false;
  }

  if (!selesaiBukaTutup && runHoming) // hanya dieksekusi sekali, pada boot
  {
    stepper->setSpeedInHz(500);
    // stepper->setAcceleration(500); // kalau akselerasinya terlalu lambat, bisa2 balik nanti
    stepper->setAcceleration(10000);
    stepper->moveTo(33245);
    digitalWrite(ledGerbangTerbuka, HIGH);
    digitalWrite(ledGerbangTertutup, LOW);
  }
  else if (receiverState)
  {
    stepper->setSpeedInHz(2000);
    stepper->setAcceleration(150);
    stepper->moveTo(33245);
    digitalWrite(ledGerbangTerbuka, HIGH);
    digitalWrite(ledGerbangTertutup, LOW);
  }
  else if (!receiverState)
  {
    stepper->setSpeedInHz(1500);
    stepper->setAcceleration(200);
    stepper->moveTo(0);
    digitalWrite(ledGerbangTerbuka, LOW);
    digitalWrite(ledGerbangTertutup, HIGH);
  }

  // jika motor masih bergerak, kemudian mentrigger saklar
  // maka hentikan pergerakan dan set menjadi max/min
  if (decelState && !selesaiBukaTutup)
  {
    if (runHoming)
    {
      runHoming = false;
      // stepper->setCurrentPosition(33245);
      // stepper->forceStop(); // stop tanpa deccelerasi
      stepper->forceStopAndNewPosition(33245);
      stepper->setDelayToDisable(3000);
    }
    else if (receiverState && !runHoming)
    {
      // stepper->setCurrentPosition(33245);
      // stepper->forceStop(); // stop tanpa deccelerasi
      stepper->forceStopAndNewPosition(33245);
      stepper->setDelayToDisable(3000);
    }
    else if (!receiverState && !runHoming)
    {
      // stepper->setCurrentPosition(0);
      // stepper->forceStop(); // stop tanpa deccelerasi
      stepper->forceStopAndNewPosition(0);
      stepper->setDelayToDisable(3000);
    }
    // Serial.println("selesai!");
    selesaiBukaTutup = true;
    decelState = false; // reset rising edge
  }
}