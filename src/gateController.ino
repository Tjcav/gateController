//gate states
#define OPENED 0
#define CLOSED 1
#define TRANSITIONING 2
#define INOP 3

//gate controller mode
#define OPEN 0
#define CLOSE 1
#define OPENING 2
#define CLOSING 3
#define STOPPED 4

//the controller's mode
int gateMode = STOPPED;
int gateState;

#define EMERGENCY_STOP 0
#define OPEN_CONTACT 1
#define CLOSE_CONTACT 2
#define OPEN_CLOSE_BTN 3
#define OPEN_CLOSE_FOB 4
#define OPEN_CLOSE_ZWAVE 5
#define OBSTACLE_DETECTED 6
#define MOTOR_CLOSE 7
#define MOTOR_OPEN 8
#define BUZZER 9

void setup() {
//setup the pin modes
  pinMode(OPEN_CONTACT, INPUT);
  pinMode(CLOSE_CONTACT, INPUT);
  pinMode(EMERGENCY_STOP, INPUT);
  pinMode(OPEN_CLOSE_BTN, INPUT);
  pinMode(OPEN_CLOSE_FOB, INPUT);
  pinMode(OPEN_CLOSE_ZWAVE, INPUT);
  pinMode(OBSTACLE_DETECTED, INPUT);
  pinMode(MOTOR_CLOSE, OUTPUT);
  pinMode(MOTOR_OPEN, OUTPUT);


  //find out what the gate state is
  updateGateState();
  //set the initial gate mode
  switch (gateState) {
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
      //todo: this is an error, should not occur
      break;
    }
    case INOP:
    {
      //todo: handle this condition. this means a faulty sensor reading
      break;
    }
    default:
    {
      //todo: should never get here
      break;
    }
  };

}

void loop() {
  //update the gateState
  updateGateState();
  //todo: put button reading in another routine
  boolean gateModeChangeRqst = digitalRead(OPEN_CLOSE_BTN) || digitalRead(OPEN_CLOSE_FOB) || digitalRead(OPEN_CLOSE_ZWAVE);
  boolean emergencyStopRqst = digitalRead(EMERGENCY_STOP);
  boolean openContact = digitalRead(OPEN_CONTACT);
  boolean closeContact = digitalRead(CLOSE_CONTACT);

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
      //openGate();
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
void updateGateState() {
  boolean openContact = digitalRead(OPEN_CONTACT);
  boolean closeContact = digitalRead(CLOSE_CONTACT);
  if (openContact && closeContact) {
      gateState = INOP;
  } else if (openContact) {
      gateState = OPENED;
  } else if (closeContact) {
      gateState = CLOSED;
  } else {
      gateState = TRANSITIONING;
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
