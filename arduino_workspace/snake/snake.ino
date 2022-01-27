

// softwareI2C lib
#define I2C_HARDWARE 0
#define I2C_TIMEOUT 10
#define I2C_MAXWAIT 10
#define I2C_PULLUP 1
#define I2C_FASTMODE 1
#define SDA_PORT PORTB
#define SDA_PIN 0
#define SCL_PORT PORTB
#define SCL_PIN 2

// tinyprint lib
#define TP_PRINTLINES 0
#define TP_FLASHSTRINGHELPER 1
#define TP_NUMBERS 1
#define TP_FLOAT 0
#define TP_WINDOWSLINEENDS 0


#include "SH1106Lib.h"
#include "font_3x5.h"


#define RANDOM_SEED_BY_JOYSTICK 1
#define BUTTON_3_ENABLED 0
#include "gameboy_inputs.h"

SH1106Lib display;

#include "image.h"
#include "tones.h"
#include "music.h"


#define RIGHT_PANEL_WIDTH 22

#define NUMBER_OF_LEVELS 6

static const uint8_t level_bricks_x[] PROGMEM = {
  // level 1 - nothing
  // level 2
  5, 15, 5, 15,
  // level 3
  4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 
  // level 4
  7, 7, 7, 7, 7, 7, 13, 13, 13, 13, 13, 13,
  // level 5
  0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 12, 12, 12, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19,
  // level 6
  5, 15, 10, 10, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
};

static const uint8_t level_bricks_y[] PROGMEM = {
  // level 1 - nothing
  // level 2
  3, 3, 9, 9,
  // level 3
  1, 2, 3, 4, 5, 6, 7, 4, 5, 6, 7, 8, 9, 10,
  // level 4
  0, 2, 4, 6, 8, 10, 1, 3, 5, 7, 9, 11,
  // level 5
  6, 6, 6, 6, 6, 6, 6, 6, 5, 4, 2, 1, 0, 11, 10, 9, 7, 6, 5, 5, 5, 5, 5, 5, 5, 5,
  // level 6
  3, 3, 5, 6, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8
};

// where to position in "level_bricks_x/level_bricks_y" to reach the given level
uint8_t level_bricks_offset[] = {
  0, 0, 4, 18, 30, 56, 73
};

uint8_t highscores[NUMBER_OF_LEVELS];


#define SNAKE_MAX_LENGTH 128
#define SNAKE_DIR_LEFT 0
#define SNAKE_DIR_RIGHT 1
#define SNAKE_DIR_UP 2
#define SNAKE_DIR_DOWN 3

#define EEPROM_GAME_ID 100
#define EEPROM_GAME_ID_ADDR 0
#define EEPROM_HIGHSCORE_ADDR 1
#include "eeprom_util.h"

uint8_t snake_length;
uint8_t snake_speed;
uint8_t btn_1_state;
uint8_t mute_btn_state;

uint8_t snake_pos_x[SNAKE_MAX_LENGTH];
uint8_t snake_pos_y[SNAKE_MAX_LENGTH];

uint8_t snake_pos;   // where the snake data starts in the snake_pos arrays 
uint8_t new_snake_dir = SNAKE_DIR_RIGHT;
uint8_t game_over = 0;
const uint8_t max_pos_x = (124 - RIGHT_PANEL_WIDTH) / TILE_WIDTH;  // rightmost tile
const uint8_t max_pos_y = 60 / TILE_HEIGHT;                        // downmost tile
const uint8_t board_height = max_pos_y * TILE_HEIGHT;              // total height of the board
const uint8_t panel_x = (max_pos_x * TILE_WIDTH) + 3;              // where the right panel starts


uint8_t food_x;
uint8_t food_y;
uint8_t current_level = 0;   // 0 .. NUMBER_OF_LEVELS-1


