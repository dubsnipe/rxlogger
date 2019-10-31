//This function is used to power up the SD card and initialize it.
bool sd_power_on() {
  //Turn on the power through the transistor, then initialize the
  //SD card afterwards (this initializes the SPI bus and the
  //chip-select line).
  digitalWrite(PIN_SD_PWR, LOW);
  return SD.begin(PIN_SD_CS);
}

//This funcion is used to power down the SD card to save power.
void sd_power_off() {
  //Turn off the power through the transistor, then shut down
  //the SPI bus and the chip-select line on the SD card.
  digitalWrite(PIN_SD_PWR, HIGH);
  pinMode(PIN_SD_CS, INPUT);
  SPI.end();
}

//This function performs temperature and humidity logging.
void do_tem_hum_logging() {
  char txt_buf[64];
  char num_buf[8];
  File file;

  //Turn the LED on, then try to power up and initialize the SD card.
  digitalWrite(PIN_LED, HIGH);
  if (!sd_power_on()) {
    Serial.println(F("Cannot initialize SD card!"));
    goto end;
  }

  //Prepare an string with the date and time columns, then add the temperature and
  //humidity readings.
  get_date_time_str(txt_buf, sizeof(txt_buf));
  strcat(txt_buf, dtostrf(dht.readTemperature(), 5, 1, num_buf));
  strcat_P(txt_buf, PSTR(", "));
  strcat(txt_buf, dtostrf(dht.readHumidity(), 5, 1, num_buf));
  strcat_P(txt_buf, PSTR("\n"));

  //Print a copy of the data row to the serial terminal.
  Serial.print(txt_buf);

  //Open the file for writing, then append the data row.
  file = SD.open((const __FlashStringHelper *) temp_hum_log_file_name, FILE_WRITE);
  if (file) {
    file.print(txt_buf);
    file.close();
  }
  else
    Serial.println(F("Could not write to SD card!"));

end:
  //Turn of the SD card before leaving.
  sd_power_off();
  digitalWrite(PIN_LED, LOW);
}

//This function performs button press logging.
void do_button_logging() {
  char txt_buf[64];
  char num_buf[8];
  int pin_number;
  int button_number;
  File file;

  //Turn the LED on, then try to power up and initialize the SD card.
  digitalWrite(PIN_LED, HIGH);
  if (!sd_power_on()) {
    Serial.println(F("Cannot initialize SD card!"));
    goto end;
  }

  //Get the pin number from which the button interrupt was generated. Then look it up
  //int the pin table to determine its index.
  pin_number = get_last_button_interrupt_pin();
  for (button_number = 0; button_number < num_buttons; button_number++)
    if (button_pins[button_number] == pin_number)
      break;

  //Prepare an string with the date and time columns, then add the button number.
  get_date_time_str(txt_buf, sizeof(txt_buf));
  snprintf_P(num_buf, sizeof(num_buf), PSTR("%d"), button_number);
  strcat(txt_buf, num_buf);
  strcat_P(txt_buf, PSTR("\n"));

  //Print a copy of the data row to the serial terminal.
  Serial.print(txt_buf);

  //Open the file for writing, then append the data row.
  file = SD.open((const __FlashStringHelper *) button_log_file_name, FILE_WRITE);
  if (file) {
    file.print(txt_buf);
    file.close();
  }
  else
    Serial.println(F("Could not write to SD card!"));

end:
  //Turn of the SD card before leaving.
  sd_power_off();
  digitalWrite(PIN_LED, LOW);
}

//This function formats and prints the current date and time to a given string.
void get_date_time_str(char *str, size_t size) {
  snprintf_P(str, size, PSTR("%d/%02d/%02d, %02d:%02d:%02d, "),
             rtc.getYear(), rtc.getMonth(), rtc.getDay(),
             rtc.getHour(), rtc.getMinute(), rtc.getSecond());
}
