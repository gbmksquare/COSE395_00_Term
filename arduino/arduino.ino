#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define True 1
#define False 0
#define ON 255
#define OFF 0
#define Active True
#define Inactive False

#define IR_NONE 0
#define IR_UP 16575
#define IR_DOWN 24735
#define IR_LEFT 49215
#define IR_RIGHT 41055
#define IR_CENTER 8415
#define IR_B 255
#define IR_V -32641
#define IR_HOLD -1

// ---------- Declaration ----------
int N;

// Pin
int cdsPins[4] = {A0, A1, A2, A3};
int ledPins[4] = {11, 10, 9, 6};
int IR_pin = 8;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// IR
IRrecv irrecv(IR_pin);
int currentHoldingButton = IR_NONE;

// Game settings
int maximumConcurrentTarget = 2;
unsigned long targetDuration = 3;
int brightnessThreshold = 120;
unsigned long gameMaxTime = 60;

// Game
int isGameInProgress = False;
unsigned long gameStartTime = 0;
int score = 0;

int target_key_match[4] = {IR_RIGHT, IR_CENTER, IR_UP, IR_LEFT};
int targetStatus[4] = {Inactive, Inactive, Inactive, Inactive};
unsigned long targetActivatedTime[4] = {0, 0, 0, 0};
int targetCount = 0;

// -------------------- Declaration --------------------
void startGame();
void endGame();
void handleGame();
void addRandomTarget();
int detectHitTarget(int pin);

void configure();
void configurePinMode();

void increaseScore();
void resetScore();
String scoreInString();
void printCurrentScore();

int getBrightness(int pin);

void turnOnLed(int pin);
void turnOffLed(int pin);

void configureIr();
int receiveIrSignal();
void handleIrSignal(int code);

void configureLcd();
void printLcd(int line, String string);
void clearLcd();

// -------------------- Main --------------------
void setup() {
	configure();
	configurePinMode();
	configureIr();
	configureLcd();
}

void loop() {
	unsigned long currentTime = millis();

	// Get IR signal
	int code = receiveIrSignal();
	handleIrSignal(code);

	// Game running
	if (isGameInProgress == true) {
		handleGame(currentTime);
		if (currentTime > gameStartTime && currentTime - gameStartTime > gameMaxTime * 1000.0) {
			endGame();
		}
	}
}

// -------------------- Setup --------------------
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

// -------------------- Game --------------------
void startGame() {
	Serial.println("Game starting!");
	clearLcd();
	printLcd(0, "Welcome!");
	printLcd(1, "Starting game...");

	isGameInProgress = True;
	resetScore();
	gameStartTime = millis();
}

void endGame() {
	isGameInProgress = False;

	for(int i = 0; i < N; i++) {
		turnOffLed(ledPins[i]);
	}

	Serial.println("Game ended!");
	Serial.print("You scored ");
	Serial.print(score);
	Serial.print("!");

	String string = scoreInString();
	clearLcd();
	printLcd(0, "End! You scored:");
	printLcd(1, string);
}

void handleGame(int currentTime) {  
   // Detect if target is hit
   for (int i = 0; i < N; i++){
	   if (targetStatus[i] == Active) {
		   if (detectHitTarget(i)) {
				targetStatus[i] = Inactive;
				targetCount -= 1;
				turnOffLed(ledPins[i]);
				increaseScore();
				clearLcd();
				printCurrentScore();
			}
		}
   }
   
	// Phase out unhit targets 
	for (int i = 0; i < N; i++) {
		if (targetStatus[i] == Active && currentTime - targetActivatedTime[i] > targetDuration * 1000) {
			targetStatus[i] = Inactive;
			targetCount -= 1;
			turnOffLed(ledPins[i]);
		}
   }  

	// Serial.print("HOLDING BUTTON : ");
	// Serial.println(currentHoldingButton);

   // Add a new target if necessary
   if (targetCount < maximumConcurrentTarget) {
	addRandomTarget(currentTime);
   }
}

void addRandomTarget(int currentTime) {
	int target;
	do {
		target = random(0, N);	
	} while(targetStatus[target] == Active);

	targetStatus[target] = Active;
	targetActivatedTime[target] = currentTime;
	targetCount += 1;
	turnOnLed(ledPins[target]);
}

int detectHitTarget(int targetIndex) {
	// by Laser
	if(getBrightness(cdsPins[targetIndex]) > brightnessThreshold) {
		irrecv.resume();
		return True;
	}
  	// by IR
	else if (currentHoldingButton == target_key_match[targetIndex]) {
		currentHoldingButton = IR_NONE;
		return True;
	} else {
		return False;
	}
}

// -------------------- Score --------------------
void increaseScore() {
	score += 1;
}

void resetScore() {
	score = 0;
}

String scoreInString() {
	return String(score);
}

void printCurrentScore() {
	String string = scoreInString();
	printLcd(0, "Score");
	printLcd(1, string);
}

// -------------------- CDS --------------------
int getBrightness(int pin) {
	return analogRead(pin);
}

// -------------------- LED --------------------
void turnOnLed(int pin) {
	digitalWrite(pin, ON);
	// analogWrite(pin, 50);
}

void turnOffLed(int pin) {
	digitalWrite(pin, OFF);
	// analogWrite(pin, 0);
}

// -------------------- IR --------------------
void configureIr() {
	irrecv.enableIRIn();
}

int receiveIrSignal() {
	decode_results results;
	if (irrecv.decode(&results)) {
		int code = results.value & 0xFFFF;
    	irrecv.resume();
		return code;
	} else {
		return IR_NONE;
	}
}

void handleIrSignal(int code) {
	switch (code) {
		case IR_UP: 
		Serial.println("UP");
		currentHoldingButton = IR_UP;
		break;
		case IR_DOWN:
		Serial.println("DOWN");
		currentHoldingButton = IR_DOWN;
		break;
		case IR_LEFT:
		Serial.println("LEFT");
		currentHoldingButton = IR_LEFT;
		break;
		case IR_RIGHT:
		Serial.println("RIGHT");
		currentHoldingButton = IR_RIGHT;
		break;
		case IR_CENTER:
		Serial.println("CENTER");
		currentHoldingButton = IR_CENTER;
		break;
		case IR_B:
		Serial.println("B");
		currentHoldingButton = IR_B;
		if (isGameInProgress == False) {
			startGame();
		}
		break;
		case IR_V:
		Serial.println("V");
		currentHoldingButton = IR_V;
		if (isGameInProgress == True) {
			endGame();
		}
		break;
		case IR_HOLD:
		Serial.println("HOLD");
		break;
		case IR_NONE: // None
		break;
		default: 
		Serial.print("Unknown code : ");
		Serial.println(code);
		break;
	}
}

// -------------------- LCD --------------------
void configureLcd() {
	lcd.init();
	lcd.backlight();
}

void printLcd(int line, String string) {
	lcd.setCursor(0, line);
	lcd.print(string);
}

void clearLcd() {
	lcd.clear();
}
