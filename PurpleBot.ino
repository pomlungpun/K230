#include <ServoTimer2.h> 
#include <SoftwareSerial.h> // 1. เรียกใช้ Library

ServoTimer2 servo1;
ServoTimer2 servo2;

SoftwareSerial k230(12, 11);  // RX = D9
String rxBuffer = "";

#define NUMSENSOR 8
#define SW 2

// --- Motor Pins ---
#define IN1_A  7
#define IN2_A  8
#define PWM_A  9 

#define IN1_B  5
#define IN2_B  4
#define PWM_B  6

// --- Servo Pins ---
#define SERVO_PIN_1  10
#define SERVO_PIN_2  3

// --- PID Constants ---
#define KP 0.06
#define KD 0.7

bool isOnLine = false;
bool lastDetectedSide = false;
int previousError = 0;

bool Mode = true ;

const int Sensores[NUMSENSOR] = {A0,A1,A2,A3,A4,A5,A6,A7};
const int maxSensorValues[NUMSENSOR] = {549	,765	,670	,714	,807	,773	,681	,660	};
const int minSensorValues[NUMSENSOR] = {54	,87,	54,	58	,84	,67,	57,	53	};

const int maxSensorValues2[NUMSENSOR] = {568	,768	,665	,690,	813	,794	,647,	611		};
const int minSensorValues2[NUMSENSOR] = {54	,87,	54,	58,	84,	67,	57,	53		}; 

int sensorValues[NUMSENSOR] = { };

int left_slot = 5;
int right_slot = 5;

//////////////////////////// Mode ///////////////////////////////////////
int checkCount = 6 ;
bool Line = true ;
void writeServoAngle(ServoTimer2 &sv, int angle);

void setup() {
  INIT();

  Serial.begin(9600); // Debug คอมพิวเตอร์
   k230.begin(9600);
  Serial.println("ready to press !!!");
   wait_sw(); 
  delay(500);
   Serial.println("Go");
//////////////////////////////////////////////// สนามเเรก
    // Junction(1); 
    // Junction(1); 
    // Junction(1); 
    // ChanngeC(1);
    // Junction(0); 
    // Junction(0); 
    // ChanngeC(0);


    // Junction(1); 
    // Junction(1); 
    // Finished();
//////////////////////////////////////////// สามนามสอง
    Junction(1); 
    Junction(1); 
    Junction(1); 
    Junction(1); 
    Junction(1); 

    ChanngeC(1);
     Junction(0); 
     Junction(0);

     ChanngeC(0);

     Finished();

}

void loop() {
  // ReadSensor();
   
  readdata();

        //  int Value = readLine(false);
        // // callibrate();
        // for(byte i = 0;i<NUMSENSOR;i++){
        //   Serial.print(sensorValues[i]);
        //   Serial.print("\t");

        
        // Serial.println(Value);
        // delay(50);
        //  ReadSensor();
        // ปกติ loop จะว่างในโค้ดเดินเส้นแบบ Step-by-step
        // แต่ถ้าต้องอ่านค่าตลอดเวลา ใส่ checkCamera() ที่นี่ได้
}


