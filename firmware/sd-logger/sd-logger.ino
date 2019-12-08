//Dependencies.
//-------------

//Arduino IDE version: 1.8.10
//Arduino AVR boards version: 1.8.1
//Board: Arduino Pro or Pro Mini
//Processor: ATMega328P, 3.3V, 8MHz

//Built-in libraries.
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <SPI.h>

//SD by Arduino, Sparkfun version 1.2.4
#include <SD.h>

//DHT sensor library by Adafruit version 1.3.7
//(Requires Adafruit unified sensor by Adafruit version 1.0.3)
#include <DHT.h>

//Rtc by Makuna version=2.3.3
#include <ThreeWire.h>
#include <RtcDS1302.h>

//Program configurations.
//-----------------------

//Logging interval for temperature/humidity sensor (in seconds).
const int log_interval = 15;

//Timeout internal for entering command mode at startup (in seconds).
const int command_timeout = 5;

//Pins used to register user-input events through buttons. Each must map to a valid
//pin-change interrupt source.
const int button_pins[] = {
  6, 5
};
const int num_buttons = sizeof(button_pins) / sizeof(button_pins[0]);

//Filename for the temperature/humidity log.
const char temp_hum_log_file_name[] PROGMEM = "TEMP-HUM.CSV";

//Filename for the button log.
const char button_log_file_name[] PROGMEM = "BUTTONS.CSV";

//Pin configurations.
const int PIN_LED = 8;
const int PIN_SD_PWR = 9;
const int PIN_SD_CS = 10;
const int PIN_DHT = 3;

//Peripheral driver instances.
DHT dht(PIN_DHT, DHT11);
ThreeWire ThreeWireRTC(A4, A5, 7); //RTC pins: IO, SCLK, CE
RtcDS1302<ThreeWire> rtc(ThreeWireRTC);

//Interrupt flags.
bool wdt_flag = false;
bool button_flag = false;

void setup() {
  //Initialize the SD card power pin.
  pinMode(PIN_SD_PWR, OUTPUT);
  digitalWrite(PIN_SD_PWR, HIGH);

  //Initialize the temperature/humidity sensor.
  dht.begin();

  //Initialize the RTC.
  rtc.Begin();

  //Initialize the serial terminal.
  Serial.begin(9600);

  //Initialize the LED pin.
  pinMode(PIN_LED, OUTPUT);

  //Configure the power saving mode.
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  //Configure the WDT for wake-up interrupts every second (approximately).
  MCUSR = 0;                          //Clear all status flags.
  WDTCSR = (1 << WDCE) | (1 << WDE);  //Enable configuration of WDT.
  WDTCSR =                            //Set interrupt mode and 1s period.
    (1 << WDIE) | (1 << WDP2) | (1 << WDP1);
  wdt_reset();                        //Reset the time count.

  //Prepare for button wake-up interrupts.
  for (int i = 0; i < num_buttons; i++) {
    pinMode(button_pins[i], INPUT_PULLUP);
    enable_button_interrupt(button_pins[i]);
  }

  //Perform the initial wait before starting to log.
  do_initial_wait();
}

void loop() {
  static int sleep_count = 0;

  //The processor executes the loop function every time it wakes up.
  //Once every second, the WDT wakes up the processor. Increase the sleep
  //count whenever this happens and perform logging every time the log
  //interval elapses.
  if (wdt_flag) {
    sleep_count++;

    if (sleep_count >= log_interval) {
      do_tem_hum_logging();
      sleep_count = 0;
    }

    wdt_flag = false;
  }

  if (button_flag) {
    do_button_logging();
    button_flag = false;
  }

  Serial.flush();
  sleep_cpu();
}

//Interrupt service request function for the watchdog timer. Since more than
//one source can wake up the processor, set the flag so the main loop takes
//the appropriate action.
ISR(WDT_vect) {
  wdt_flag = true;
}

//This function waits for user input at startup. If user input is detected within
//the command timeout period, it then calls the command loop function (which
//doesn't return). Otherwise this function returns.
void do_initial_wait() {
  unsigned long t_last_second = millis();
  unsigned long t_last_blink = t_last_second;
  uint8_t count_down = command_timeout;

  Serial.println(F("Waiting for commands (press Enter to stop)..."));
  Serial.println(count_down);

  //Wait until a character arrives or the timeout period expires.
  for (;;) {
    unsigned long t = millis();

    //Blink the LED while waiting.
    if (t - t_last_blink >= 100) {
      digitalWrite(PIN_LED, !digitalRead(PIN_LED));
      t_last_blink = t;
    }

    //Check for any incoming character.
    if (Serial.available()) {
      //Flush the character. Then enter the command loop (this never returns and
      //requires reset to exit).
      Serial.read();
      do_command_loop();
    }

    //Print the count down. Leave the loop once the count reaches zero.
    if (t - t_last_second >= 1000) {
      Serial.println(--count_down);
      if (count_down == 0)
        break;
      t_last_second = t;
    }
  }

  //Make sure the LED is off upon leaving.
  digitalWrite(PIN_LED, LOW);
}
