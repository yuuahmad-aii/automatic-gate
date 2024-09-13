#include <AccelStepper.h>

// Definisikan pin untuk driver stepper
#define PULSE_PIN D1  // Pin untuk sinyal Pulse (pulsa)
#define DIR_PIN D2    // Pin untuk sinyal Direction (arah)
#define ENABLE_PIN D3 // Pin untuk Enable driver

// Definisikan pin untuk input interrupt
#define INTERRUPT_PIN_1 D5 // Tombol 1 (GPIO14)
#define INTERRUPT_PIN_2 D6 // Tombol 2 (GPIO12)
#define INTERRUPT_PIN_3 D7 // Tombol 3 (GPIO13)

// Variabel untuk menampung state interrupt
volatile bool interruptFlag1 = false;
volatile bool interruptFlag2 = false;
volatile bool interruptFlag3 = false;

// Setup stepper dengan menggunakan AccelStepper
AccelStepper stepper(AccelStepper::DRIVER, PULSE_PIN, DIR_PIN);

// Fungsi interrupt untuk tombol 1 (gerakkan ke kanan)
void ICACHE_RAM_ATTR handleInterrupt1()
{
    interruptFlag1 = true;
}

// Fungsi interrupt untuk tombol 2 (gerakkan ke kiri)
void ICACHE_RAM_ATTR handleInterrupt2()
{
    interruptFlag2 = true;
}

// Fungsi interrupt untuk tombol 3 (hentikan motor)
void ICACHE_RAM_ATTR handleInterrupt3()
{
    interruptFlag3 = true;
}

void setup()
{
    // Setup pin untuk Enable driver
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW); // Aktifkan driver (LOW = aktif)

    // Setup pin untuk input interrupt
    pinMode(INTERRUPT_PIN_1, INPUT_PULLUP);
    pinMode(INTERRUPT_PIN_2, INPUT_PULLUP);
    pinMode(INTERRUPT_PIN_3, INPUT_PULLUP);

    // Attach interrupt ke masing-masing pin
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_1), handleInterrupt1, FALLING);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_2), handleInterrupt2, FALLING);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_3), handleInterrupt3, FALLING);

    // Konfigurasi motor stepper dengan AccelStepper
    stepper.setMaxSpeed(1000);    // Set kecepatan maksimum (langkah per detik)
    stepper.setAcceleration(500); // Set akselerasi (langkah per detik kuadrat)

    Serial.begin(115200);
}

void loop()
{
    // Jika tombol 1 ditekan, gerakkan motor ke kanan
    if (interruptFlag1)
    {
        interruptFlag1 = false; // Reset flag interrupt
        Serial.println("Tombol 1 ditekan: Motor ke kanan");
        stepper.move(1000); // Gerakkan motor 1000 langkah ke kanan
    }

    // Jika tombol 2 ditekan, gerakkan motor ke kiri
    if (interruptFlag2)
    {
        interruptFlag2 = false; // Reset flag interrupt
        Serial.println("Tombol 2 ditekan: Motor ke kiri");
        stepper.move(-1000); // Gerakkan motor 1000 langkah ke kiri
    }

    // Jika tombol 3 ditekan, berhentikan motor
    if (interruptFlag3)
    {
        interruptFlag3 = false; // Reset flag interrupt
        Serial.println("Tombol 3 ditekan: Motor berhenti");
        stepper.stop(); // Berhentikan motor
    }

    // Jalankan motor jika ada perintah pergerakan
    stepper.run();
}
