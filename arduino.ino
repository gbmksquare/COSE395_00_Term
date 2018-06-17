#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define True 1
#define False 0
#define ON 255
#define OFF 0
#define Active True
#define Inactive False

#define IR_UNIDENTIFIED 0
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
int cdsPins[] = {A0, A1, A2, A3};
int ledPins[] = {11, 10, 9, 6};
int IR_pin = 8;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// IR
IRrecv irrecv(IR_pin);

// Game settings
int maximumConcurrentTarget = 3;
int targetDuration = 5;
int brightnessThreshold = 18;
int gameMaxTime = 60;

// Game
int isGameInProgress = False;
int gameStartTime = 0;
int score = 0;

int targetStatus[] = {Inactive, Inactive, Inactive, Inactive, Inactive};
int targetActivatedTime[] = {0, 0, 0, 0, 0};
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
	// Get IR signal
	int code = receiveIrSignal();
	handleIrSignal(code);

	// End game after some time
	if (gameStartTime - millis() > gameMaxTime*1000.0) {
		endGame();
	}

	// Game running
	if (isGameInProgress == true) {
		handleGame(code);
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

	Serial.println("Game ended!");
	Serial.print("You scored ");
	Serial.print(score);
	Serial.print("!");

	String string = scoreInString();
	clearLcd();
	printLcd(0, "End! You scored:");
	printLcd(1, string);
}

void handleGame(int irCode) {

	int currentTime = millis();
  
   // Detect if target is hit
   for (int i=0; i<N; i++){
    if (targetStatus[i] == Active) {
      if (detectHitTarget(i)) {
        targetStatus[i] = Inactive;
        targetCount--;
        turnOffLed(ledPins[i]);
        increaseScore();
      }
    }
   }
   
  // If hit, add new target, add score

   for (int i=0; i<N; i++){
    if (targetStatus[i] == Active && millis() - targetActivatedTime[i] > targetDuration * 1000.0) {
      targetStatus[i] = Inactive;
      targetCount--;
      turnOffLed(ledPins[i]);
    }
   }  

   // Add new target if necessary
   if (targetCount < maximumConcurrentTarget) {
	addRandomTarget();
   }
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

int detectHitTarget(int targetIndex) {
	// return (getBrightness(pin) > brightnessThreshold ) ? True : False;


	int target_key_match[5] ={IR_UP, IR_LEFT, IR_CENTER, IR_RIGHT, IR_DOWN};
  
  if(receiveIrSignal() == target_key_match[targetIndex]) {
    return True;
  }
  else {
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
	// digitalWrite(pin, ON);
	analogWrite(pin, 255);
}

void turnOffLed(int pin) {
	// digitalWrite(pin, OFF);
	analogWrite(pin, 0);
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
		return IR_UNIDENTIFIED;
	}
}

void handleIrSignal(int code) {
	switch (code) {
		case IR_UP: 
		Serial.println("UP");
		break;
		case IR_DOWN:
		Serial.println("DOWN");
		break;
		case IR_LEFT:
		Serial.println("LEFT");
		break;
		case IR_RIGHT:
		Serial.println("RIGHT");
		break;
		case IR_CENTER:
		Serial.println("CENTER");
		break;
		case IR_B:
		Serial.println("B");
		if (isGameInProgress == False) {
			startGame();
		}
		break;
		case IR_V:
		Serial.println("V");
		if (isGameInProgress == True) {
			endGame();
		}
		break;
		case IR_HOLD:
		Serial.println("HOLD");
		break;
		case 0: // None
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
