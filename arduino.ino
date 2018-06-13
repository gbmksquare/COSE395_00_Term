#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define True 1
#define False 0
#define ON 255
#define OFF 0
#define Active True
#define Inactive False

// ---------- Declaration ----------
int N;

// Pin
int cdsPins[] = {A0, A1, A2, A3, A4};
int ledPins[] = {13, 12, 11, 10, 9};
int IR_pin = 8;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// IR
IRrecv irrecv(IR_pin);

int targetStatus[] = {Inactive, Inactive, Inactive, Inactive, Inactive};
int targetActivatedTime[] = {0, 0, 0, 0, 0};
int targetCount = 0;

int maximumConcurrentTarget = 3;
int targetDuration = 5;
int brightnessThreshold = 18;

int score = 0;

void configure();
void configurePinMode();
void configureIr();
void configureLcd();
void startGame();
void endGame();
void addRandomTarget();
int detectHitTarget(int pin);
void getScore();
void resetScore();
void turnOnLed(int pin);
void turnOffLed(int pin);
int getBrightness(int pin);

// ---------- Main ----------
void setup() {
	configure();
	configurePinMode();
	configureIr();
	configrueLcd();
	startGame();
}

void loop() {
	int currentTime = millis();

	getIRSignal();

	
	// Detect if target is hit
	for (int i=0; i<N; i++){
		if (targetStatus[i] == Active) {
			if (detectHitTarget(cdsPins[i])) {
				targetStatus[i] = Inactive;
				targetCount--;
				getScore();
			}
		}
	}
	// If hit, add new target, add score

	for (int i=0; i<N; i++){
		if (targetStatus[i] == Active && millis() - targetActivatedTime[i] > targetDuration) {
			targetStatus[i] = Inactive;
			targetCount--;
		}
	}	

	// Add new target if necessary
	if (targetCount < maximumConcurrentTarget) {
		addRandomTarget();
	}
	
}

// ---------- Setup ----------
void configure() {
	Serial.begin(9600);
	randomSeed(analogRead(0));
	N = sizeof(cdsPins) / sizeof(cdsPins[0]);
}

void configurePinMode() {
	for(int i = 0; i < N; i++) {
		int currentPin = ledPins[i];
		pinMode(currentPin, OUTPUT);
	}
	
	for(int i = 0; i < N; i++) {
		int currentPin = cdsPins[i];
		pinMode(currentPin, INPUT);
	}

	pinMode(IR_pin, INPUT);
}

void configureIr() {
	irrecv.enableIRIn();
}

void configrueLcd() {
	lcd.init();
	lcd.backlight();
}

// ---------- Game ----------
void startGame() {
	Serial.println("Game starting!");
}

void endGame() {
	Serial.println("Game ended!");
	Serial.print("You scored ");
	Serial.print(score);
	Serial.print("!");
}

void addRandomTarget() {
	int target;
	
	do {
		target = random(0, N-1);	
	} while(targetStatus[target] == Active);

	targetStatus[target] = Active;
	targetActivatedTime[target] = millis();
	targetCount++;
	turnOnLed(target);
}

int detectHitTarget(int pin) {
	return (getBrightness(pin) > brightnessThreshold ) ? True : False;
}

// Score
void getScore() {
	score += 1;
}

void resetScore() {
	score = 0;
}

// ---------- Helper ----------
// LED
void turnOnLed(int pin) {
	digitalWrite(pin, ON);
}

void turnOffLed(int pin) {
	digitalWrite(pin, OFF);
}

// CDS
int getBrightness(int pin) {
	return analogRead(pin);
}

// IR
int getIRSignal() {
	// U 16575
	// L 49215
	// R 41055
	// D 24735
	// Center 8415
	// B 255
	// V 32895
	// Press 65535

	decode_results results;
	if (irrecv.decode(&results)) {
		Serial.println(results.value & 0xFFFF);
    	irrecv.resume();
		return 1;
	}
	else {
		return 0;
	}
}

// LCD
void printLcd(int line, char *string) {
	lcd.setCursor(0, line);
	lcd.print(string);
}

void clearLcd() {
	lcd.clear();
}
