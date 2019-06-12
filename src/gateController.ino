//gate positions
#define OPENED 0
#define CLOSED 1
#define TRANSITIONING 2
#define UNKNOWN 3
int gatePosition = UNKNOWN;

//define controller commands
#define STOP_CMD 0
#define OPEN_CMD 1
#define CLOSE_CMD 2

//define controller i/o pins
#define EMERGENCY_STOP 2    //green
#define OPEN_CONTACT 3      //orange
#define CLOSE_CONTACT 4     //yellow
#define OPEN_CLOSE_BTN 5    //blue
#define OPEN_FOB 6
#define CLOSE_FOB 7
#define OPEN_CLOSE_ZWAVE 8  //purple
#define OBSTACLE_DETECTED 9 //grey
#define BUZZER 10
#define MOTOR_OPEN 11
#define MOTOR_CLOSE 12

//debug pinouts ******************** DEBUG *************
#define MODE_OPEN_DEBUG 0
#define MODE_CLOSE_DEBUG 1
#define MODE_TRANSITIONING_DEBUG 13

//io data
volatile boolean buttonRequest;
volatile boolean zWaveRequest;
volatile boolean keyFobOpenRequest;
volatile boolean keyFobCloseRequest;
volatile boolean estopCommand;
volatile boolean objectDetected;

//variables used to detect trailing edge and remove debounce
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
  //beep once to signal program start initialize
  pinMode(BUZZER, OUTPUT);
  tone(BUZZER, 1000, 100);

  //setup the controller i/o pin modes
  pinMode(EMERGENCY_STOP, INPUT_PULLUP);
  pinMode(OPEN_CONTACT, INPUT_PULLUP);
  pinMode(CLOSE_CONTACT, INPUT_PULLUP);
  pinMode(OPEN_CLOSE_BTN, INPUT_PULLUP);
  pinMode(OPEN_FOB, INPUT_PULLUP);
  pinMode(CLOSE_FOB, INPUT_PULLUP);
  pinMode(OPEN_CLOSE_ZWAVE, INPUT_PULLUP);
  pinMode(OBSTACLE_DETECTED, INPUT_PULLUP);
  pinMode(MOTOR_OPEN, OUTPUT);
  pinMode(MOTOR_CLOSE, OUTPUT);

  //setup debug pinouts ******************** DEBUG *************
  pinMode(MODE_OPEN_DEBUG, OUTPUT);
  pinMode(MODE_CLOSE_DEBUG, OUTPUT);
  pinMode(MODE_TRANSITIONING_DEBUG, OUTPUT);

  //find out what the current gate position is
  updateGatePosition();

  //beep once to signal program done initializing
  tone(BUZZER, 1000, 500);
}

void loop() {
  int currentCommand = STOP_CMD;

  while (true) {
    int newCommand = getCommands(currentCommand);

    if (newCommand == STOP_CMD) {
      stopGate();
      currentCommand = STOP_CMD;
    } else if (newCommand == CLOSE_CMD) {
      closeGate();
      currentCommand = CLOSE_CMD;
    } else if (newCommand == OPEN_CMD) {
      openGate();
      currentCommand = OPEN_CMD;
    } 
  }
}

void stopGate() {
  digitalWrite(MOTOR_OPEN, false);
  digitalWrite(MOTOR_CLOSE, false);
}

void openGate() {
  digitalWrite(MOTOR_OPEN, true);
}

void closeGate() {
  digitalWrite(MOTOR_CLOSE, true);
}

