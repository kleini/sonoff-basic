#define FW_NAME "Sonoff Basic"
#define FW_VERSION "1.1.1"

#include <Homie.h>

const int PIN_RELAY = 12;
const int PIN_LED = 13;
const int PIN_BUTTON = 0;

unsigned long buttonDownTime = 0;
byte lastButtonState = 1;
byte buttonPressHandled = 0;

HomieNode switchNode("switch", "switch", "switch");

bool switchOnHandler(HomieRange range, String value) {
    if (value != "true" && value != "false") return false;

    bool on = (value == "true");
    digitalWrite(PIN_RELAY, on ? HIGH : LOW);
    switchNode.setProperty("on").send(value);
    Homie.getLogger() << "Switch is " << (on ? "on" : "off") << endl;

    return true;
}

void toggleRelay() {
    bool on = digitalRead(PIN_RELAY) == HIGH;
    digitalWrite(PIN_RELAY, on ? LOW : HIGH);
    switchNode.setProperty("on").send(on ? "false" : "true");
    Homie.getLogger() << "Switch is " << (on ? "off" : "on") << endl;
}

void loopHandler() {
    byte buttonState = digitalRead(PIN_BUTTON);
    if (buttonState != lastButtonState) {
        if (buttonState == LOW) {
            buttonDownTime = millis();
            buttonPressHandled = 0;
        } else {
            unsigned long dt = millis() - buttonDownTime;
            if (dt >= 90 && dt <= 900 && buttonPressHandled == 0) {
                toggleRelay();
                buttonPressHandled = 1;
            }
        }
        lastButtonState = buttonState;
    }
}

void onHomieEvent(const HomieEvent & event) {
  switch (event.type) {
    case HomieEventType::WIFI_DISCONNECTED:
      WiFi.disconnect();
      break;
    default:
      break;
  }
}

void setup() {
    Serial.begin(115200);
    Serial << endl << endl;

    pinMode(PIN_RELAY, OUTPUT);
    pinMode(PIN_BUTTON, INPUT);
    digitalWrite(PIN_RELAY, LOW);

    Homie_setFirmware(FW_NAME, FW_VERSION);
    Homie.setResetTrigger(PIN_BUTTON, LOW, 5000);
    Homie.setLedPin(PIN_LED, LOW);

    switchNode.advertise("on").settable(switchOnHandler);

    Homie.setLoopFunction(loopHandler);
    Homie.onEvent(onHomieEvent);
    Homie.setup();
}

void loop() {
    Homie.loop();
}
