#include <Arduino.h>
#include "FastAccelStepper.h"
#include "AVRStepperPins.h" // Only required for AVR controllers

// stepper pin
#define dirPinStepper 5
#define enablePinStepper 6
#define stepPinStepper 9

// Pin definitions
#define limitProxyPin 2 // pin limit proxy connected to pin 2 (interrupt 0)
#define rfReceiverPin 3 // rf receiver connected to pin 3 (interrupt 1)

// Variables to keep track of button states
volatile bool limitProxyTriggered = false;
volatile bool rfReceiverChange = false;

// Debounce timing
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
const unsigned long debounceDelay = 100; // 100 ms debounce delay

// variabel untuk service gerbang
bool runHoming = true;

// initialize stepper
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

// initialize indicator pinout
#define ledPin 13             // led notifikasi deccel state
#define ledGerbangTerbuka 12  // led notif gerbang terbuka
#define ledGerbangTertutup 11 // led notif gerbang tertutup
#define buzzer 10             // buzzer notifikasi

int32_t pulse_gerbang = 11100;

// Interrupt Service Routine for pin limit proxy (falling edge on pin 2)
void button1ISR()
{
  unsigned long currentTime = millis(); // Get current time
  // Debounce check
  if ((currentTime - lastDebounceTime1) > debounceDelay)
  {
    // digitalRead(limitProxyPin) == 0 ? limitProxyTriggered = true : limitProxyTriggered = false;
    limitProxyTriggered = true;
    lastDebounceTime1 = currentTime; // Update last debounce time
  }
}

// Interrupt Service Routine for rf receiver (rising and falling edge on pin 3)
void button2ISR()
{
  unsigned long currentTime = millis(); // Get current time
  // Debounce check
  if ((currentTime - lastDebounceTime2) > debounceDelay)
  {
    rfReceiverChange = true;
    lastDebounceTime2 = currentTime; // Update last debounce time
  }
}

void setup()
{
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

  // pinmode untuk pin indikator
  pinMode(ledPin, OUTPUT);
  pinMode(ledGerbangTerbuka, OUTPUT);
  pinMode(ledGerbangTertutup, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // buat semua pin output mnjadi low pada awal program
  digitalWrite(ledGerbangTerbuka, LOW);
  digitalWrite(ledGerbangTertutup, LOW);
  digitalWrite(buzzer, LOW);

  // Set pins as inputs
  pinMode(limitProxyPin, INPUT_PULLUP); // Using internal pull-up resistor
  pinMode(rfReceiverPin, INPUT_PULLUP);

  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(limitProxyPin), button1ISR, FALLING); // Interrupt on falling edge
  attachInterrupt(digitalPinToInterrupt(rfReceiverPin), button2ISR, CHANGE);  // Interrupt on both rising and falling edge

  // Initialize serial communication for debugging
  Serial.begin(115200);
}

void loop()
{
  // Check if limit proxy was triggered (falling edge karena pin di pullup internal)
  if (limitProxyTriggered)
  {
    Serial.println("limit proxy triggered (falling edge)");
    limitProxyTriggered = false; // Reset the flag
    if (runHoming)               // kalau sedang homing (awal hidup)
    {
      runHoming = false; // reset flag run homing ketika tertrigger
      stepper->forceStopAndNewPosition(pulse_gerbang);
      stepper->disableOutputs();
      digitalWrite(buzzer, LOW);
    }
    else
    {
      if (digitalRead(rfReceiverPin) == LOW)
      {
        stepper->forceStopAndNewPosition(pulse_gerbang);
        stepper->disableOutputs();
        // matikan buzzer
        digitalWrite(ledGerbangTerbuka, HIGH);
        digitalWrite(ledGerbangTertutup, LOW);
        digitalWrite(buzzer, LOW);
      }
      else
      {
        stepper->forceStopAndNewPosition(0);
        stepper->disableOutputs();
        // matikan buzzer
        digitalWrite(ledGerbangTerbuka, LOW);
        digitalWrite(ledGerbangTertutup, HIGH);
        digitalWrite(buzzer, LOW);
      }
    }
  }

  // Check if rf receiver signal state changed (either rising or falling edge detected)
  if (rfReceiverChange) // pin3
  {
    rfReceiverChange = false; // Reset the flag
    if (runHoming)            // hanya dieksekusi sekali, saat pertamakali hidup
    {
      stepper->setSpeedInHz(150);
      stepper->setAcceleration(5000); // kalau akselerasinya terlalu lambat, bisa2 balik nanti
      stepper->moveTo(pulse_gerbang); // perpendek lagi, dari sebelumny 31500
    }
    else
    {
      if (digitalRead(rfReceiverPin) == LOW)
      {
        Serial.println("rf receiver pressed (falling edge)");
        stepper->setSpeedInHz(2000);
        stepper->setAcceleration(80);
        stepper->moveTo(pulse_gerbang); // sebelumnya 33245
      }
      else
      {
        Serial.println("rf receiver released (rising edge)");
        stepper->setSpeedInHz(2000);
        stepper->setAcceleration(80);
        stepper->moveTo(0);
      }
    }
  }
}
