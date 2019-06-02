//gate positions
#define OPENED 0
#define CLOSED 1
#define TRANSITIONING 2
#define UNKNOWN 3
int gatePosition = UNKNOWN;

//gate controller modes
#define OPEN 0
#define CLOSE 1
#define OPENING 2
#define CLOSING 3
#define STOPPED 4
int gateMode = STOPPED;

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


void setup() {
  //setup the controller i/o pin modes
  pinMode(EMERGENCY_STOP, INPUT_PULLUP);
  pinMode(OPEN_CONTACT, INPUT_PULLUP);
  pinMode(CLOSE_CONTACT, INPUT_PULLUP);
  pinMode(OPEN_CLOSE_BTN, INPUT_PULLUP);
  pinMode(OPEN_FOB, INPUT_PULLUP);
  pinMode(CLOSE_FOB, INPUT_PULLUP);
  pinMode(OPEN_CLOSE_ZWAVE, INPUT_PULLUP);
  pinMode(OBSTACLE_DETECTED, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  pinMode(MOTOR_OPEN, OUTPUT);
  pinMode(MOTOR_CLOSE, OUTPUT);

  //setup debug pinouts ******************** DEBUG *************
  pinMode(MODE_OPEN_DEBUG, OUTPUT);
  pinMode(MODE_CLOSE_DEBUG, OUTPUT);
  pinMode(MODE_TRANSITIONING_DEBUG, OUTPUT);

  //beep once to signal program start initialize
  tone(BUZZER, 1000, 100);

  //find out what the current gate position is
  updateGatePosition();

  //set the initial gate controller mode
  if (gatePosition == OPENED) {
    gateMode = OPEN;
  } else if (gateMode == CLOSED) {
    gateMode = CLOSE;
  } else if (gatePosition == TRANSITIONING) {
    gateMode = STOPPED;
  } else {
    //todo: if gatePosition is unknown what should happen?
    gateMode = STOPPED;
  }

  //beep once to signal program done initializing
  tone(BUZZER, 1000, 500);
}

void loop() {
  //update the gatePosition
  updateGatePosition();

  //check for user inputs
  boolean buttonPressed = monitorButtonPress();
  boolean zWaveRequest = monitorZWave();
  boolean keyFobOpenRequest = monitorKeyFobOpen();
  boolean keyFobCloseRequest = monitorKeyFobClose();
  boolean estopCommand = monitorEstop(); //todo: should this use an interrupt?

  //check sensors
  boolean objectDetected = monitorObjectDetector(); //todo: should this use an interrupt?

  //run state machine
  switch (gateMode) {
    case OPEN:
    {
      //do mode operations
      stopGate();
      //handle mode transitions
      if (buttonPressed || zWaveRequest || keyFobCloseRequest) {
        gateMode = CLOSING;
      }
      //handle an uncommanded position change
      if (gatePosition == CLOSED) {
        gateMode = CLOSE;
      }
      break;
    }
    case CLOSE:
    {
      //do mode operations
      stopGate();
      //handle mode transitions
      if (buttonPressed || zWaveRequest || keyFobOpenRequest) {
        gateMode = OPENING;
      }
      //handle an uncommanded position change
      if (gatePosition == OPENED) {
        gateMode = OPEN;
      }
      break;
    }
    case OPENING:
    {
      //do mode operations
      openGate();
      //handle mode transitions
      if (buttonPressed || zWaveRequest || keyFobCloseRequest || keyFobOpenRequest || estopCommand || objectDetected) {
        gateMode = STOPPED;
      }
      if (gatePosition == OPEN) {
        gateMode = OPEN;
      }
      break;
    }
    case CLOSING:
    {
      //do mode operations
      closeGate();
      //handle mode transitions
      if (buttonPressed || zWaveRequest || keyFobCloseRequest || keyFobOpenRequest || estopCommand || objectDetected) {
        gateMode = STOPPED;
      }
      if (gatePosition == CLOSE) {
        gateMode = CLOSE;
      }
      break;
    }
    case STOPPED:
    {
      //do mode operations
      stopGate();
      //handle mode transitions
      if (buttonPressed || zWaveRequest || keyFobOpenRequest) {
        gateMode = OPENING;
      } else if (keyFobCloseRequest) {
        gateMode = CLOSING;
      }
      //handle an uncommanded position change
      if (gatePosition == CLOSED) {
        gateMode = CLOSE;
      }
      if (gatePosition == OPENED) {
        gateMode = OPEN;
      }
      break;
    }
    default:
    {
      //todo: should never get here
      stopGate();
      break;
    }
  };

}

boolean monitorButtonPress() {
  boolean buttonPressed = false;
  //check for press and depress
  while (!digitalRead(OPEN_CLOSE_BTN)) {
    buttonPressed = true;
    //todo: this assumes there is always a depress....
    digitalWrite(BUZZER, HIGH);
    delay(10);
  }
  digitalWrite(BUZZER, LOW);
  return buttonPressed;
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

void openGate() {
  digitalWrite(MOTOR_OPEN, true);
}

void closeGate() {
  digitalWrite(MOTOR_CLOSE, true);
}

void stopGate() {
  digitalWrite(MOTOR_OPEN, false);
  digitalWrite(MOTOR_CLOSE, false);
}
