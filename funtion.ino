void DropBox(int box) {
  int dropL = 0;
  int dropR = 0;

  // --- ส่วนที่ 1: คำนวณการแบ่งของ (เน้นความเร็ว) ---
  // ถ้าของมากกว่า 1 และทั้งสองด้านมีที่ว่าง -> แบ่งครึ่งเพื่อให้ปล่อยพร้อมกันได้
  if (box > 1 && left_slot > 0 && right_slot > 0) {
    int half = box / 2;
    dropL = min(left_slot, half);          // แบ่งไปซ้ายครึ่งนึง
    dropR = min(right_slot, box - dropL);  // ที่เหลือไปขวา

    // เช็คอีกที: ถ้าแบ่งแล้วยังเหลือ (เช่น ขวาเต็ม) ให้เทกลับมาซ้าย
    if (dropL + dropR < box) {
      dropL = min(left_slot, box - dropR);
    }
  } else {
    // ถ้ามาแค่ 1 ชิ้น หรือด้านใดด้านหนึ่งเต็ม -> ใช้ Logic เดิม (เติมซ้ายก่อน)
    dropL = min(left_slot, box);
    int remain = box - dropL;
    dropR = min(right_slot, remain);
  }

  // --- ส่วนที่ 2: สั่งปล่อยของแบบขนาน (Parallel) ---
  // วนลูปทำพร้อมกันทีละคู่
  while (dropL > 0 || dropR > 0) {
    
    // 1. สั่งเปิด (Open) พร้อมกัน
    if (dropL > 0) writeServoAngle(servo1, 0);   // เปิดซ้าย
    if (dropR > 0) writeServoAngle(servo2, 100);  // เปิดขวา
    
    delay(700); // รอครั้งเดียว

    // 2. สั่งปิด (Close) พร้อมกัน และลดจำนวน
    if (dropL > 0) {
      writeServoAngle(servo1, 100); // ปิดซ้าย
      dropL--;     
      left_slot--; 
    }
    if (dropR > 0) {
      writeServoAngle(servo2, 0);  // ปิดขวา
      dropR--;     
      right_slot--;
    }
    
    delay(700); // รอครั้งเดียว
  }
}

// ฟังก์ชันปล่อยซ้าย (เก็บไว้เผื่อเรียกใช้เดี่ยวๆ)
void dropLeft(int box) {
  for (int i = 0; i < box; i++) {
    writeServoAngle(servo1, 0);
    delay(700);
    writeServoAngle(servo1, 100);
    delay(700);
  }
}

// ฟังก์ชันปล่อยขวา (เก็บไว้เผื่อเรียกใช้เดี่ยวๆ)
void dropRight(int box) {
  for (int i = 0; i < box; i++) {
    writeServoAngle(servo2, 100);
    delay(700);
    writeServoAngle(servo2, 0);
    delay(700);
  }
}



void writeServoAngle(ServoTimer2 &sv, int angle) {
  int pulseWidth = map(angle, 0, 180, 750, 2250);
  sv.write(pulseWidth);
}

void STOP(){
    digitalWrite(IN1_A, HIGH);
    digitalWrite(IN2_A, HIGH);
    analogWrite(PWM_A, 0); 
    digitalWrite(IN1_B, HIGH);
    digitalWrite(IN2_B, HIGH);
    analogWrite(PWM_B, 0);
}

void wait_sw(){
  while(digitalRead(SW) == HIGH){} 
}

void INIT(){
  pinMode(IN1_A, OUTPUT); pinMode(IN2_A, OUTPUT); pinMode(PWM_A, OUTPUT);
  pinMode(IN1_B, OUTPUT); pinMode(IN2_B, OUTPUT); pinMode(PWM_B, OUTPUT);
  pinMode(SW, INPUT_PULLUP);

  servo1.attach(SERVO_PIN_1);
  servo2.attach(SERVO_PIN_2);

  writeServoAngle(servo1, 100);
  writeServoAngle(servo2, 0);
}

void motor(uint8_t n, int16_t speed) {
    int in1, in2, pwmPin;
    if (n == 1) { in1 = IN1_A; in2 = IN2_A; pwmPin = PWM_A; } 
    else if (n == 2) { in1 = IN1_B; in2 = IN2_B; pwmPin = PWM_B; } 
    else { return; }

    if (speed > 255) speed = 255;
    if (speed < -255) speed = -255;

    if (speed > 0) {
      digitalWrite(in1, HIGH); digitalWrite(in2, LOW); analogWrite(pwmPin, speed);
    } else if (speed < 0) {
      digitalWrite(in1, LOW); digitalWrite(in2, HIGH); analogWrite(pwmPin, -speed); 
    } else {
      digitalWrite(in1, LOW); digitalWrite(in2, LOW); analogWrite(pwmPin, 0);
    }
}

void ReadSensor(){
  for(uint8_t i = 0 ; i < NUMSENSOR ; i++  ){
    int sensorValue = analogRead(Sensores[i]);
    Serial.print(sensorValue); Serial.print("\t");
  }
  Serial.println(); delay(100);
}

void callibrate(bool Mode) {
  long mappedValue = 0 ;
  for (byte i = 0; i < NUMSENSOR; i++) {
  if(Mode){ mappedValue = 1000 - map(analogRead(Sensores[i]), minSensorValues[i], maxSensorValues[i], 0, 1000);}
  else { mappedValue = map(analogRead(Sensores[i]), minSensorValues2[i], maxSensorValues2[i], 0, 1000);}

    if(mappedValue > 800) mappedValue = 1000; 
    else if(mappedValue < 200) mappedValue = 0;
    sensorValues[i] = mappedValue;
  }
}