void setup() {
  display.initialize();
  display.clearDisplay();

  display.setFont(font_3x5, FONT_3x5_WIDTH, FONT_3x5_HEIGHT, 45, FONT_NUMBERS | FONT_UPPERCASECHARS);
  display.setTextWrap(false);
  display.setTextColor(WHITE, TRANSPARENT);

  // read / init highscore in EEPROM
  uint8_t eeprom_game_id = EEPROM_read(EEPROM_GAME_ID_ADDR);
  if(eeprom_game_id == EEPROM_GAME_ID) {
    for(int i=0; i<NUMBER_OF_LEVELS; i++) {
      highscores[i] = EEPROM_read(EEPROM_HIGHSCORE_ADDR + i);
    }
  } else { 
    for(int i=0; i<NUMBER_OF_LEVELS; i++) {
      highscores[i] = 3;
      EEPROM_update(EEPROM_HIGHSCORE_ADDR + i, 3);
    }
    EEPROM_update(EEPROM_GAME_ID_ADDR, EEPROM_GAME_ID);
  }
  
  init_game();
}


void loop() {

  // "start" the timer
  unsigned long until = millis() + snake_speed * 150;

  uint8_t old_head = (snake_pos + snake_length - 1) % SNAKE_MAX_LENGTH;
  uint8_t new_head = (old_head + 1) % SNAKE_MAX_LENGTH;
  uint8_t snake_dir = new_snake_dir;
  if(snake_dir == SNAKE_DIR_LEFT || snake_dir == SNAKE_DIR_RIGHT) {
    snake_pos_y[new_head] = snake_pos_y[old_head];
    snake_pos_x[new_head] = snake_pos_x[old_head] + (snake_dir == SNAKE_DIR_RIGHT ? 1 : -1);
  } else {
    snake_pos_x[new_head] = snake_pos_x[old_head];
    snake_pos_y[new_head] = snake_pos_y[old_head] + (snake_dir == SNAKE_DIR_DOWN ? 1 : -1);
  }

  if(collision(snake_pos_x[new_head], snake_pos_y[new_head]) || snake_length == SNAKE_MAX_LENGTH) {
    handle_game_over();
  } else {
    draw_bitmap_tile(snake_pos_x[new_head], snake_pos_y[new_head], snake_tile);
    if(snake_pos_x[new_head] == food_x && snake_pos_y[new_head] == food_y) {
      snake_length++;
      play_tones(food_freq, food_lengths, sizeof(food_lengths));
      new_food();
      update_number(snake_length, (max_pos_x * TILE_WIDTH) + 6, 11, 3); // update score on the screen
    } else {
      draw_empty_tile(snake_pos_x[snake_pos], snake_pos_y[snake_pos]);
      snake_pos = (snake_pos + 1) % SNAKE_MAX_LENGTH;
    }
  }

  while(millis() < until || game_over) {  
    update_tones();

    read_all_inputs();

    if(btn_1_state == 0 && button1) {
      if(game_over) { 
        btn_1_state = button1;
        init_game();
        break;
      } else {
        snake_speed = (snake_speed + 1) % 3 + 1;
        update_speed_icon();
      }
    }        
    btn_1_state = button1;

    handle_mutate_button(); 

    if(joystick_x == -2) {
      if(snake_dir != SNAKE_DIR_RIGHT) new_snake_dir = SNAKE_DIR_LEFT;
    } 
    if(joystick_x == 2) {
      if(snake_dir != SNAKE_DIR_LEFT) new_snake_dir = SNAKE_DIR_RIGHT;
    } 
    if(joystick_y == -2) {
      if(snake_dir != SNAKE_DIR_DOWN) new_snake_dir = SNAKE_DIR_UP;
    } 
    if(joystick_y == 2) {
      if(snake_dir != SNAKE_DIR_UP) new_snake_dir = SNAKE_DIR_DOWN;
    }
    
  }
    
}


