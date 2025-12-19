#include <EEPROM.h>
#include <Servo.h>

// Joystick pins
const int X_PIN = A0;
const int Y_PIN = A1;
const int SWITCH_PIN = 2;

// Servo pins
const int X_SERVO_PIN = 6;
const int Y_SERVO_PIN = 5;

Servo xServo; // Servo moving mirrors on x-axis
Servo yServo; // Servo moving mirrors on y-axis

// Joystick values
int xVal, yVal, switchVal;
int xStaticMin = 510;
int xStaticMax = 540;
int yStaticMin = 520;
int yStaticMax = 550;
int lastSwitch = 1;

// y - axis servo
float centralPosY = 40; // Central position (moving 12.5 degrees in each direction)
float posY = 40;
// Boundaries
const float minPosY = 27.5;
const float maxPosY = 52.5;
const float deltaY = 0.25; // Positional shift (for smooth servo movements)

// x - axis servo
float centralPosX = 20; // central position (moving 20 degrees in each direction)
float posX = 20;
// Boundaries
const float minPosX = 0;
const float maxPosX = 40;
const float deltaX = 0.3; // Positional shift (for smooth servo movements)

// smoothing
const float delta = 0.5;

// Auto sleep functionality
bool isActive = true;
bool moving = false;
float targetX = 20, targetY = 40;
unsigned long lastMoveTime = 0;
const unsigned long TIMEOUT = 10000;

// delay
int dt = 30;

const int ADDR_X = 0;
const int ADDR_Y = sizeof(posX);

// Function for getting joystick values
void getJoystickValue()
{
    xVal = analogRead(X_PIN);
    yVal = analogRead(Y_PIN);
    switchVal = digitalRead(SWITCH_PIN);
}

// Function for updating servo positions
void updateServo()
{
    if (moving)
        return;

    bool moved = false;

    // X-axis movement
    if (xVal < xStaticMin)
    {
        posX -= deltaX;
        moved = true;
    }
    else if (xVal > xStaticMax)
    {
        posX += deltaX;
        moved = true;
    }

    if (posX < minPosX)
        posX = minPosX;
    if (posX > maxPosX)
        posX = maxPosX;

    // Y-axis movement
    if (yVal < yStaticMin)
    {
        posY += deltaY;
        moved = true;
    }
    else if (yVal > yStaticMax)
    {
        posY -= deltaY;
        moved = true;
    }

    if (posY < minPosY)
        posY = minPosY;
    if (posY > maxPosY)
        posY = maxPosY;

    if (moved)
    {
        lastMoveTime = millis();
        Serial.print(posX);
        Serial.print(" ");
        Serial.println(posY);
    }
}

void moveToCentralPosition(float targetX, float targetY)
{
    if (!moving || !isActive)
        return;

    bool doneX = abs(posX - targetX) < 0.3;
    bool doneY = abs(posY - targetY) < 0.3;

    if (!doneX)
    {
        if (posX < targetX)
            posX = posX + delta;
        else if (posX > targetX)
            posX = posX - delta;
    }

    if (!doneY)
    {
        if (posY < targetY)
            posY = posY + delta;
        else if (posY > targetY)
            posY = posY - delta;
    }

    // Checking if values are crossing the boundaries
    if (posX <= minPosX)
        posX = minPosX;
    else if (posX >= maxPosX)
        posX = maxPosX;

    if (posY <= minPosY)
        posY = minPosY;
    else if (posY >= maxPosY)
        posY = maxPosY;

    if (doneX && doneY)
    {
        posX = targetX;
        posY = targetY;
        Serial.print(posX);
        Serial.print(" ");
        Serial.println(posY);
        moving = false;
    }
}

void checkActivity()
{
    // ako je prošlo više od TIMEOUT i serva su aktivna → sleep
    if (isActive && millis() - lastMoveTime > TIMEOUT)
    {
        xServo.detach();
        yServo.detach();
        isActive = false;
    }

    // ako su serva neaktivna, a joystick se pomakne ili switch pritisne → wake up
    if (!isActive &&
        (xVal < xStaticMin || xVal > xStaticMax ||
         yVal < yStaticMin || yVal > yStaticMax ||
         switchVal == 0))
    {

        xServo.attach(X_SERVO_PIN);
        yServo.attach(Y_SERVO_PIN);
        isActive = true;
        lastMoveTime = millis();
    }
}

void setup()
{
    Serial.begin(115200);

    xServo.attach(X_SERVO_PIN);
    yServo.attach(Y_SERVO_PIN);

    pinMode(X_PIN, INPUT);
    pinMode(Y_PIN, INPUT);
    pinMode(SWITCH_PIN, INPUT_PULLUP);

    Serial.println("READY");

    float storedX, storedY;

    EEPROM.get(ADDR_X, storedX);
    EEPROM.get(ADDR_Y, storedY);

    if (isnan(storedX) || isnan(storedY))
    {
        posX = centralPosX;
        posY = centralPosY;
        EEPROM.put(ADDR_X, posX);
        EEPROM.put(ADDR_Y, posY);
    }
    else
    {
        posX = storedX;
        posY = storedY;
    }

    lastMoveTime = millis();
}

void loop()
{
    if (Serial.available())
    {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.startsWith("INIT"))
        {
            if (cmd.length() == 4)
            {
                EEPROM.get(ADDR_X, posX);
                EEPROM.get(ADDR_Y, posY);
            }
            else
            {
                // INIT posX posY → postavi iz poslanih vrijednosti
                float x, y;
                int space1 = cmd.indexOf(' ');
                int space2 = cmd.indexOf(' ', space1 + 1);
                x = cmd.substring(space1 + 1, space2).toFloat();
                y = cmd.substring(space2 + 1).toFloat();
                // posX = x;
                // posY = y;
                moving = true;
                targetX = x;
                targetY = y;
            }
            if (!isActive)
            {
                xServo.attach(X_SERVO_PIN);
                yServo.attach(Y_SERVO_PIN);
                isActive = true;
            }
            lastMoveTime = millis();
        }

        if (cmd.startsWith("DELETE"))
        {
            moving = true;
            targetX = centralPosX;
            targetY = centralPosY;
            if (!isActive)
            {
                xServo.attach(X_SERVO_PIN);
                yServo.attach(Y_SERVO_PIN);
                isActive = true;
            }
            lastMoveTime = millis();
        }
        if (cmd.startsWith("SAVE"))
        {
            EEPROM.put(ADDR_X, posX);
            EEPROM.put(ADDR_Y, posY);
        }
    }
    // Getting joystick values
    getJoystickValue();
    checkActivity();

    // If joystick is clicked - mirror in central position
    if (lastSwitch == 0 && switchVal == 1)
    {
        moving = true;
        targetX = centralPosX;
        targetY = centralPosY;
    }

    updateServo();
    moveToCentralPosition(targetX, targetY);

    if (isActive)
    {
        xServo.write(posX);
        yServo.write(posY);
    }

    delay(dt);
}