int getCommands(int currentMode) {
  //update the gatePosition
  updateGatePosition();

  //check for estop or object detected
  if (monitorEstop() || monitorObjectDetector()) {
    return STOP_CMD;
  }

  switch(currentMode) {
    case OPEN_CMD: 
      if (monitorButton() || monitorZWave() || monitorKeyFobClose() || monitorKeyFobOpen() || gatePosition == OPENED) {
        return STOP_CMD;
      }
      break;
    case CLOSE_CMD: 
      if (monitorButton() || monitorZWave() || monitorKeyFobClose() || monitorKeyFobOpen() || gatePosition == CLOSED) {
        return STOP_CMD;
      }
      break;
    case STOP_CMD:
      if (gatePosition == CLOSED) {
        if (monitorButton() || monitorZWave() || monitorKeyFobOpen()) {
          return OPEN_CMD;
        }
      } else if (gatePosition == OPENED) {
        if (monitorButton() || monitorZWave() || monitorKeyFobClose()) {
          return CLOSE_CMD;
        } 
      } else if (gatePosition == TRANSITIONING) {
        if (monitorButton() || monitorZWave() || monitorKeyFobOpen()) {
          return OPEN_CMD;
        } else if (monitorKeyFobClose()) {
          return CLOSE_CMD;
        }
      } else {
        return STOP_CMD;
      }
      break;
    default:
      return STOP_CMD;
      break;
  }
  return currentMode;
}

boolean monitorButton() {
  boolean currentButtonReading = !digitalRead(OPEN_CLOSE_BTN);
  boolean buttonCommand = LOW;

  // If the switch changed, due to noise or pressing:
  if (currentButtonReading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (currentButtonReading != buttonState) {
      buttonState = currentButtonReading;

      // only send the button command if the new button state is LOW
      if (buttonState == LOW) {
        buttonCommand = HIGH;
      }
    }
  }
  
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = currentButtonReading;
  return buttonCommand;
}

boolean monitorZWave() {
  boolean zWaveSignalRcvd = false;
  //check for press and depress
  while (!digitalRead(OPEN_CLOSE_ZWAVE)) {
    zWaveSignalRcvd = true;
  }
  return zWaveSignalRcvd;
}

boolean monitorKeyFobOpen() {
  //todo: i think the fob module can be configured to send a pulse. should probably do that
  boolean buttonPressed = false;
  //check for press and depress
  while (!digitalRead(OPEN_FOB)) {
    buttonPressed = true;
  }
  return buttonPressed;
}

boolean monitorKeyFobClose() {
  boolean buttonPressed = false;
  //check for press and depress
  while (!digitalRead(CLOSE_FOB)) {
    buttonPressed = true;
  }
  return buttonPressed;
}

boolean monitorObjectDetector() {
  return !digitalRead(OBSTACLE_DETECTED);
}

boolean monitorEstop() {
  return !digitalRead(EMERGENCY_STOP);
}

//determine current gate state
void updateGatePosition() {
  boolean openContact = digitalRead(OPEN_CONTACT);   //sensor is normally closed
  boolean closeContact = digitalRead(CLOSE_CONTACT); //sensor is normally closed
  if (openContact && closeContact) {
      gatePosition = UNKNOWN;
      digitalWrite(MODE_OPEN_DEBUG, HIGH);
      digitalWrite(MODE_CLOSE_DEBUG, HIGH);
      digitalWrite(MODE_TRANSITIONING_DEBUG, HIGH);
  } else if (openContact) {
      gatePosition = OPENED;
      digitalWrite(MODE_OPEN_DEBUG, HIGH);
      digitalWrite(MODE_CLOSE_DEBUG, LOW);
      digitalWrite(MODE_TRANSITIONING_DEBUG, LOW);
  } else if (closeContact) {
      gatePosition = CLOSED;
      digitalWrite(MODE_OPEN_DEBUG, LOW);
      digitalWrite(MODE_CLOSE_DEBUG, HIGH);
      digitalWrite(MODE_TRANSITIONING_DEBUG, LOW);
  } else {
      digitalWrite(MODE_OPEN_DEBUG, LOW);
      digitalWrite(MODE_CLOSE_DEBUG, LOW);
      digitalWrite(MODE_TRANSITIONING_DEBUG, HIGH);
      gatePosition = TRANSITIONING;
  }
}