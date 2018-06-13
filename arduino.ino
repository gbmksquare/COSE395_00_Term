
#define True 1
#define False 0
#define ON 255
#define OFF 0
#define Active True
#define Inactive False

int N;

int cdsPins[] = {A0, A1, A2, A3, A4};
int ledPins[] = {13, 12, 11, 10, 9};

int targetStatus[] = {Inactive, Inactive, Inactive, Inactive, Inactive};
int targetActivatedTime[] = {0, 0, 0, 0, 0};
int targetCount = 0;

int maximumConcurrentTarget = 3;
int targetDuration = 5;
int brightnessThreshold = 18;

int score = 0;

void startGame();
void endGame();
void addRandomTarget();
int detectHitTarget(int pin);
void getScore();
void resetScore();
void turnOnLed(int pin);
void turnOffLed(int pin);
int getBrightness(int pin);

// Main
void setup() {
	Serial.begin(9600);
	randomSeed(analogRead(0));

	N = sizeof(cdsPins) / sizeof(cdsPins[0]);

	// Set pin mode
	for(int i = 0; i < N; i++) {
		int currentPin = ledPins[i];
		pinMode(currentPin, OUTPUT);
	}
	
	for(int i = 0; i < N; i++) {
		int currentPin = cdsPins[i];
		pinMode(currentPin, INPUT);
	}

	startGame();
}

void loop() {
	int currentTime = millis();

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

