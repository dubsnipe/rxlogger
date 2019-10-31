//Previous and current pin states.
uint8_t port_state_last[3] = { 0xFF, 0xFF, 0xFF };
uint8_t port_state[3] = { 0xFF, 0xFF, 0xFF };

//Port interrupt flags.
uint8_t port_flag[3] = { 0x00, 0x00, 0x00 };

//This function enables an interrupt for a particular button.
void enable_button_interrupt(uint8_t pin) {
  uint8_t bitmask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin) - 2;

  //Set the pin change mask for the selected bit in the corresponding port.
  switch (port) {
    case 0:   //PORT B
      PCMSK0 |= bitmask;
      break;
    case 1:   //PORT C
      PCMSK1 |= bitmask;
      break;
    case 2:   //PORT D
      PCMSK2 |= bitmask;
      break;
  }

  //Set the pin change interrupt enable for the selected port.
  PCICR |= 1 << port;
}

//This function returns the pin that generated the last button interrupt.
int get_last_button_interrupt_pin() {
  uint8_t i;

  //Check all pins looking for the one that generated the interrupt.
  for (i = 0; i < NUM_DIGITAL_PINS; i++)
    if (port_flag[digitalPinToPort(i) - 2] & digitalPinToBitMask(i))
      break;

  //Interrupt pin was found. Clear all flags now.
  port_flag[0] = 0;
  port_flag[1] = 0;
  port_flag[2] = 0;

  return i;
}

//Interrupt service routine for pin change interrupt request 0 (PORT B).
SIGNAL(PCINT0_vect) {
  uint8_t active_bits;

  //Update the port state while keeping the last one.
  port_state_last[0] = port_state[0];
  port_state[0] = PINB;

  //Mask every bit from current and last state so only bits which have changed
  //from 1 to 0 and that are enabled for interrupts are set.
  active_bits = port_state_last[0] & ~port_state[0] & PCMSK0;

  //If any bit has been activated, store the interrupt flags
  if (active_bits) {
    port_flag[0] = active_bits;
    button_flag = true;
  }
}

//Interrupt service routine for pin change interrupt request 1 (PORT C).
SIGNAL(PCINT1_vect) {
  uint8_t active_bits;

  port_state_last[1] = port_state[1];
  port_state[1] = PINC;

  active_bits = port_state_last[1] & ~port_state[1] & PCMSK1;

  if (active_bits) {
    port_flag[1] = active_bits;
    button_flag = true;
  }
}

//Interrupt service routine for pin change interrupt request 2 (PORT D).
SIGNAL(PCINT2_vect) {
  uint8_t active_bits;

  port_state_last[2] = port_state[2];
  port_state[2] = PIND;

  active_bits = port_state_last[2] & ~port_state[2] & PCMSK2;

  if (active_bits) {
    port_flag[2] = active_bits;
    button_flag = true;
  }
}
