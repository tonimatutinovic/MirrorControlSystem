#include <EEPROM.h>
#include <Servo.h>

// Joystick input pins
const int X_PIN = A1;
const int Y_PIN = A2;
const int SWITCH_PIN = 2;

// Servo control pins
const int X_SERVO_PIN = 6;
const int Y_SERVO_PIN = 5;

// Servo objects for X and Y mirror movement
Servo xServo;
Servo yServo;

// Current joystick readings
int xVal, yVal, switchVal;

// Current joystick readings
int xStaticMin = 440;
int xStaticMax = 550;
int yStaticMin = 510;
int yStaticMax = 560;
int lastSwitch = 1;

// Temporary lock that prevents joystick input immediately after automatic movement
unsigned long joystickUnlockTime = 0;

// Y-axis servo configuration
const float centralPosY = 40; // Central position (moving 12.5 degrees in each direction)
float posY = 40;
// Boundaries
const float minPosY = 27.5;
const float maxPosY = 52.5;
const float deltaY = 0.25; // Positional shift (for smooth servo movements)

// X-axis servo configuration
const float centralPosX = 20; // central position (moving 20 degrees in each direction)
float posX = 20;
// Boundaries
const float minPosX = 0;
const float maxPosX = 40;
const float deltaX = 0.3; // Positional shift (for smooth servo movements)

// Step size used for smooth automatic movement towards a target position
const float delta = 0.5;

// System state flags
bool isActive = true;
bool moving = false;

// Current automatic movement target
float targetX = 20, targetY = 40;

// Current automatic movement target
unsigned long lastMoveTime = 0;
const unsigned long TIMEOUT = 10000;

// Main loop delay
int dt = 30;

// EEPROM addresses used for storing the last saved mirror position
const int ADDR_X = 0;
const int ADDR_Y = sizeof(posX);

/*
  Reads the specified analog pin multiple times and returns the averaged value.
  This helps reduce joystick noise and improves movement stability.
*/
int readAveragedAnalog(int pin, int samples = 12)
{
    long sum = 0;
    for (int i = 0; i < samples; i++)
    {
        sum += analogRead(pin);
    }
    return sum / samples;
}

/*
  Reads the current joystick X/Y values and switch state.
*/
void getJoystickValue()
{
    xVal = readAveragedAnalog(X_PIN);
    yVal = readAveragedAnalog(Y_PIN);
    switchVal = digitalRead(SWITCH_PIN);
}

/*
  Updates servo positions based on manual joystick input.
  Movement is ignored while the system is performing automatic target movement
  or during the short unlock delay after automatic positioning.
*/
void updateServo()
{
    if (moving || millis() < joystickUnlockTime)
        return;

    bool moved = false;

    // Manual X-axis movement
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

    // Clamp X position to allowed range
    if (posX < minPosX)
        posX = minPosX;
    if (posX > maxPosX)
        posX = maxPosX;

    // Manual Y-axis movement
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

    // Clamp Y position to allowed range
    if (posY < minPosY)
        posY = minPosY;
    if (posY > maxPosY)
        posY = maxPosY;

    // Update activity timer and report position if movement occurred
    if (moved)
    {
        lastMoveTime = millis();
        Serial.print(posX);
        Serial.print(" ");
        Serial.println(posY);
    }
}

/*
  Smoothly moves the mirror towards the given target position.
  Movement stops once both axes reach the target within the defined step size.
*/
void moveToTarget(float targetX, float targetY)
{
    if (!moving || !isActive)
        return;

    bool doneX = false;
    bool doneY = false;

    // Move X-axis towards target
    if (fabs(posX - targetX) <= delta)
    {
        posX = targetX;
        doneX = true;
    }
    else if (posX < targetX)
    {
        posX += delta;
    }
    else
    {
        posX -= delta;
    }

    // Move Y-axis towards target
    if (fabs(posY - targetY) <= delta)
    {
        posY = targetY;
        doneY = true;
    }
    else if (posY < targetY)
    {
        posY += delta;
    }
    else
    {
        posY -= delta;
    }

    // Clamp both axes to valid mechanical limits
    if (posX < minPosX)
        posX = minPosX;
    if (posX > maxPosX)
        posX = maxPosX;
    if (posY < minPosY)
        posY = minPosY;
    if (posY > maxPosY)
        posY = maxPosY;

    // Keep the system awake during automatic movement
    lastMoveTime = millis();

    // Send updated position to the host application
    Serial.print(posX);
    Serial.print(" ");
    Serial.println(posY);

    // Finalize movement once both axes have reached the target
    if (doneX && doneY)
    {
        posX = targetX;
        posY = targetY;
        Serial.print(posX);
        Serial.print(" ");
        Serial.println(posY);
        moving = false;
        // Prevent joystick noise from immediately overriding the target position
        joystickUnlockTime = millis() + 300;
    }
}

/*
  Handles automatic servo sleep after inactivity and wakes the system
  when joystick movement or button input is detected.
*/
void checkActivity()
{
    // Put servos to sleep after inactivity timeout
    if (isActive && millis() - lastMoveTime > TIMEOUT)
    {
        xServo.detach();
        yServo.detach();
        isActive = false;
    }

    // Wake servos on joystick movement or button press
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

/*
  Initializes serial communication, attaches servos, configures I/O pins,
  and restores the last stored mirror position from EEPROM.
*/
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

    // If EEPROM does not contain valid data, initialize with central positions
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

/*
  Main control loop:
  - processes serial commands from the Python GUI
  - reads joystick input
  - handles inactivity and wake-up logic
  - performs manual or automatic mirror movement
*/
void loop()
{
    if (Serial.available())
    {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.startsWith("INIT"))
        {
            // INIT -> restore the last saved EEPROM position
            if (cmd.length() == 4)
            {
                EEPROM.get(ADDR_X, posX);
                EEPROM.get(ADDR_Y, posY);
            }
            else
            {
                // INIT posX posY -> move to a user-defined target position
                float x, y;
                int space1 = cmd.indexOf(' ');
                int space2 = cmd.indexOf(' ', space1 + 1);
                x = cmd.substring(space1 + 1, space2).toFloat();
                y = cmd.substring(space2 + 1).toFloat();
                moving = true;
                targetX = x;
                targetY = y;
            }
            // Wake the system if necessary
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
            // Return mirror to the central position
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
            // Save current mirror position to EEPROM
            EEPROM.put(ADDR_X, posX);
            EEPROM.put(ADDR_Y, posY);
        }
    }

    // Read joystick state and update activity state
    getJoystickValue();
    checkActivity();

    // Button press returns the mirror to the central position
    if (lastSwitch == 1 && switchVal == 0)
    {
        moving = true;
        targetX = centralPosX;
        targetY = centralPosY;
    }

    // Apply manual or automatic movement logic
    updateServo();
    moveToTarget(targetX, targetY);

    // Output current servo positions if the system is active
    if (isActive)
    {
        xServo.write(posX);
        yServo.write(posY);
    }

    // Store current switch state for edge detection in the next loop iteration
    lastSwitch = switchVal;

    delay(dt);
}