
#include <Arduino.h>
#include "FastAccelStepper.h"
#include "AVRStepperPins.h" // Only required for AVR controllers
#define dirPinStepper 5
#define enablePinStepper 6
#define stepPinStepper 9

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

#define ledPin 13             // led notifikasi deccel state
#define ledGerbangTerbuka 12  // led notif gerbang terbuka
#define ledGerbangTertutup 11 // led notif gerbang tertutup
#define buzzer 10             // buzzer notifikasi
#define limitPin A1
#define receiverPin 2

// debounce tombol
unsigned long debounceDuration = 3000; // millis //daru yang semula 1000 (problem mendal, tidk dpt dibuka lagi)
unsigned long lastTimeButtonStateChanged = 0;

byte lastlimitState = LOW;
byte lastReceiverState = LOW;
bool decelState = false;
bool selesaiBukaTutup = true;
// unutk homing
bool runHoming = true;

// variabel panjang gerbang
int32_t pulse_gerbang = 11100;

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

    // speed up in ~0.025s, which needs 625 steps without linear mode
    stepper->setSpeedInHz(2000); // step/s
    // stepper->setAcceleration(8000); // step/s2 //ini tidak bisa mengerem, sehingga menabrak
    stepper->setAcceleration(250);
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
  pinMode(buzzer, OUTPUT);

  pinMode(limitPin, INPUT_PULLUP);
  pinMode(receiverPin, INPUT_PULLUP);

  // buat semua pin output mnjadi low
  digitalWrite(ledGerbangTerbuka, LOW);
  digitalWrite(ledGerbangTertutup, LOW);
  digitalWrite(buzzer, LOW);
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
    if (receiverState)
    {
      digitalWrite(ledGerbangTerbuka, HIGH);
      digitalWrite(ledGerbangTertutup, LOW);
      digitalWrite(buzzer, HIGH);
    }
    else if (!receiverState)
    {
      digitalWrite(ledGerbangTerbuka, LOW);
      digitalWrite(ledGerbangTertutup, HIGH);
      digitalWrite(buzzer, HIGH);
    }
  }

  if (!selesaiBukaTutup && runHoming) // hanya dieksekusi sekali, pada boot
  {
    stepper->setSpeedInHz(150);
    stepper->setAcceleration(5000); // kalau akselerasinya terlalu lambat, bisa2 balik nanti
    stepper->moveTo(pulse_gerbang); // perpendek lagi, dari sebelumny 31500
  }
  else if (receiverState)
  {
    stepper->setSpeedInHz(2000);
    stepper->setAcceleration(80);
    stepper->moveTo(pulse_gerbang); // sebelumnya 33245
  }
  else if (!receiverState)
  {
    stepper->setSpeedInHz(2000);
    stepper->setAcceleration(80);
    stepper->moveTo(0);
  }

  // jika motor masih bergerak, kemudian mentrigger saklar
  // maka hentikan pergerakan dan set menjadi max/min
  if (decelState && !selesaiBukaTutup)
  {
    if (runHoming)
    {
      runHoming = false;
      stepper->forceStopAndNewPosition(pulse_gerbang);
      stepper->setDelayToDisable(600);
      digitalWrite(buzzer, LOW);
    }
    else if (receiverState && !runHoming)
    {
      stepper->forceStopAndNewPosition(pulse_gerbang);
      stepper->setDelayToDisable(600);
      // matikan buzzer
      digitalWrite(ledGerbangTerbuka, HIGH);
      digitalWrite(ledGerbangTertutup, LOW);
      digitalWrite(buzzer, LOW);
    }
    else if (!receiverState && !runHoming)
    {
      stepper->forceStopAndNewPosition(0);
      stepper->setDelayToDisable(600);
      // matikan buzzer
      digitalWrite(ledGerbangTerbuka, LOW);
      digitalWrite(ledGerbangTertutup, HIGH);
      digitalWrite(buzzer, LOW);
    }

    // Serial.println("selesai!");
    selesaiBukaTutup = true;
    decelState = false; // reset rising edge
  }
}