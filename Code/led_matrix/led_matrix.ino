#include "charNumber2matrix.h"
#include <Wire.h>

#define INPUT_X_A D0  //was D1
#define INPUT_X_B D9 //RX
#define INPUT_X_C D3
#define INPUT_Y   D4
#define SRCLK     D5 //shift register clock, first layer of registers
#define RCLK      D6 //register clock,  second layer of registers and row SR clock
#define OE        D7 // output enable, output is disabled when high
#define CLR       D8

#define SDA       D2
#define SCL       D1

#define WIDTH_SR_A 7
#define WIDTH_SR_B 6
#define WIDTH_SR_C 6
#define WIDTH_SR_MATRIX   (WIDTH_SR_A + WIDTH_SR_B + WIDTH_SR_C)
#define HEIGHT_MATRIX   5

//Configure if timer or counter should be displayed
#define DISPLAY_TIME 1
#define DISPLAY_COUNTER 0
#define ENABLE_ANIMATIONS 1

#if (DISPLAY_TIME==1) && (DISPLAY_COUNTER==1)
  #error "Cannot display both counter and time"
#endif

enum direction{DIR_LEFT, DIR_RIGHT};

int AW[5][19]=  {
                 {0,1,1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0},
                 {0,1,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0},
                 {0,1,1,1,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0},
                 {0,1,0,0,1,0,1,1,0,1,1,0,0,0,0,0,0,0,0},
                 {0,1,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0}
                };

int clear_matrix [5][19] = {
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
                };

int tmp_matrix [5][19] = {
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
                };
                
int display_matrix [5][19];          
int (*p)[19];
int *row;

bool start_snake_animation = false;

void write2screen();
void writeline2screen(int*);
void init_y();
void string2matrix(char []);
String get_time();

void setup() {
  //set gpio pins as output
  pinMode(INPUT_X_A, OUTPUT);
  pinMode(INPUT_X_B, OUTPUT);
  pinMode(INPUT_X_C, OUTPUT);
  pinMode(INPUT_Y, OUTPUT);
  pinMode(SRCLK, OUTPUT);
  pinMode(RCLK, OUTPUT);
  pinMode(OE, OUTPUT);
  pinMode(CLR, OUTPUT);

  Wire.begin(D1,D2); //
  // Serial.begin(300);

  digitalWrite(OE, LOW); //Enable output of the 74HC595 sr
  digitalWrite(CLR, HIGH); //Disable clear

  memcpy(&display_matrix ,&clear_matrix , sizeof(clear_matrix)); //Set display to 0
  // memcpy(&display_matrix ,&test_MATRIX , sizeof(clear_matrix));// Test is all leds are working
}

void write2screen(){
  /*
    Get row of MATRIX  and display through writeline2screen
    Set one row active by shifting a 0 through the row SR
    RCLK controls the row shift-register and sends data from
    the first layer of registers to the second of the column
    shift register
  */
  int y_init = 1;
  init_y();
  for(int i = 0; i <= HEIGHT_MATRIX -1; i++){
    digitalWrite(RCLK, LOW);
    row = &display_matrix [i][0];
    writeline2screen(row);
    y_init = (i == 0) ? 0 : 1;
    digitalWrite(INPUT_Y, y_init);
    digitalWrite(RCLK, HIGH);
    delay(1);
  }
}

void writeline2screen(int *row){
  /*
    Split row in three arrays and send serially to 
    shift registers. B and C uses one less output than 
    A. 
  */
  int A[WIDTH_SR_A], B[WIDTH_SR_B], C[WIDTH_SR_C];
  memcpy(&A, row, WIDTH_SR_A*4);
  memcpy(&B, row + WIDTH_SR_A, WIDTH_SR_B*4);
  memcpy(&C, row + WIDTH_SR_A + WIDTH_SR_B, WIDTH_SR_C*4);
  for(int i = 6; i >= 0; i--){

    digitalWrite(SRCLK, LOW);
    digitalWrite(INPUT_X_A, A[i]);
    if(i == 6){
      digitalWrite(INPUT_X_B, 0);
      digitalWrite(INPUT_X_C, 0);
    }else{
      digitalWrite(INPUT_X_B, B[i]);
      digitalWrite(INPUT_X_C, C[i]);
    }
    digitalWrite(SRCLK, HIGH);
    delay(0.01); // Delay in ms
  }
  digitalWrite(SRCLK, LOW); 
}
void init_y(){
  /*
    Set all bits in the row register to high to disable
    all rows. 
  */
  for(int i = 0; i < 7; i++){
    digitalWrite(RCLK, LOW);
    digitalWrite(INPUT_Y, 1);
    digitalWrite(RCLK, HIGH);
    delay(1);
  }
}

void string2matrix(char str[]){
  /*
    Converts a 4 char string into a MATRIX  that can
    be written on LED MATRIX 
  */
  memcpy(&display_matrix ,&clear_matrix , sizeof(clear_matrix)); // clear MATRIX 
  int number_offset[] = {1, 5, 11, 15}; //starting x position of numbers
  int temp_char[5][3];
  for(int i = 0; i < 4; i++){
    memcpy(&temp_char, &numbers[str[i] - '0'], sizeof(temp_char));// char to index gives an array of led MATRIX  for each number

    for(int column = 0; column < 3; column++){
      for(int row = 0; row <= 5; row++){
        display_matrix [row][column + number_offset[i]] = temp_char[row][column]; // Copy 3x5 digit array to display array
      }
    }
  }
}

