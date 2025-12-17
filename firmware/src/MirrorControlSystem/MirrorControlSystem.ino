#include <EEPROM.h>
#include <Servo.h>

// Joystick pins
const int X_PIN = A0;
const int Y_PIN = A1;
const int SWITCH_PIN = 2;

// Servo pins
const int X_SERVO_PIN = 6;
const int Y_SERVO_PIN = 5;

Servo xServo;   // Servo moving mirrors on x-axis
Servo yServo;   // Servo moving mirrors on y-axis

// Joystick values
int xVal, yVal, switchVal;
int xStaticMin = 510;
int xStaticMax = 540;
int yStaticMin = 520;
int yStaticMax = 550;
int lastSwitch = 1;

// y - axis servo
float centralPosY = 40;  // Central position (moving 12.5 degrees in each direction)
float posY = 40;
// Boundaries
const float minPosY = 27.5;
const float maxPosY = 52.5;
const float deltaY = 0.25;  // Positional shift (for smooth servo movements)

// x - axis servo
float centralPosX = 20;  // central position (moving 20 degrees in each direction)
float posX = 20;
// Boundaries
const float minPosX = 0;
const float maxPosX = 40;
const float deltaX = 0.3;  // Positional shift (for smooth servo movements)

// smoothing
const float delta = 0.5;

// Auto sleep functionality
bool isActive = true;
unsigned long lastMoveTime = 0;
const unsigned long TIMEOUT = 10000;

// delay
int dt = 30;

// central movement
bool movingToCenter = false;

const int ADDR_X = 0;
const int ADDR_Y = sizeof(posX);

// Function for getting joystick values
void getJoystickValue() {
    xVal = analogRead(X_PIN);
    yVal = analogRead(Y_PIN);
    switchVal = digitalRead(SWITCH_PIN);
}

// Function for updating servo positions
void updateServo() {
    if (movingToCenter) return;

    bool moved = false;

    // X-axis movement
    if (xVal < xStaticMin) {
        posX -= deltaX;
        moved = true;
    } else if (xVal > xStaticMax) {
        posX += deltaX;
        moved = true;
    }

    if (posX < minPosX) posX = minPosX;
    if (posX > maxPosX) posX = maxPosX;

    // Y-axis movement
    if (yVal < yStaticMin) {
        posY += deltaY;
        moved = true;
    } else if (yVal > yStaticMax) {
        posY -= deltaY;
        moved = true;
    }

    if (posY < minPosY) posY = minPosY;
    if (posY > maxPosY) posY = maxPosY;

    if(moved){
        lastMoveTime = millis();
    }

}

void moveToCentralPosition() {
    if (!movingToCenter) return;

    lastMoveTime = millis();

    // X-axis
    if (abs(posX - centralPosX) > delta) {
        if (posX < centralPosX) posX += delta;
        else posX -= delta;
    } else {
        posX = centralPosX;
    }

    // Y-axis
    if (abs(posY - centralPosY) > delta) {
        if (posY < centralPosY) posY += delta;
        else posY -= delta;
    } else {
        posY = centralPosY;
    }

    // Check if reached center
    if (posX == centralPosX && posY == centralPosY) {
        movingToCenter = false;
    }
}

void checkActivity() {
    // ako je prošlo više od TIMEOUT i serva su aktivna → sleep
    if (isActive && millis() - lastMoveTime > TIMEOUT) {
        EEPROM.put(ADDR_X, posX);
        EEPROM.put(ADDR_Y, posY);

        xServo.detach();
        yServo.detach();
        isActive = false;

        Serial.println("AUTO-SLEEP: position saved, servos detached");
    }

    // ako su serva neaktivna, a joystick se pomakne ili switch pritisne → wake up
    if (!isActive &&
        (xVal < xStaticMin || xVal > xStaticMax ||
         yVal < yStaticMin || yVal > yStaticMax ||
         switchVal == 0)) {

        xServo.attach(X_SERVO_PIN);
        yServo.attach(Y_SERVO_PIN);
        isActive = true;
        lastMoveTime = millis();
        Serial.println("WAKE-UP: servos attached");
    }
}


void setup() {
    Serial.begin(115200);

    xServo.attach(X_SERVO_PIN);
    yServo.attach(Y_SERVO_PIN);

    pinMode(X_PIN, INPUT);
    pinMode(Y_PIN, INPUT);
    pinMode(SWITCH_PIN, INPUT_PULLUP);

    lastMoveTime = millis();

    Serial.println("SYSTEM ACTIVE");

    float storedX, storedY;

    EEPROM.get(ADDR_X, storedX);
    EEPROM.get(ADDR_Y, storedY);

    if (isnan(storedX) || isnan(storedY)) {
        posX = centralPosX;
        posY = centralPosY;
        EEPROM.put(ADDR_X, posX);
        EEPROM.put(ADDR_Y, posY);
        Serial.println("EEPROM INIT: default position");
    } else {
        posX = storedX;
        posY = storedY;
        Serial.println("EEPROM LOAD: position restored");
    }

}

void loop() {
    getJoystickValue();
    checkActivity();

    // Check for switch press
    if (lastSwitch == 0 && switchVal == 1) {
        movingToCenter = true;
        lastMoveTime = millis();
    }
    lastSwitch = switchVal;

    updateServo();
    moveToCentralPosition();

    if(isActive){
        xServo.write(posX);
        yServo.write(posY);
    }

    delay(dt);
}