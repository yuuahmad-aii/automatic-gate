// COBA COMIT KE MAIN GITHUB
#include <Arduino.h>
#include <AccelStepper.h>

#define VERBOSE 1
// #define PANJANG_GERBANG 10000
#define PANJANG_GERBANG 10000
#define MAX_SPD_GERBANG 1300
#define DCC_GERBANG 350
#define ACC_GERBANG 800

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

// debounce trigger
unsigned long lastDebounceTimeRf = 0;
unsigned long lastDebounceTimeTrig = 0;

const unsigned long debounceDelayRf = 100;   // 100 ms debounce delay
const unsigned long debounceDelayTrig = 100; // 100 ms debounce delay

// Setup stepper dengan menggunakan AccelStepper
AccelStepper stepper(AccelStepper::DRIVER, PULSE_PIN, DIR_PIN);

// Fungsi interrupt untuk tombol 1 (gerakkan ke kanan)
void IRAM_ATTR handleLimit()
{
  // limitState = true;
  unsigned long currentTime = millis(); // Get current time
  // Debounce check
  if ((currentTime - lastDebounceTimeTrig) > debounceDelayTrig)
  {
    limitTriggerKe++;
    lastDebounceTimeTrig = currentTime; // Update last debounce time
  }
#ifdef VERBOSE
  Serial.print("trigger ke-");
  Serial.println(limitTriggerKe);
#endif
  limitTriggerKe == 2 ? limitDecel = true : limitDecel = false;
  limitTriggerKe == 3 ? limitStop = true : limitStop = false;
}

// Fungsi interrupt untuk tombol 2 (gerakkan ke kiri)
void IRAM_ATTR handleRf()
{
  unsigned long currentTime = millis(); // Get current time
  // Debounce check
  if ((currentTime - lastDebounceTimeRf) > debounceDelayRf)
  {
    // button1Pressed = true;
    rfState = true;
    lastDebounceTimeRf = currentTime; // Update last debounce time
  }
}

void setup()
{
  // Setup pin untuk Enable driver
  pinMode(ENABLE_PIN, OUTPUT);

  // Setup pin untuk input interrupt
  pinMode(LIMIT_PIN, INPUT_PULLUP);
  pinMode(RADIO_PIN, INPUT_PULLUP);

  // Attach interrupt ke masing-masing pin
  attachInterrupt(digitalPinToInterrupt(LIMIT_PIN), handleLimit, RISING);
  attachInterrupt(digitalPinToInterrupt(RADIO_PIN), handleRf, CHANGE);

  // Konfigurasi motor stepper dengan AccelStepper
  stepper.setEnablePin(ENABLE_PIN);
  stepper.setPinsInverted(false, false, false);
  stepper.disableOutputs();             // pin enable (LOW = aktif)
  stepper.setMaxSpeed(MAX_SPD_GERBANG); // Set kecepatan maksimum (langkah per detik)
  stepper.setAcceleration(ACC_GERBANG); // Set akselerasi (langkah per detik kuadrat)
#ifdef VERBOSE
  Serial.begin(115200);
#endif
}

void loop()
{
  // perintah stop motor dan service-nya
  if (stepper.isRunning())
  {
    if (limitDecel)
    {
      limitDecel = false;
#ifdef VERBOSE
      Serial.println("Motor decel");
#endif
      stepper.setAcceleration(DCC_GERBANG); // Set dekselerasi (langkah per detik kuadrat)
      stepper.stop();                       // stop motor dengan dekselerasi
    }
    else if (limitStop)
    {
      limitStop = false;
      limitTriggerKe = 0;
#ifdef VERBOSE
      Serial.println("Motor disable");
#endif
      stepper.disableOutputs(); // stop motor secara instan (immediately)
      if (digitalRead(RADIO_PIN))
        stepper.setCurrentPosition(PANJANG_GERBANG); // Gerakkan motor PANJANG_GERBANG langkah ke kiri
      else
        stepper.setCurrentPosition(-PANJANG_GERBANG); // Gerakkan motor PANJANG_GERBANG langkah ke kiri
    }
  }
  else // reset semua flag jika motor tidak bergerak
  {
    limitDecel = false;
    limitStop = false;
    limitTriggerKe = 0;
    stepper.disableOutputs();
  }

  // Jika tombol 2 ditekan, gerakkan motor ke kiri
  if (rfState)
  {
    rfState = false;                      // Reset flag interrupt
    stepper.setAcceleration(ACC_GERBANG); // Set akselerasi (langkah per detik kuadrat)
    if (digitalRead(RADIO_PIN))
    {
#ifdef VERBOSE
      Serial.println("Motor ke kanan");
#endif
      stepper.enableOutputs();
      stepper.move(PANJANG_GERBANG); // Gerakkan motor PANJANG_GERBANG langkah ke kiri
    }
    else
    {
#ifdef VERBOSE
      Serial.println("Motor ke kiri");
#endif
      stepper.enableOutputs();
      stepper.setAcceleration(ACC_GERBANG); // Set akselerasi (langkah per detik kuadrat)
      stepper.move(-PANJANG_GERBANG);       // Gerakkan motor PANJANG_GERBANG langkah ke kiri
    }
  }
  stepper.run();
}
