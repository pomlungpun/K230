void readdata() {
  Serial.println("Reading...");

  rxBuffer = "";
  unsigned long startTime = millis();
  
  // รอข้อมูลสูงสุด 1.5 วินาที (ปรับเพิ่มจากเดิมเพื่อให้ชัวร์ขึ้น)
  while (millis() - startTime < 800) { 
    
    if (k230.available()) {
      char c = k230.read();
      
      // เมื่อจบประโยค (\n)
      if (c == '\n') {
        int data = rxBuffer.toInt();
        rxBuffer = ""; // ล้างตัวเก็บข้อความ
        
        Serial.print("Cmd: "); Serial.println(data);

        // ==================================================
        // กรณีที่ 1: DataMatrix (เลข 1-14) -> ทิ้งของ
        // ==================================================
        if (data > 0 && data < 15) { 
           Serial.print("Drop Box: "); Serial.println(data);
           
           STOP();
           delay(200); // รอให้นิ่ง
           
           DropBox(data); // สั่งทิ้งของ
           
           // ⭐ ล้างบัฟเฟอร์หลังทิ้งของเสร็จ 
           // (เพราะระหว่างทิ้งของ กล้องอาจจะส่งค่าขยะมา)
           delay(100);
           while(k230.available()) k230.read();
           
           return; // ออกไปเดินต่อ
        }
        
        // ==================================================
        // กรณีที่ 2: ไฟแดง (16) -> หยุดรอจนกว่าจะเขียว (15)
        // ==================================================
        else if (data == 16) {
          Serial.println("RED LIGHT: STOP");
          STOP();
          motor(1, 0); motor(2, 0);
          
          delay(50);
          // ล้างค่าเก่าทิ้งก่อนเริ่มรอสัญญาณใหม่
          while(k230.available()) k230.read(); 

          String waitBuf = "";
          
          // Loop นี้จะขังหุ่นไว้จนกว่าจะได้เลข 15
          while (true) {
            if (k230.available()) {
              char wc = k230.read();
              if (wc == '\n') {
                 int val = waitBuf.toInt();
                 waitBuf = "";
                 
                 // ถ้าได้ 15 (Go)
                 if (val == 15) {
                    Serial.println("GREEN -> GO!");
                    delay(100);
                    while(k230.available()) k230.read(); // ล้างทิ้งก่อนไปต่อ
                    return; // หลุด Loop ไปวิ่งต่อ
                 }
                 // ถ้าได้ค่าอื่น (เช่น 16 ซ้ำ) ให้เฉยไว้
                 else {
                    Serial.print("Waiting... Ignored: "); Serial.println(val);
                 }
              } else if (isDigit(wc)) {
                 waitBuf += wc;
              }
            }
          }
        }
        
        // ==================================================
        // กรณีที่ 3: ไฟเขียว/ทางผ่าน (15) -> ไปต่อเลย
        // ==================================================
       
        
      } 
      // เก็บสะสมตัวเลข
      else if (isDigit(c)) {
        rxBuffer += c;
      }
    }
  }
  
  // ถ้าหมดเวลาแล้วไม่มีข้อมูลที่ถูกต้อง
  Serial.println("Timeout - Continue");
}