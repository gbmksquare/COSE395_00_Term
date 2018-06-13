int cdsPin = A0;
int isOn = 0;

#define ON 150
#define OFF 0

void setup() {
	Serial.begin(9600);
	pinMode(cdsPin, OUTPUT);
}

void loop() {
	int value = analogRead(cdsPin);
	Serial.print("cds = ");
	Serial.println(value);
	delay(500);
}

