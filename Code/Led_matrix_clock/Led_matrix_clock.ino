#include "charNumber2matrix.h"
#define INPUT_X_A D1
#define INPUT_X_B D2
#define INPUT_X_C D3
#define INPUT_Y   D4
#define SRCLK     D5 //shift register clock, first layer of registers
#define RCLK      D6 //register clock,  second layer of registers and row SR clock
#define OE        D7 // output enable, output is disabled when high
#define CLR       D8

#define WIDTH_A 7
#define WIDTH_B 6
#define WIDTH_C 6
#define HEIGHT  5


int AW[5][19]=  {
                 {0,1,1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0},
                 {0,1,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0},
                 {0,1,1,1,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0},
                 {0,1,0,0,1,0,1,1,0,1,1,0,0,0,0,0,0,0,0},
                 {0,1,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0}
                };

int clear_matrix[5][19] = {
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                 {0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                 {0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
                };

int test_matrix[5][19] = {
                 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                };
int display_matrix[5][19];          
int (*p)[19];
int *row;


void write2screen();
void writeline2screen(int*);
void init_y();
void string2matrix(char []);



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

  digitalWrite(OE, LOW); //Enable output of the 74HC595 sr
  digitalWrite(CLR, HIGH); //Disable clear

  memcpy(&display_matrix,&clear_matrix, sizeof(clear_matrix)); //Set display to 0
  // memcpy(&display_matrix,&test_matrix, sizeof(clear_matrix));// Test is all leds are working
}

void write2screen(){
  /*
    Get row of matrix and display through writeline2screen
    Set one row active by shifting a 0 through the row SR
    RCLK controls the row shift-register and sends data from
    the first layer of registers to the second of the column
    shift register
  */
  int y_init = 1;
  init_y();
  for(int i = 0; i <= HEIGHT-1; i++){
    digitalWrite(RCLK, LOW);
    row = &display_matrix[i][0];
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
  int A[WIDTH_A], B[WIDTH_B], C[WIDTH_C];
  memcpy(&A, row, WIDTH_A*4);
  memcpy(&B, row + WIDTH_A, WIDTH_B*4);
  memcpy(&C, row + WIDTH_A + WIDTH_B, WIDTH_C*4);
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
    Converts a 4 char string into a matrix that can
    be written on LED matrix
  */
  memcpy(&display_matrix,&clear_matrix, sizeof(clear_matrix)); // clear matrix
  int positions[] = {1, 5, 11, 15}; //starting x position of numbers
  int temp_char[5][3];
  for(int i = 0; i < 4; i++){
    memcpy(&temp_char, &numbers[str[i] - '0'], sizeof(temp_char));// char to index gives an array of led matrix for each number
    for(int column = 0; column < 3; column++){
      for(int row = 0; row <= 5; row++){
        display_matrix[row][column+positions[i]] = temp_char[row][column]; // Copy 3x5 digit array to display array
      }
    }
  }
}

int counter = 0;
int curr_time = 0, prev_time = 0;
char nums [4];
char clear_nums[4] = {'0','0','0','0'};
void loop() {
  write2screen();

  //simple counter
  //Converts a 4 digit number into chars 
  if(millis() - prev_time > 1000){
    counter++;
    memcpy(&nums, &clear_nums, sizeof(nums));
    if(counter<10){
      nums[3] = counter + '0';
    }else if(counter<100){
      nums[2] = counter/10 + '0';
      nums[3] = counter%10 + '0';
    }else if(counter<1000){
      nums[1] = counter/100 + '0';
      nums[2] = (counter%100)/10 + '0';
      nums[3] = counter%10 + '0';
    }
    string2matrix(nums);
    display_matrix[1][9] = 1;
    display_matrix[3][9] = 1;
    prev_time = millis();
  }
}
