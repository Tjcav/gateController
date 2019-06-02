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
#define EMERGENCY_STOP 2
#define OPEN_CONTACT 3
#define CLOSE_CONTACT 4
#define OPEN_CLOSE_BTN 5
#define OPEN_FOB 5 //should be 6 changed for debug
#define CLOSE_FOB 7
#define OPEN_CLOSE_ZWAVE 8
#define OBSTACLE_DETECTED 9
#define BUZZER 10
#define MOTOR_OPEN 11
#define MOTOR_CLOSE 12

//debug pinouts ******************** DEBUG *************
#define MODE_OPEN_DEBUG 0
#define MODE_CLOSE_DEBUG 1
#define MODE_TRANSITIONING_DEBUG 6

//io data
volatile boolean buttonRequest;
volatile boolean zWaveRequest;
volatile boolean keyFobOpenRequest;
volatile boolean keyFobCloseRequest;
volatile boolean estopCommand;
volatile boolean objectDetected;

//save the state of buttons to detect traling edge
boolean buttonPV = false;


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
    } else if (newCommand == CLOSE_CMD) {
      closeGate();
    } else if (newCommand == OPEN_CMD) {
      openGate();
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
      if (monitorButton() || monitorZWave() || monitorKeyFobClose() || monitorKeyFobOpen()) {
        return STOP_CMD;
      }
      break;
    case CLOSE_CMD: 
      if (monitorButton() || monitorZWave() || monitorKeyFobClose() || monitorKeyFobOpen()) {
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
  return false;
}

boolean monitorZWave() {
  boolean zWaveSignalRcvd = false;
  //check for press and depress
  while (!digitalRead(OPEN_CLOSE_ZWAVE)) {
    zWaveSignalRcvd = true;
    //todo: this assumes there is always a depress....
    digitalWrite(BUZZER, HIGH);
    delay(10);
  }
  digitalWrite(BUZZER, LOW);
  return zWaveSignalRcvd;
}

boolean monitorKeyFobOpen() {
  //todo: i think the fob module can be configured to send a pulse. should probably do that
  boolean buttonPressed = false;
  //check for press and depress
  while (!digitalRead(OPEN_FOB)) {
    buttonPressed = true;
    //todo: this assumes there is always a depress....
    digitalWrite(BUZZER, HIGH);
    delay(10);
  }
  digitalWrite(BUZZER, LOW);
  return buttonPressed;
}

boolean monitorKeyFobClose() {
  boolean buttonPressed = false;
  //check for press and depress
  while (!digitalRead(CLOSE_FOB)) {
    buttonPressed = true;
    //todo: this assumes there is always a depress....
    digitalWrite(BUZZER, HIGH);
    delay(10);
  }
  digitalWrite(BUZZER, LOW);
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
  boolean openContact = !digitalRead(OPEN_CONTACT);
  boolean closeContact = !digitalRead(CLOSE_CONTACT);
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