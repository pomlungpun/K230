void Finished(){
  while(true){
    if(sensorValues[0]>500&&sensorValues[7]>500){break;}
    else robotRun(110 , 170,true);
  }
  while(1){
    STOP();
  }
}
void ChanngeC(bool M){
  robotRunTime(120, 150,M); 
    while(true){
      if(sensorValues[0]>800&&sensorValues[7]>800){robotRunTime(120, 100,!M); break;}
    else robotRun(110 , 200,M);
     
    }
   

}

// ---------------------------------------------------------
void Junction(bool Mode){
   robotRunTime(80, 50, Mode); 
  
   while(true){
      if(Mode) robotRun(120, 180, Mode);
      else robotRun1(120, 180, Mode);
    
      int blackCount = 0;
      for(int i=0; i<NUMSENSOR; i++){
         if(sensorValues[i] > 500) blackCount++;
      }
      if(blackCount == 8) break; 
   }

   robotRunTime(70, 110, Mode); 
   STOP(); 
   delay(200); // รอให้หยุดนิ่ง + กล้องประมวลผลใหม่
   rxBuffer = ""; // ล้างข้อความค้าง
   
   readdata(); // อ่านค่าใหม่
}