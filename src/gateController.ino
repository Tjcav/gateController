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

#define OPEN_CONTACT 00
#define CLOSE_CONTACT 01

void setup() {
//setup the pin modes
  pinMode(OPEN_CONTACT, INPUT);
  pinMode(CLOSE_CONTACT, INPUT);

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
  // put your main code here, to run repeatedly:

}

void updateGateState() {
  //determine current gate state
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