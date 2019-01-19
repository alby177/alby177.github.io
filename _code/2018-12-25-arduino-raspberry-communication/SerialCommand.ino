int LEDPIN = 13;          // Led pin
char receivedChar = "";   // Character received through serial
String command = "";      // Command string

void setup() {
  // Set the led pin as output  
  pinMode(LEDPIN, OUTPUT);

  // Set up serial communication
  Serial.begin(9600);
}

void loop() {
  
  // Check for available serial data
  if (Serial.available())
  { 
    // Read serial payload
    receivedChar = Serial.read();

    // Store received command character
    command.concat(receivedChar);
  }
  
  // Check for string terminator
  if (receivedChar == '\n' && command != "")
  { 
    // Check for On command
    if (command == "On\n")
    {
      // Light up the led
      digitalWrite(LEDPIN, true);
  
      // Reset command string
      command = "";
    }
  
    // Check for Off command
    else if (command == "Off\n")
    {
      // Light off the led
      digitalWrite(LEDPIN, false);
  
      // Reset command string
      command = "";
    }
    else
    {
      // Command unknown
      Serial.print("Command Unknown\n");

      // Reset command string
      command = "";
    }
  }
}