void init_game() {
  wellcome_screen();
  display.clearDisplay();
  
 
  // border
  display.fillRect(0, 0, 128, board_height+3, WHITE);
  display.fillRect(1, 1, 126, board_height+1, BLACK);

  // right panel
  display.fillRect(panel_x, 0, 128 - panel_x, board_height+3, WHITE);
  display.fillRect(panel_x + 1, 1, 128 - panel_x - 2, board_height+1, BLACK);
  display.setCursor(panel_x + 3, 2);
  display.print(F("YOU"));
 
  display.setCursor(panel_x + 3, 23);
  display.print(F("HIGH"));

  // bricks
  for(int i=level_bricks_offset[current_level]; i<level_bricks_offset[current_level+1]; i++) {
    draw_bitmap_tile(pgm_read_byte(level_bricks_x+i), pgm_read_byte(level_bricks_y+i), brick_tile);
  }

  // draw "initial" snake
  snake_length = 3;
  snake_speed = 3; // start with the slowest speed
  snake_pos = 0;   
  new_snake_dir = SNAKE_DIR_RIGHT;
  snake_pos_x[0] = 1;
  snake_pos_x[1] = 2;
  snake_pos_x[2] = 3;
  snake_pos_y[0] = 7;
  snake_pos_y[1] = 7;
  snake_pos_y[2] = 7;
  game_over = 0;

  update_number(snake_length, (max_pos_x * TILE_WIDTH) + 6, 11, 3);  // update score number on the display
  update_number(highscores[current_level], (max_pos_x * TILE_WIDTH) + 6, 32, 3);  // update highscore number on the display
  update_speed_icon();
  draw_snake(1);
  new_food();
  update_music_icon();
}

void wellcome_screen() {
  display.clearDisplay();
  display.drawBitmap(4, 4, snake_image,SNAKE_IMAGE_WIDTH, SNAKE_IMAGE_HEIGHT, WHITE, TRANSPARENT);
  
  display.setCursor(80, 10);
  display.print(F("LEVEL"));
  display.drawBitmap(80, 20, speed_icon_full_reverse, SPEED_ICON_WIDTH, SPEED_ICON_HEIGHT, WHITE, TRANSPARENT);
  display.drawBitmap(100, 20, speed_icon_full, SPEED_ICON_WIDTH, SPEED_ICON_HEIGHT, WHITE, TRANSPARENT);

  display.setCursor(80, 35);
  display.print(F("HIGH"));
  update_level_selector();
  update_music_icon();

  int8_t level_selector_state = 0;
  while(1) {
    if(!still_playing()) play_tones(main_theme_freq, main_theme_lengths, sizeof(main_theme_lengths));
    
    read_all_inputs();
    if(level_selector_state != joystick_x && joystick_x != 0) {
      if(joystick_x==-2) current_level = current_level == 0 ? NUMBER_OF_LEVELS - 1 : current_level-1;
      if(joystick_x==2) current_level = current_level == NUMBER_OF_LEVELS - 1 ? 0 : current_level+1;
      update_level_selector();
    }
    level_selector_state = joystick_x;

    if(btn_1_state == 0 && button1) {
      btn_1_state = button1;
      silence();
      break;
    }
    btn_1_state = button1;

    handle_mutate_button();

    update_tones();
    delay(40);
  }
}

void draw_bitmap_tile(uint8_t pos_x, uint8_t pos_y, byte* tile) {
  display.drawBitmap(pos_x * TILE_WIDTH + 2, pos_y * TILE_HEIGHT + 2, tile, TILE_WIDTH, TILE_HEIGHT, WHITE, TRANSPARENT);
}

void draw_empty_tile(uint8_t pos_x, uint8_t pos_y) {
  display.fillRect(pos_x * TILE_WIDTH + 2, pos_y * TILE_HEIGHT + 2, TILE_WIDTH, TILE_HEIGHT-1, BLACK);
}

void draw_snake(uint8_t visible) {
  for (int i = snake_pos; i < snake_pos + snake_length; i++)
  {
    uint8_t j = i % SNAKE_MAX_LENGTH;
    if(visible) {
      draw_bitmap_tile(snake_pos_x[j], snake_pos_y[j], snake_tile);
    } else {
      draw_empty_tile(snake_pos_x[j], snake_pos_y[j]);
    }
  }
}

void handle_mutate_button() {
  if(mute_btn_state == 0 && button2) {
    enable_sound = !enable_sound;
    update_music_icon();
  }
  mute_btn_state = button2;
}

