
// Lawrence of Arabia, main theme
static const uint16_t main_theme_freq[] PROGMEM = {
  NOTE_D5, NOTE_A4, NOTE_FS4, NOTE_G4, NOTE_AS4, NOTE_CS5,
  NOTE_D5, NOTE_A4, NOTE_FS4, NOTE_G4, NOTE_DS4, NOTE_CS4,
  NOTE_D4, NOTE_A4, NOTE_G4, NOTE_C5, NOTE_AS4, NOTE_A4, NOTE_B4, NOTE_CS5
};

static const uint8_t main_theme_lengths[] PROGMEM = {
  8, 12, 4, 2, 2, 2, 
  8, 12, 4, 2, 2, 2, 
  8, 8, 8, 8, 8, 16, 2, 2
};


static const uint16_t game_over_freq[] PROGMEM = {
  NOTE_D5, NOTE_B4, NOTE_A4, NOTE_G4
};

static const uint8_t game_over_lengths[] PROGMEM = {
  2, 2, 2, 8
};


static const uint16_t food_freq[] PROGMEM = {
  NOTE_D5, NOTE_G5
};

static const uint8_t food_lengths[] PROGMEM = {
  2, 4
};

static const uint16_t collision_freq[] PROGMEM = {
  NOTE_G2, NOTE_G2, NOTE_G2, NOTE_G2
};

static const uint8_t collision_lengths[] PROGMEM = {
  1, 1, 1, 1
};