int readLine(bool isBlackLine) {
  uint32_t weightedSum = 0;
  uint32_t sum = 0;
  isOnLine = false;
  
  callibrate(isBlackLine); // อ่านค่าดิบ (White=1000, Black=0)

  for (byte i = 0; i < NUMSENSOR; i++) {
    // ปรับค่าตามโหมด เพื่อให้ "เส้น" มีค่าเป็น 1000 เสมอ
    // เช็คว่าเจอเส้นไหม (ตอนนี้ค่า > 200 คือเจอเส้นแน่นอนไม่ว่าโหมดไหน)
    if (sensorValues[i] > 200) { isOnLine = true; }
    if (sensorValues[i] > 50) { // กรอง Noise ขั้นต่ำ
       weightedSum += (uint32_t)sensorValues[i] * i * 1000;
       sum += sensorValues[i];
    }
  }
  return (sum > 0) ? weightedSum / sum : 0;
}

void robotRunTime(int Speed, int Time , bool Mode){
  unsigned long startTime = millis();
  while(millis() - startTime < Time){
    if(Mode) robotRun(Speed , 255 ,Mode);
    else robotRun(Speed , 255 ,Mode);
     
  }
}





void robotRun(int Speed, int TURN, bool lineMode) {
    // 1. อ่านค่าตำแหน่ง
    unsigned int position = readLine(lineMode); 
    int currentError = position - (NUMSENSOR - 1) * 1000 / 2;

    // =========================================================
    // กรณีที่ 1: เจอเส้น (isOnLine == true) -> ใช้ PID เต็มรูปแบบ
    // =========================================================
    if (isOnLine) { 
        
        // -----------------------------------------------------
        // ⭐ จุดที่แก้ไข: เช็คแยกตัด (Cross Line) ก่อนเช็คเลี้ยว ⭐
        // -----------------------------------------------------
        // สร้างตัวแปรมาเก็บสถานะว่าเจอขอบไหม (ใช้ค่า 600-700)
        bool sawLeft = (sensorValues[0] > 600);
        bool sawRight = (sensorValues[NUMSENSOR - 1] > 600);
        // 1. ถ้าเจอทั้ง "ซ้าย" และ "ขวา" พร้อมกัน = เส้นตัด/สี่แยก
        if (sawLeft && sawRight) {
          
             // ไม่ต้องทำอะไร ปล่อยให้มันหลุดลงไปทำ PID ด้านล่าง
             // หุ่นจะวิ่งตรงผ่านไปนิ่มๆ ไม่สะบัด
        }
        // 2. ถ้าเจอแค่ "ซ้าย" อย่างเดียว = มุมฉากซ้าย
        else if (sawLeft) { 
             motor(1, -Speed); motor(2, Speed); // ล้อซ้ายถอย
             previousError = currentError; // จำค่าล่าสุดไว้
             return; // ออกจากฟังก์ชันเลย
        }
        // 3. ถ้าเจอแค่ "ขวา" อย่างเดียว = มุมฉากขวา
        else if (sawRight) { 
             motor(1, Speed); motor(2, -Speed); // ล้อขวาถอย
             previousError = currentError; // จำค่าล่าสุดไว้
             return; // ออกจากฟังก์ชันเลย
        }

        // -----------------------------------------------------
        // คำนวณ PID (Smooth)
        // -----------------------------------------------------
        int P = currentError;
        int D = currentError - previousError; // ความชัน 
        
        long controlSignal = (P * KP) + (D * KD); 
        
        // อัปเดต previousError *เฉพาะตอนอยู่บนเส้น*
        previousError = currentError; 

        // สั่งมอเตอร์
        int pwmL = Speed + controlSignal;
        int pwmR = Speed - controlSignal;

        // ลิมิตความเร็วไม่ให้เกิน 255
        pwmL = constrain(pwmL, -255, 255);
        pwmR = constrain(pwmR, -255, 255);

        motor(1, pwmL);
        motor(2, pwmR);
    } 
    
    // =========================================================
    // กรณีที่ 2: ไม่เจอเส้น (isOnLine == false) -> เส้นประ หรือ หลุดโค้ง
    // =========================================================
    else {
        // เช็คจาก "อดีต" (previousError)
        
        // ถ้า Error เก่าน้อย (วิ่งมาตรงๆ) -> เส้นประ
        if (abs(previousError) < 2000) { 
            motor(1, Speed);
            motor(2, Speed);
        } 
        // ถ้า Error เก่าเยอะ (กำลังเลี้ยว) -> หลุดโค้ง
        else {
            if (previousError < 0) { // หลุดทางซ้าย
                motor(1, -TURN); 
                motor(2, TURN);
            } else { // หลุดทางขวา
                motor(1, TURN); 
                motor(2, -TURN);
            }
        }
    }
} 

void robotRun1(int Speed, int TURN, bool lineMode) {
    unsigned int position = readLine(lineMode);
    int error = position - (NUMSENSOR - 1) * 1000 / 2;
    int deltaError = error - previousError;
    previousError = error;
  
     
    if (isOnLine && position && position != (NUMSENSOR - 1) * 1000) {
      if (sensorValues[0]) lastDetectedSide = 0;
      if (sensorValues[NUMSENSOR - 1]) lastDetectedSide = 1;

      long controlSignal = error * KP + deltaError * KD;
      // controlSignal = constrain(controlSignal, -255, 255);

      int pwmL = Speed + controlSignal;
      int pwmR = Speed - controlSignal;

      pwmL = constrain(pwmL, -255, 255);
      pwmR = constrain(pwmR, -255, 255);

       motor(1, pwmL); motor(2, pwmR); 
    

    } 
    else {
    
      if (!lastDetectedSide) {
        motor(1, -TURN); motor(2, TURN); 
       
      }
       else {
        motor(1, TURN); motor(2, -TURN); 
                                                                                                                                           
      }

    }
    
}