void update_level_selector() {
  update_number(current_level+1, 90, 20, 1);  // update level ID on the display
  update_number(highscores[current_level], 90, 45, 3);  // update highscore on the display for the selected level
}

void update_number(uint8_t number, uint8_t x, uint8_t y, uint8_t digits) {
  display.fillRect(x, y, 4*digits+1, 4, BLACK);
  display.setCursor(x, y);
  display.print(number, DEC);
}

void draw_speed_tile(uint8_t x, uint8_t y, uint8_t full) {
  display.drawBitmap(x, y, full ? speed_icon_full : speed_icon, SPEED_ICON_WIDTH, SPEED_ICON_HEIGHT, WHITE, TRANSPARENT);
}

void update_speed_icon() {
    const uint8_t x = (max_pos_x * TILE_WIDTH) + 5;
    const uint8_t y = 52;
    display.fillRect(x, y, 3*SPEED_ICON_WIDTH, SPEED_ICON_HEIGHT, BLACK);
    draw_speed_tile(x, y, 1);
    draw_speed_tile(x + SPEED_ICON_WIDTH, y, snake_speed < 3);
    draw_speed_tile(x + 2 * SPEED_ICON_WIDTH, y, snake_speed < 2);
}

void update_music_icon() {
   display.fillRect(MUSIC_ICON_X, MUSIC_ICON_Y, MUSIC_ICON_WIDTH, MUSIC_ICON_HEIGHT, BLACK);
   display.drawBitmap(MUSIC_ICON_X, MUSIC_ICON_Y, enable_sound ? music_on : music_off, MUSIC_ICON_WIDTH, MUSIC_ICON_HEIGHT, WHITE, TRANSPARENT);
}

void handle_game_over() {
  play_tones(collision_freq, collision_lengths, sizeof(collision_lengths));
  while(still_playing()) {
    if(millis() % 100) draw_snake((millis() / 200) %2);
    update_tones();
  }
  display.fillRect(8, 13, 84, 24, WHITE);
  display.fillRect(10, 15, 80, 20, BLACK);

  if(snake_length > highscores[current_level]) {
    display.setCursor(26, 24);
    display.print(F("NEW RECORD"));
    highscores[current_level] = snake_length;
    EEPROM_update(EEPROM_HIGHSCORE_ADDR + current_level, snake_length);
  } else {
    display.setCursor(26, 24);
    display.print(F("GAME OVER"));
  }
  game_over = 1;
  play_tones(game_over_freq, game_over_lengths, sizeof(game_over_lengths));
}

uint8_t collision(uint8_t x, uint8_t y) {
  if(x < 0 || x >= max_pos_x || y < 0 || y >= max_pos_y) {
    return 1;
  }
  for (int i = snake_pos; i < snake_pos + snake_length; i++) {
    uint8_t j = i % SNAKE_MAX_LENGTH;
    if(snake_pos_x[j] == x && snake_pos_y[j] == y) {
      return 1;
    }
  }
  for(int i=level_bricks_offset[current_level]; i<level_bricks_offset[current_level+1]; i++) {
    if(pgm_read_byte(level_bricks_x+i) == x && pgm_read_byte(level_bricks_y+i) == y) return 1;
  }

  return 0;
}

void new_food() {
  NEW_RANDOM_FOOD: food_x = random(max_pos_x);
  food_y = random(max_pos_y);
  for (int i = snake_pos; i < snake_pos + snake_length; i++) {
    uint8_t j = i % SNAKE_MAX_LENGTH;
    if(snake_pos_x[j] == food_x && snake_pos_y[j] == food_y) goto NEW_RANDOM_FOOD;
  }
  for(int i=level_bricks_offset[current_level]; i<level_bricks_offset[current_level+1]; i++) {
    if(pgm_read_byte(level_bricks_x+i) == food_x && pgm_read_byte(level_bricks_y+i) == food_y) goto NEW_RANDOM_FOOD;
  }
  draw_bitmap_tile(food_x, food_y, food_tile);
}
