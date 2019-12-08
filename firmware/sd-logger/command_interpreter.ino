//This function performs the main commmand loop, while slowly blinking the LED
//to indicate this activity. It doesn't return and requires to press the reset
//button to exit.
void do_command_loop() {
  unsigned long t_last_blink;

  Serial.println(F("Command mode activated. Type 'h' for help."));

  for (;;) {
    int t = millis();

    do_commands();

    if (t - t_last_blink >= 1000) {
      digitalWrite(PIN_LED, !digitalRead(PIN_LED));
      t_last_blink = t;
    }
  }
}

//This function listens for incoming characters and performs user commands.
void do_commands() {
  char c;

  //Check for an incoming character. Return immediately if none is found.
  if (!Serial.available())
    return;

  //Take the firs character as the command.
  c = Serial.read();

  //Take an action depending on the received command.
  switch (c) {
    case 'g':
      //Command 'g' gets the current time and prints it.
      print_date_time();
      break;
    case 'd':
      //Command 'd' sets the current date for the RTC.
      {
        int year = Serial.parseInt();
        uint8_t month = Serial.parseInt();
        uint8_t day = Serial.parseInt();
        RtcDateTime old_dt = rtc.GetDateTime();
        RtcDateTime new_dt(year, month, day, old_dt.Hour(),
                           old_dt.Minute(), old_dt.Second());
        rtc.SetDateTime(new_dt);
        Serial.println(F("Date has been set. New date & time is:"));
        print_date_time();
      }
      break;
    case 't':
      //Command 't' sets the current time for the RTC.
      {
        uint8_t hour = Serial.parseInt();
        uint8_t minute = Serial.parseInt();
        RtcDateTime old_dt = rtc.GetDateTime();
        RtcDateTime new_dt(old_dt.Year(), old_dt.Month(), old_dt.Day(),
                           hour, minute, 0);
        rtc.SetDateTime(new_dt);
        Serial.println(F("Time has been set. New date & time is:"));
        print_date_time();
      }
      break;
    case 'h':
      //Command 'h' prints the help message.
      Serial.println(F(
                       "Available commands:\n"
                       "g - Get current date & time\n"
                       "d YYYY/MM/DD - Set current date\n"
                       "t HH:MM - Set current time\n"
                       "h - shows this help"
                     ));
      break;
  }
}

//This function formats and prints the current date and time.
void print_date_time() {
  char txt_buf[32];
  RtcDateTime dt = rtc.GetDateTime();
  snprintf_P(txt_buf, sizeof(txt_buf), PSTR("%d/%02d/%02d %02d:%02d:%02d"),
             dt.Year(), dt.Month(), dt.Day(),
             dt.Hour(), dt.Minute(), dt.Second());
  Serial.println(txt_buf);
}
