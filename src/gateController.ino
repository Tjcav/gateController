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
#define OPEN_CLOSE_FOB 6
#define OPEN_CLOSE_ZWAVE 7
#define OBSTACLE_DETECTED 8
#define BUZZER 9
#define MOTOR_OPEN 10
#define MOTOR_CLOSE 11

//debug pinouts ******************** DEBUG *************
#define MODE_OPEN_DEBUG 0
#define MODE_CLOSE_DEBUG 1
#define MODE_TRANSITIONING_DEBUG 12


void setup() {
  //setup the controller i/o pin modes
  pinMode(EMERGENCY_STOP, INPUT_PULLUP);
  pinMode(OPEN_CONTACT, INPUT_PULLUP);
  pinMode(CLOSE_CONTACT, INPUT_PULLUP);
  pinMode(OPEN_CLOSE_BTN, INPUT_PULLUP);
  pinMode(OPEN_CLOSE_FOB, INPUT_PULLUP);
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
  switch (gatePosition) {
    case OPENED:
    {
      gateMode = OPEN;
      break;
    }
    case CLOSED:
    {
      gateMode = CLOSE;
      break;
    }
    case TRANSITIONING:
    {
      gateMode = STOPPED;
      break;
    }
    case UNKNOWN:
    {
      //todo: handle this condition. this means a faulty sensor reading
      gateMode = STOPPED;
      break;
    }
    default:
    {
      //todo: should never get here
      break;
    }
  };

  //beep once to signal program done initializing
  tone(BUZZER, 1000, 500);
}

void loop() {
  //update the gatePosition
  updateGatePosition();
  //todo: put button reading in another routine
  boolean emergencyStopRqst = !digitalRead(EMERGENCY_STOP);
  boolean gateModeChangeRqst = !digitalRead(OPEN_CLOSE_BTN) || !digitalRead(OPEN_CLOSE_FOB) || !digitalRead(OPEN_CLOSE_ZWAVE);
  boolean openContact = !digitalRead(OPEN_CONTACT);
  boolean closeContact = !digitalRead(CLOSE_CONTACT);

  //run state machine
  switch (gateMode) {
    case OPEN:
    {
      //do mode operations
      stopGate();
      //handle mode transitions
      if (gateModeChangeRqst) {
        gateMode = CLOSING;
      }
      break;
    }
    case CLOSE:
    {
      //do mode operations
      stopGate();
      //handle mode transitions
      if (gateModeChangeRqst) {
        gateMode = OPENING;
      }
      break;
    }
    case OPENING:
    {
      //do mode operations
      openGate();
      //handle mode transitions
      if (gateModeChangeRqst || emergencyStopRqst) {
        gateMode = STOPPED;
      }
      if (openContact) {
        gateMode = OPEN;
      }
      break;
    }
    case CLOSING:
    {
      //do mode operations
      closeGate();
      //handle mode transitions
      if (gateModeChangeRqst || emergencyStopRqst) {
        gateMode = STOPPED;
      }
      if (closeContact) {
        gateMode = CLOSE;
      }
      break;
    }
    case STOPPED:
    {
      //do mode operations
      stopGate();
      //handle mode transitions
      if (gateModeChangeRqst) {
        gateMode = OPENING;
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
