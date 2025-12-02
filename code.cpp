#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int TMP36_PIN     = A0;   // Hőmérséklet
const int HUMID_POT_PIN = A1;   // Páratartalom potméter
const int BUTTON_PIN    = 2;    // Nyomógomb (INPUT_PULLUP)

const int HALL_POT_PIN  = A2;   // Mágnesesség / "szélerősség" potméter
const int LED_PIN       = 8;    // Figyelmeztető LED
const int MOTOR_PIN     = 9;    // Motor PWM (tranzisztor bázis)

int displayMode = 0;

void readTempAndHumidity(float &temperatureC, int &humidity) {
  // Hőmérséklet TMP36-ból
  int rawTemp = analogRead(TMP36_PIN);
  float voltage = rawTemp * (5.0 / 1023.0);      // 0–5 V
  temperatureC = (voltage - 0.5) * 100.0;        // TMP36: 0.5V = 0°C

  // Páratartalom potméterből (0–100%)
  int rawHum = analogRead(HUMID_POT_PIN);
  humidity = map(rawHum, 0, 1023, 0, 100);
}

// Nyomógomb kezelése – kijelző mód váltása
void handleButtonMode() {
  int buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW) {           // lenyomva
    displayMode = (displayMode + 1) % 4;  // 0-1-2-3-0-...
    delay(300);                      // debounce + egyszeri váltás nyomásonként
  }
}

void readMagneticAndWind(int &magneticPercent, int &motorPWM, int &rawHall) {
  rawHall = analogRead(HALL_POT_PIN);        // 0–1023
  magneticPercent = map(rawHall, 0, 1023, 0, 100);
  motorPWM = map(rawHall, 0, 1023, 0, 255);
}

// Motor vezérlése a kiszámolt PWM alapján
void handleMotor(int motorPWM) {
  analogWrite(MOTOR_PIN, motorPWM);
}

// LED vezérlése a "szélerősség" / mágnesesség alapján
void handleLed(int rawHall) {
  const int LED_THRESHOLD = 700; // 0-1023 tartományban
  if (rawHall > LED_THRESHOLD) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void updateDisplay(float temperatureC, int humidity, int magneticPercent,
                   int motorPWM, int rawHall) {
  const int LED_THRESHOLD = 700;

  lcd.clear();
  switch (displayMode) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Homerseklet:");
      lcd.setCursor(0, 1);
      lcd.print(temperatureC, 1);
      lcd.print((char)223); // fok jel
      lcd.print("C");
      break;

    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Paratartalom:");
      lcd.setCursor(0, 1);
      lcd.print(humidity);
      lcd.print(" %");
      break;

    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Magnesesseg:");
      lcd.setCursor(0, 1);
      lcd.print(magneticPercent);
      lcd.print(" %");
      break;

    case 3:
      lcd.setCursor(0, 0);
      lcd.print("Szelero:");
      lcd.setCursor(0, 1);
      int windPercent = map(motorPWM, 0, 255, 0, 100);
      lcd.print(windPercent);
      lcd.print(" %  ");
      if (rawHall > LED_THRESHOLD) {
        lcd.print("LED ON");
      } else {
        lcd.print("LED OFF");
      }
      break;
  }
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);

  lcd.init();        // I2C LCD inicializálás
  lcd.backlight();   // háttérvilágítás bekapcsolása

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Idojaras allomas");
  lcd.setCursor(0, 1);
  lcd.print("Init...");
  delay(1500);
}

void loop() {
  float temperatureC = 0.0;
  int humidity = 0;
  int magneticPercent = 0;
  int motorPWM = 0;
  int rawHall = 0;

  // 1. rész: hőmérséklet + páratartalom + gomb
  readTempAndHumidity(temperatureC, humidity);
  handleButtonMode();

  // 2. rész: mágnesesség / szél + motor + LED
  readMagneticAndWind(magneticPercent, motorPWM, rawHall);
  handleMotor(motorPWM);
  handleLed(rawHall);

  // Közös LCD frissítés a pillanatnyi displayMode alapján
  updateDisplay(temperatureC, humidity, magneticPercent, motorPWM, rawHall);

  delay(150);  // kicsi késleltetés
}
