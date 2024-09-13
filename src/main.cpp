// COBA COMIT KE MAIN GITHUB
#include <Arduino.h>
#include <AccelStepper.h>

// #define verbose 1

// Definisikan pin untuk driver stepper
#define PULSE_PIN D1  // Pin untuk sinyal Pulse (pulsa)
#define DIR_PIN D2    // Pin untuk sinyal Direction (arah)
#define ENABLE_PIN D3 // Pin untuk Enable driver

// Definisikan pin untuk input interrupt
#define LIMIT_PIN D5 // Tombol 1 (GPIO14)
#define RADIO_PIN D6 // Tombol 2 (GPIO12)

// Variabel untuk menampung state interrupt
volatile bool limitState = false;
volatile bool limitDecel = false;
volatile bool limitStop = false;
volatile int limitTriggerKe = 0;
volatile bool rfState = false;

// Setup stepper dengan menggunakan AccelStepper
AccelStepper stepper(AccelStepper::DRIVER, PULSE_PIN, DIR_PIN);

// Fungsi interrupt untuk tombol 1 (gerakkan ke kanan)
void IRAM_ATTR handleLimit()
{
  // limitState = true;
  limitTriggerKe++;
#ifdef verbose
  Serial.print("trigger ke-");
  Serial.println(limitTriggerKe);
#endif
  limitTriggerKe == 2 ? limitDecel = true : limitDecel = false;
  limitTriggerKe == 3 ? limitStop = true : limitStop = false;
}

// Fungsi interrupt untuk tombol 2 (gerakkan ke kiri)
void IRAM_ATTR handleRf()
{
  rfState = true;
}

void setup()
{
  // Setup pin untuk Enable driver
  pinMode(ENABLE_PIN, OUTPUT);

  // Setup pin untuk input interrupt
  pinMode(LIMIT_PIN, INPUT_PULLUP);
  pinMode(RADIO_PIN, INPUT_PULLUP);

  // Attach interrupt ke masing-masing pin
  attachInterrupt(digitalPinToInterrupt(LIMIT_PIN), handleLimit, FALLING);
  attachInterrupt(digitalPinToInterrupt(RADIO_PIN), handleRf, CHANGE);

  // Konfigurasi motor stepper dengan AccelStepper
  stepper.setEnablePin(ENABLE_PIN);
  stepper.setPinsInverted(false, false, false);
  stepper.disableOutputs();     // pin enable (LOW = aktif)
  stepper.setMaxSpeed(1000);    // Set kecepatan maksimum (langkah per detik)
  stepper.setAcceleration(500); // Set akselerasi (langkah per detik kuadrat)
#ifdef verbose
  Serial.begin(115200);
#endif
}

void loop()
{
  // Jika tombol 1 ditekan, gerakkan motor ke kanan
  if (limitDecel)
  {
    limitDecel = false;
#ifdef verbose
    Serial.println("Motor decel");
#endif
    stepper.stop(); // stop motor
  }
  else if (limitStop)
  {
    limitStop = false;
    limitTriggerKe = 0;
#ifdef verbose
    Serial.println("Motor disable");
#endif
    stepper.disableOutputs(); // stop motor
    if (digitalRead(RADIO_PIN))
      stepper.setCurrentPosition(8000); // Gerakkan motor 8000 langkah ke kiri
    else
      stepper.setCurrentPosition(-8000); // Gerakkan motor 8000 langkah ke kiri
  }

  // Jika tombol 2 ditekan, gerakkan motor ke kiri
  if (rfState)
  {
    rfState = false; // Reset flag interrupt
    if (digitalRead(RADIO_PIN))
    {
#ifdef verbose
      Serial.println("Motor ke kanan");
#endif
      stepper.enableOutputs();
      stepper.move(8000); // Gerakkan motor 8000 langkah ke kiri
    }
    else
    {
#ifdef verbose
      Serial.println("Motor ke kiri");
#endif
      stepper.enableOutputs();
      stepper.move(-8000); // Gerakkan motor 8000 langkah ke kiri
    }
  }
  stepper.run();
}
