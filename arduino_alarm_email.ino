// Arduino Uno + W5100 Ethernet shield

#include <SPI.h>
#include <Ethernet.h>

// set alarm signal pin
const byte alarmPin = 7;
boolean statusCheck = false;

// set w5100 mac. this must be unique on LAN.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x59, 0x67 };
// set ip address
IPAddress ip(192, 168, 1, 100);

char server[] = "mail.smtp2go.com";
int port = 2525; // You can also try using Port Number 25, 8025 or 587.

EthernetClient client;

void setup()
{
  Serial.begin(9600);
  pinMode(alarmPin, INPUT_PULLUP);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  Ethernet.begin(mac, ip);
  delay(2000);
  Serial.println(Ethernet.localIP());
}

void loop()
{
  if (digitalRead(alarmPin) == HIGH && statusCheck == false)
  {
    if (sendEmail()) Serial.println(F("Email sent"));
    else Serial.println(F("Email failed"));
    statusCheck = true;
  }
  else if (digitalRead(alarmPin) == LOW)
  {
    statusCheck = false;
  }
}

byte sendEmail()
{
  byte thisByte = 0;
  byte respCode;

  if (client.connect(server, port) == 1) {
    Serial.println(F("connected"));
  } else {
    Serial.println(F("connection failed"));
    return 0;
  }
  if (!eRcv()) return 0;

  Serial.println(F("Sending hello"));
  //set to arduino ip
  client.println("EHLO 192.168.1.100");
  if (!eRcv()) return 0;

  Serial.println(F("Sending auth login"));
  client.println("auth login");
  if (!eRcv()) return 0;

  Serial.println(F("Sending User"));
  // Change to your base64 encoded user
  client.println(F("base64user"));
  if (!eRcv()) return 0;

  Serial.println(F("Sending Password"));
  // change to your base64 encoded password
  client.println(F("base64pass"));
  if (!eRcv()) return 0;

  // change to your email address (sender)
  Serial.println(F("Sending From"));
  client.println("MAIL From: <arduino@email.com>");
  if (!eRcv()) return 0;

  // change to recipient address
  Serial.println(F("Sending To"));
  client.println("RCPT To: <rcpt1@email.com>");
  if (!eRcv()) return 0;
  Serial.println(F("Sending To"));
  client.println("RCPT To: <rcpt2@email.com>");
  if (!eRcv()) return 0;

  Serial.println(F("Sending DATA"));
  client.println("DATA");
  if (!eRcv()) return 0;

  Serial.println(F("Sending email"));

  // change to your address
  client.println("From: HomeALARM <arduino@email.com>");
  client.println("Subject: Your Subject"); 
  // message - add/remove lines as needed
  client.println("First message line.");
  client.println(); //blank line
  client.println("Third message line.");
  client.println(".");
  if (!eRcv()) return 0;

  Serial.println(F("Sending QUIT"));
  client.println("QUIT");
  if (!eRcv()) return 0;

  client.stop();

  Serial.println(F("disconnected"));

  return 1;
}

byte eRcv()
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  respCode = client.peek();

  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  if (respCode >= '4')
  {
    efail();
    return 0;
  }

  return 1;
}

void efail()
{
  byte thisByte = 0;
  int loopCount = 0;

  client.println(F("QUIT"));

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return;
    }
  }

  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  client.stop();

  Serial.println(F("disconnected"));
}

