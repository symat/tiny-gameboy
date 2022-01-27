

// these values are updated by the read_inputs function
uint16_t adc3_btn1_y = 0;
uint16_t adc4_btn2_x = 0;
uint16_t adc5_btn3 = 0;

// these values are updated by the update_state() function based on the adc values
uint8_t button1 = 0;
uint8_t button2 = 0;
uint8_t button3 = 0;
int8_t joystick_x = 0;
int8_t joystick_y = 0;

// you can tune / calibrate the joystick by changing these values
#define MAX_X_ADC 688    
#define MIN_X_ADC 512
#define MIN_Y_ADC 515
#define MAX_Y_ADC 682
#define ORIGO_X 616
#define ORIGO_Y 616

#define MAX_X_ADC_BTN 412    
#define MIN_X_ADC_BTN 341
#define MIN_Y_ADC_BTN 343
#define MAX_Y_ADC_BTN 410
#define ORIGO_X_BTN 385
#define ORIGO_Y_BTN 385

#define BTN_3_PROGRAMMER_THRESHOLD 800   // when programmer is connected, reset ADC is lower (extra pull-down resistor?)
#define BTN_3_THRESHOLD_PRG 760
#define BTN_3_THRESHOLD_NO_PRG 980



void change_gameboy_input_channel(uint8_t pin) {
  // REFS[2..0 ]= 0    : VCC used as voltage reference (disconnected from AREF/PB0)
  // ADLAR = 0         : Left-adjust the results
  // MUX[3..0] = 0011  : single ended input, 0011 for PB3, 0010 for PB4 and 0000 for PB5
  if(pin == PB3) {
    ADMUX = 0b00000011;
  } else if(pin == PB4) {
    ADMUX = 0b00000010;
  } else {
    ADMUX = 0b00000000;
  }
}


void init_gameboy_inputs() {  
  change_gameboy_input_channel(PB3); 
  
  // ADCSRA bits:
  //    ADEN  = 1        : ADC enabled
  //    ADSC  = 1        : ADC conversion started
  //    ADATE = 0        : no auto-trigger
  //    ADIF  = 0        : interrupt flag (set to 1 when ADC completes)
  //    ADIE  = 0        : interrupt disabled
  //    ADPS[2:0] = 000  : ADC prescaler select, use the smallest ADC division
  ADCSRA = 0b10000000;
}

void wait_for_adc() {
  while((ADCSRA & (1 << ADSC)) > 0) ;
}

void read_adc_results(uint16_t* adc) {
  uint8_t adc_low8bit = ADCL;
  *adc = ((ADCH<<8)|adc_low8bit) % 1024; 
}


// mapping from -50...50 measurement to -2..2 values 
// (+/-2 means hard push, +/-1 means medium push, 0 means origo)
int8_t map_to_logical_joystick_value(int8_t scaled) {
  int8_t abs_scaled = abs(scaled);
  if(abs_scaled < 2) return 0;
  if(abs_scaled < 48) return scaled < 0 ? -1 : 1;
  return scaled < 0 ? -2 : 2;
}

// scale ADC measures between -50..0..50
int8_t scale_measures(int16_t *adc, int16_t min_adc, int16_t max_adc, int16_t origo_adc) {
  if(*adc < origo_adc) return map(*adc, min_adc, origo_adc, -50, 0);
  return map(*adc, origo_adc, max_adc, 0, 50);
}

void update_state() {
  int8_t scaled_x = 0;
  int8_t scaled_y = 0;
  
  button1 = adc3_btn1_y < 500 ? 1 : 0;
  button2 = adc4_btn2_x < 500 ? 1 : 0;
  if(adc5_btn3 < BTN_3_PROGRAMMER_THRESHOLD) {
    button3 = adc5_btn3 < BTN_3_THRESHOLD_PRG ? 1 : 0;
  } else {
    button3 = adc5_btn3 < BTN_3_THRESHOLD_NO_PRG ? 1 : 0;
  }
  
  // first scale between -50 ... 50
  if(button1) scaled_y = scale_measures(&adc3_btn1_y, MIN_Y_ADC_BTN, MAX_Y_ADC_BTN, ORIGO_Y_BTN);
  else scaled_y =  scale_measures(&adc3_btn1_y, MIN_Y_ADC, MAX_Y_ADC, ORIGO_Y);
  
  if(button2) scaled_x = scale_measures(&adc4_btn2_x, MIN_X_ADC_BTN, MAX_X_ADC_BTN, ORIGO_X_BTN);
  else scaled_x =  scale_measures(&adc4_btn2_x, MIN_X_ADC, MAX_X_ADC, ORIGO_X);

  // we rotated the joystick, so need to negate the values
  scaled_x *= -1;
  scaled_y *= -1;

  joystick_x = map_to_logical_joystick_value(scaled_x);
  joystick_y = map_to_logical_joystick_value(scaled_y);
}


void read_inputs(uint8_t pin) {
  wait_for_adc();
  if(pin == PB3) {
    read_adc_results(&adc3_btn1_y);
    change_gameboy_input_channel(PB4); 
  } else if(pin == PB4) {
    read_adc_results(&adc4_btn2_x);
    change_gameboy_input_channel(PB5); 
  } else {
    read_adc_results(&adc5_btn3);
    change_gameboy_input_channel(PB3); 
  }
  update_state();
  ADCSRA |= (1 << ADSC); // start the ADC conversion
}
