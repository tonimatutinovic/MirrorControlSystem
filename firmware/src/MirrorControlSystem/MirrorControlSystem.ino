#include <Servo.h>

// Joystick pins
const int X_PIN = A0;
const int Y_PIN = A1;
const int SWITCH_PIN = 2;

// Servo pins
const int X_SERVO_PIN = 6;
const int Y_SERVO_PIN = 5;

Servo xServo;
Servo yServo;

// Joystick values
int xVal, yVal, switchVal;
int xStaticMin = 510;
int xStaticMax = 540;
int yStaticMin = 520;
int yStaticMax = 550;
int lastSwitch = 1;

// y - axis servo
float centralPosY = 40;
float posY = 40;
const float minPosY = 27.5;
const float maxPosY = 52.5;
const float deltaY = 0.25;

// x - axis servo
float centralPosX = 20;
float posX = 20;
const float minPosX = 0;
const float maxPosX = 40;
const float deltaX = 0.3;

// smoothing
const float delta = 0.5;

// delay
int dt = 30;

// central movement
bool movingToCenter = false;

void getJoystickValue() {
    xVal = analogRead(X_PIN);
    yVal = analogRead(Y_PIN);
    switchVal = digitalRead(SWITCH_PIN);
}

void updateServo() {
    if (!movingToCenter){
        // X-axis movement
        if (xVal < xStaticMin) {
            posX -= deltaX;
        } else if (xVal > xStaticMax) {
            posX += deltaX;
        }

        if (posX < minPosX) posX = minPosX;
        if (posX > maxPosX) posX = maxPosX;

        // Y-axis movement
        if (yVal < yStaticMin) {
            posY += deltaY;
        } else if (yVal > yStaticMax) {
            posY -= deltaY;
        }

        if (posY < minPosY) posY = minPosY;
        if (posY > maxPosY) posY = maxPosY;
    }
}

void moveToCentralPosition() {
    if (!movingToCenter) return;

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

void setup() {
    Serial.begin(115200);

    xServo.attach(X_SERVO_PIN);
    yServo.attach(Y_SERVO_PIN);

    pinMode(X_PIN, INPUT);
    pinMode(Y_PIN, INPUT);
    pinMode(SWITCH_PIN, INPUT_PULLUP);
}

void loop() {
    getJoystickValue();

    // Check for switch press
    if (lastSwitch == 0 && switchVal == 1) {
        movingToCenter = true;
    }
    lastSwitch = switchVal;

    updateServo();
    moveToCentralPosition();

    xServo.write(posX);
    yServo.write(posY);

    delay(dt);
}