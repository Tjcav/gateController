enum GateState {opened, closed, transitioning, inop};
enum GateMode {open, close, opening, closing, stopped};

//the controller's mode
GateMode gateMode = stopped;

#define OPEN_CONTACT 00
#define CLOSE_CONTACT 01

void setup() {
//setup the pin modes
  pinMode(OPEN_CONTACT, INPUT);
  pinMode(CLOSE_CONTACT, INPUT);

  //find out what the gate state is
  GateState gateState = updateGateState();
  //set the initial gate mode
  switch (gateState) {
    case opened:
    {
      gateMode = open;
      break;
    }
    case closed:
    {
      gateMode = close;
      break;
    }
    case transitioning:
    {
      //todo: this is an error, should not occur
      break;
    }
    case inop:
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

GateState updateGateState() {
  //determine current gate state
  GateState gateState;
  boolean openContact = digitalRead(OPEN_CONTACT);
  boolean closeContact = digitalRead(CLOSE_CONTACT);
  if (openContact && closeContact) {
      gateState = inop;
  } else if (openContact) {
      gateState = opened;
  } else if (closeContact) {
      gateState = closed;
  } else {
      gateState = transitioning;
  }
  return gateState;
}