void clear_display_matrix(){
  memcpy(&display_matrix ,&clear_matrix , sizeof(clear_matrix)); // clear MATRIX 
  write2screen();
}

void time_add_colon_to_matrix(){
  display_matrix [1][9] = 1;
  display_matrix [3][9] = 1;
}

String get_time(){
  //See data sheet for register configurations
  // https://web.wpi.edu/Pubs/E-project/Available/E-project-010908-124414/unrestricted/DS3231-DS3231S.pdf
  byte data[7];
  String tid[7]; 
  Wire.beginTransmission(0x68);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(0x68,7);
  for(int i = 0 ; i < 7 ; i++)
    data[i]=Wire.read();

  tid[0]=String((data[0]&0xF0)>>4)+String(data[0]&0x0F);
  tid[1]=String((data[1]&0xF0)>>4)+String(data[1]&0x0F);
  tid[2]=String((data[2]&0x30)>>4)+String(data[2]&0x0F);

  String timestamp_hh_mm=tid[2]+tid[1];
  return timestamp_hh_mm;
}

void run_snake_animation(){
  const int SNAKE_LENGTH = 6; 
  int x_head = 0, y_head = 0; //(x,y) position of head of snake
  int x_tail = -SNAKE_LENGTH, y_tail = 0; //(x,y) position of head of snake
  int tail_fac = 0, head_fac = 0;
  enum direction DirHead = DIR_RIGHT, DirTail = DIR_RIGHT;

  for(int x = 0; x < (WIDTH_SR_MATRIX * HEIGHT_MATRIX) + SNAKE_LENGTH; x++){
      if(x_head >= 0 && x_head < WIDTH_SR_MATRIX)
        if(y_head >= 0 && y_head < HEIGHT_MATRIX)
          display_matrix[y_head][x_head] = 1;

      if(x_tail >= 0 && x_tail < WIDTH_SR_MATRIX)
        if(y_tail >= 0 && y_tail < HEIGHT_MATRIX)
          if(tmp_matrix [y_tail][x_tail] != 1)
            display_matrix [y_tail][x_tail] = 0;

      //head turns on leds in the matrix
      if((x - y_head * head_fac) % (WIDTH_SR_MATRIX - 1) == 0 && x != 0){
        DirHead = (DirHead == DIR_RIGHT)? DIR_LEFT : DIR_RIGHT; //toggle direction
        y_head++;
        head_fac = 0; 
      }else{
        head_fac = 1;
        switch(DirHead){
          case DIR_LEFT:
            x_head--;
            break;
          case DIR_RIGHT:
            x_head++;
            break;
        }
      }
      //tail turns off leds in the matrix
      if((x - SNAKE_LENGTH - y_tail * tail_fac) % (WIDTH_SR_MATRIX - 1) == 0 && x != SNAKE_LENGTH){
        DirTail = (DirTail == DIR_RIGHT)? DIR_LEFT : DIR_RIGHT;
        y_tail++;
        tail_fac = 0;
      }else{
        tail_fac = 1;
        switch(DirTail){
          case DIR_LEFT:
            x_tail--;
            break;
          case DIR_RIGHT:
            x_tail++;
            break;
        }
      }
      //display the current matrix for 5 repetitions
      for(int i = 0; i < 5; i++)
        write2screen();
    }
    start_snake_animation = false;
}

int counter = 0;
int curr_time = 0, prev_time = 0;
char nums [4];
String time_hh_mm, last_time_hh_mm = "";
char clear_nums[4] = {'0','0','0','0'};


void loop() {
  
  if(start_snake_animation && ENABLE_ANIMATIONS)
  {//starts a snake animation on hour change
    run_snake_animation();
  }
  else
  {
    write2screen();
  #if (DISPLAY_TIME) //configure these at the top
    //Displays time acquired from DS3231
    //string format is 1224, 0012, 2359
    if(millis() - prev_time > 1000){
      time_hh_mm = get_time();
      time_hh_mm.toCharArray(nums, 5);
      string2matrix(nums);
      time_add_colon_to_matrix();

      if(time_hh_mm.substring(2) == "00" && last_time_hh_mm.substring(2) == "59" && ENABLE_ANIMATIONS)
      { //enable animation on hour change 
        memcpy(&tmp_matrix , &display_matrix , sizeof(tmp_matrix)); // save current time MATRIX 
        start_snake_animation = true;
        clear_display_matrix();
      }
      last_time_hh_mm = time_hh_mm;
      prev_time = millis();
    }
  #elif (DISPLAY_COUNTER)
    //simple counter
    //Converts a 4 digit number into chars in time format
    if(millis() - prev_time > 1000){
      counter++;
      memcpy(&nums, &clear_nums, sizeof(nums));
      if(counter < 10)
      {
        nums[3] = counter + '0';
      }
      else if(counter < 100)
      {
        nums[2] = counter/10 + '0';
        nums[3] = counter%10 + '0';
      }else if(counter < 1000)
      {
        nums[1] = counter/100 + '0';
        nums[2] = (counter%100)/10 + '0';
        nums[3] = counter%10 + '0';
      }
      string2matrix(nums);
      time_add_colon_to_matrix();
      prev_time = millis();
    }
  #endif
  }
}
