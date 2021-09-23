/*
Adafruit Arduino - Lesson 4. 8 LEDs and a Shift Register
*/
 
int latchPin0 = 25;
int clockPin0 = 26;
int dataPin0 = 27;

int latchPin1 = 29;
int clockPin1 = 30;
int dataPin1 = 31;

int numOfRegisters = 2;
byte* registerState;
 
byte leds = 0;

//Mux control pins for analog signal (SIG_pin) default for arduino mini pro
const byte a0 = 2;
const byte a1 = 3;
const byte a2 = 4;
const byte a3 = 5;

//Mux control pins for analog signal (SIG_pin) default for arduino mini pro
const byte w0 = 7;
const byte w1 = 8;
const byte w2 = 9;
const byte w3 = 10;

//Mux in "SIG" pin default for arduino mini pro 
const byte SIG_pin0 = A0; 
const byte SIG_pin1 = A1;

const boolean muxChannel[16][4]={
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };


//incoming serial byte
int inByte = 0;

int valor = 0;  
int calibra[15][15] = {0};         //Calibration array for the min values of each od the 225 sensors.
int minsensor= 512;          //Variable for staring the min array
int multiplier = 512;

int even = 0;
 
void setup() 
{
  
  Serial.begin(115200);

   while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  
  pinMode(latchPin0, OUTPUT);
  pinMode(dataPin0, OUTPUT);  
  pinMode(clockPin0, OUTPUT);

  pinMode(latchPin1, OUTPUT);
  pinMode(dataPin1, OUTPUT);  
  pinMode(clockPin1, OUTPUT);

  //Initialize array
  registerState = new byte[numOfRegisters];
  for (size_t i = 0; i < numOfRegisters; i++) {
    registerState[i] = 0;
  }

  pinMode(w0, OUTPUT); 
  pinMode(w1, OUTPUT); 
  pinMode(w2, OUTPUT); 
  pinMode(w3, OUTPUT);

  digitalWrite(w0, LOW);
  digitalWrite(w1, LOW);
  digitalWrite(w2, LOW);
  digitalWrite(w3, LOW);

  pinMode(a0, OUTPUT); 
  pinMode(a1, OUTPUT); 
  pinMode(a2, OUTPUT); 
  pinMode(a3, OUTPUT);

  digitalWrite(a0, HIGH);
  digitalWrite(a1, HIGH);
  digitalWrite(a2, HIGH);
  digitalWrite(a3, HIGH);

  
  /*
  for (int i = 0; i < 15; i++){
    for(int j = 0; j <15 ; j++){
      calibra[i][j] = 0;
        }
  }

  for (int k = 0; k < 50; k++){
    for (int i = 0; i < 15; i++){
        regWrite(i, HIGH);
      for(int j = 15; j >= 0 ; j--){
           writeMux(j);
           valor = analogRead(A0);
            calibra[i][j] = calibra[i][j] + valor;
        }
      regWrite(i, LOW);
   }
   delay(50);
  }

  for (int i = 0; i < 15; i++){
    for(int j = 0; j < 15 ; j++){
        calibra[i][j] = calibra[i][j]/50;
        if(calibra[i][j] < minsensor){
          minsensor = calibra[i][j];
          }
        }
  }
  */
  Serial.flush();
}
 
char grid[60 * 30 * 4 + 50];
const int left_threshold = 12;
const int right_threshold = 35;
void loop() 
{
  grid[0] = '\0';
  strcpy(grid, "START");
  
  byte a[] = {a0, a1, a2, a3};
  for(int i = 0 ; i < 60 ; ++i){
    int quarter = i / 15;
    int line = i % 15;
    for(int patch = 0 ; patch < 4 ; ++patch){
      if(patch == quarter){
         digitalWrite(a[patch], LOW);
      } else {
        digitalWrite(a[patch], HIGH);
      }
    }

    
    for(int j = 14; j >= 0 ; --j){
          regWrite0(j, HIGH);
          writeMux(line);
          valor = analogRead(A0);
          valor = map(valor,1, 1024,1,127); 


           if(valor > left_threshold){
             noteOn(j, i, valor);
           }
           /*else
           {
            noteOn(j, i, 0);
           }*/
          
          //char number_str[10];
          //strcat(grid, itoa(valor, number_str, 10));
          //strcat(grid, ",");
          regWrite0(j, LOW);
    }

    
     for(int j = 14;  j >= 0 ; --j){
          regWrite1(j, HIGH);
          writeMux(line);
          valor = analogRead(A14);

         /* if(valor < calibra[j][i]){
            valor = calibra[j][i];
          }*/

         valor = map(valor,1, 350, 1,127);  
          
          if(valor > right_threshold){
             noteOn(j, i+60, valor);
           }
           /*else
           {
            noteOn(j, i+60, 0);
           }*/
           
          //char number_str[10];
          //strcat(grid, itoa(valor, number_str, 10));
          //strcat(grid, ",");
          regWrite1(j, LOW);
    }

    
    
    
    //strcat(grid, "\t");
  }

  
  //strcat(grid, "\n");
  //Serial.flush();
  //Serial.printf("%s", grid);
}
 

void regWrite0(int pin, bool state){
  //Determines register
  int reg = pin / 8;
  //Determines pin for actual register
  int actualPin = pin - (8 * reg);

  //Begin session
  digitalWrite(latchPin0, LOW);
  
  for (int i = 0; i < numOfRegisters; i++){
    //Get actual states for register
    byte* states = &registerState[i];

    //Update state
    if (i == reg){
      bitWrite(*states, actualPin, state);
    }

    //Write
    shiftOut(dataPin0, clockPin0, MSBFIRST, *states);
  }

  //End session
  digitalWrite(latchPin0, HIGH);
}

void regWrite1(int pin, bool state){
  //Determines register
  int reg = pin / 8;
  //Determines pin for actual register
  int actualPin = pin - (8 * reg);

  //Begin session
  digitalWrite(latchPin1, LOW);

  for (int i = 0; i < numOfRegisters; i++){
    //Get actual states for register
    byte* states = &registerState[i];

    //Update state
    if (i == reg){
      bitWrite(*states, actualPin, state);
    }

    //Write
    shiftOut(dataPin1, clockPin1, MSBFIRST, *states);
  }

  //End session
  digitalWrite(latchPin1, HIGH);
}


void writeMux(byte channel){
  byte controlPin[] = {w0, w1, w2, w3};

  //loop through the 4 sig
  for(byte i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

}

void noteOn(int channel, int pitch, int velocity) {
    command(0x90 + channel, pitch, velocity);
}

void command(int cmd, int value1, int value2) {
  Serial.write(cmd);
  Serial.write(value1);
  Serial.write(value2);
}
