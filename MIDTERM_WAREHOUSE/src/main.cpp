// WAREHOUSECODE, 


// okay so the warehouse needs to take an empty bucket
// place the empty bucket to be filled
// place the disc in buccket
//plcae back on warehouse

#include <Arduino.h>
#include <P1AM.h>
#include <MotorEncoder.h>

enum MachineStates {
  Home,
  x1,
  x1convey,
  x1putback,
  x2,
  x2convey,
  x2putback,
  x3,
  x3convey,
  x3putback,
  x4,
  x4convey,
  x4putback,
  x5,
  x5convey,
  x5putback,
  x6,
  x6convey,
  x6putback,
  x7,
  x7convey,
  x7putback,
  x8,
  x8convey,
  x8putback,
  x9,
  x9convey,
  x9putback
  
}; //I want to home it first, wait for sensor, then pick up, then drop off.

MachineStates curState = Home;

//module variables
int modInput=1;
int modOutput=2;
//int modAnalog=3; //no analog for warehouse

//ROBART ARM INPUTS
int horzRef=1; // ref = home
int InlightBar=2;
int OutlightBar=3; //closest to robart arm
int vertref=4;
int horzPulse1=5;
int horzPulse2=6;
int vertPulse1=7;
int vertPulse2=8;
int CANTFrontbutt=9;
int CANTRearbutt=10;
int ROBARTSIGNAL=11;

//ROBART ARM OUTPUTS
int conveyorFORWARD=1;
int conveyorBACKWARD=2;
int horzMotorIn=3; //TOWARDS RACK
int horzMotorOut=4; //TOWARDS CONVEYOR
int VERTDOWN=5;
int VERTUP=6;
int CANTFORD=7;//CANTILEVER MOTOR FORWARDS
int CANTBACK=8;
int OUTPUTREADYFORROBART=9;

//variables add in later

void setup() {
  //startup P1AM Modules
  delay(1000); //just incase delay
  while(!P1.init()){
  delay(100);
  };
  Serial.begin(9600);
  delay(1000); //just in case delay
  
}
bool FullBasket(){ //full basket at inner light barrier ready to be stored
  return !P1.readDiscrete(modInput, InlightBar);
}
bool BasketPlaced(){
  return !P1.readDiscrete(modInput, OutlightBar);
}
bool DiskDroppedInBasket(){
  return !P1.readDiscrete(modInput, ROBARTSIGNAL);
}
//cantileber buttons and movement logic


//motor movement things
//MotorEncoder horzMotor(modInput,modOutput, 4, 3, 7, 2); //SAME FOR HORIZONTAL MOTOR ON THIS
MotorEncoder horzMotor(modInput,modOutput, horzMotorIn, horzMotorOut, horzPulse1, horzRef); //SAME FOR HORIZONTAL MOTOR ON THIS
MotorEncoder vertMotor(modInput,modOutput, VERTDOWN,VERTUP, vertPulse1, vertref);

 //gonna do some location testing on 
    //ex x1=200horz, 20vert
    //[x1 x2 x3] x1=975, 50  x2=680,50  x3=395,50
    //[x4 x5 x6] x4=975; 230 x5=680,230 x6=395,230 -200
    //[x7 x8 x9] x7=975; 420 x8=680,420 x9=395,420 -390

void loop() {

  switch (curState) {
    case Home:
      //Home the robot arm,
            Serial.println("Homing the WAREARM");
      P1.writeDiscrete(false, modOutput, CANTFORD);

      // drive BACK until rear switch turns ON (active-LOW here; drop '!' if active-HIGH)
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(500);
      
      vertMotor.Home();
      horzMotor.Home();
      Serial.println("Homed");
      //delay(500);
      //go to waiting state
      curState=x1;
      break;

      case x1:
      //delay(300);
      while (!horzMotor.MoveTo(975)) {
      horzMotor.UpdatePulse();
      }
      //delay(300);
      while (!vertMotor.MoveTo(50)) {
      vertMotor.UpdatePulse();
      }
      //move arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //PICKUP
      while (!vertMotor.MoveTo(0)) {
      vertMotor.UpdatePulse();
      }
      //MOVE cant arm BACK
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);
      //MOVE TO CONVEYOR
      vertMotor.Home();
      horzMotor.Home();
      //delay(300);
      //home is inline with conveyor on horizontal
      while (!horzMotor.MoveTo(22)) {
      horzMotor.UpdatePulse();
      }
      while (!vertMotor.MoveTo(320)) {
      vertMotor.UpdatePulse();
      }
      //delay(200);
      //arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //drop down onto veyor
      while (!vertMotor.MoveTo(380)) {
      vertMotor.UpdatePulse();
      }

    curState=x1convey;
    break;
    case x1convey:
    if (!P1.readDiscrete(modInput, InlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorFORWARD);
    while(P1.readDiscrete(modInput,OutlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorFORWARD);//stops
    }
      
    P1.writeDiscrete(HIGH, modOutput, 13);
    
    //tell the robart arm to supply disc
    //wait for the disc to be dropped
    while(!P1.readDiscrete(modInput,ROBARTSIGNAL)){
      delay(1500);
    }; //waits
    P1.writeDiscrete(LOW, modOutput, 13);
    //move conveyor back
    if(!P1.readDiscrete(modInput, OutlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorBACKWARD);
    while(P1.readDiscrete(modInput,InlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorBACKWARD);//stops
    }

    curState=x1putback;
    break;
    case x1putback:
    //raise basket up
    while (!vertMotor.MoveTo(0)) {
      vertMotor.UpdatePulse();
      }
      //stuck arm in
    P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);
    
      //move to x1 home
    while (!horzMotor.MoveTo(975)) {
      horzMotor.UpdatePulse();
      }
      //delay(300);

      //stick arm out
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);

    //place
    while (!vertMotor.MoveTo(50)) {
      vertMotor.UpdatePulse();
      }
//suck arm back in
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);

      //home
      vertMotor.Home();
      horzMotor.Home();
      //delay(300);

      curState=x2;
    
    break;
    case x2:
    //delay(300);
      while (!horzMotor.MoveTo(680)) {
      horzMotor.UpdatePulse();
      }
      //delay(300);
      while (!vertMotor.MoveTo(50)) {
      vertMotor.UpdatePulse();
      }
      //move arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //PICKUP
      while (!vertMotor.MoveTo(0)) {
      vertMotor.UpdatePulse();
      }
      //MOVE cant arm BACK
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);
      //MOVE TO CONVEYOR
      vertMotor.Home();
      horzMotor.Home();
      //delay(300);
      //home is inline with conveyor on horizontal
      while (!horzMotor.MoveTo(22)) {
      horzMotor.UpdatePulse();
      }
      while (!vertMotor.MoveTo(320)) {
      vertMotor.UpdatePulse();
      }
      //delay(200);
      //arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //drop down onto veyor
      while (!vertMotor.MoveTo(380)) {
      vertMotor.UpdatePulse();
      }

    curState=x2convey;
    break;
    case x2convey:
    if (!P1.readDiscrete(modInput, InlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorFORWARD);
    while(P1.readDiscrete(modInput,OutlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorFORWARD);//stops
    }
      
    P1.writeDiscrete(HIGH, modOutput, 13);
    
    //tell the robart arm to supply disc
    //wait for the disc to be dropped
    while(!P1.readDiscrete(modInput,ROBARTSIGNAL)){
      delay(1500);
    }; //waits
    P1.writeDiscrete(LOW, modOutput, 13);
    //move conveyor back
    if(!P1.readDiscrete(modInput, OutlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorBACKWARD);
    while(P1.readDiscrete(modInput,InlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorBACKWARD);//stops
    }

    curState=x2putback;
    break;
    case x2putback:
    //raise basket up
    while (!vertMotor.MoveTo(0)) {
      vertMotor.UpdatePulse();
      }
      //stuck arm in
    P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);
    
      //move to x1 home
    while (!horzMotor.MoveTo(680)) {
      horzMotor.UpdatePulse();
      }
      //delay(300);

      //stick arm out
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);

    //place
    while (!vertMotor.MoveTo(50)) {
      vertMotor.UpdatePulse();
      }
//suck arm back in
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);

      //home
      vertMotor.Home();
      horzMotor.Home();
      //delay(300);

      curState=x3;

    break;
    case x3:
    //delay(300);
      while (!horzMotor.MoveTo(395)) {
      horzMotor.UpdatePulse();
      }
     // delay(300);
      while (!vertMotor.MoveTo(50)) {
      vertMotor.UpdatePulse();
      }
      //move arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
     // delay(200);
      //PICKUP
      while (!vertMotor.MoveTo(0)) {
      vertMotor.UpdatePulse();
      }
      //MOVE cant arm BACK
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);
      //MOVE TO CONVEYOR
      vertMotor.Home();
      horzMotor.Home();
      //delay(300);
      //home is inline with conveyor on horizontal
      while (!horzMotor.MoveTo(22)) {
      horzMotor.UpdatePulse();
      }
      while (!vertMotor.MoveTo(320)) {
      vertMotor.UpdatePulse();
      }
     // delay(200);
      //arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //drop down onto veyor
      while (!vertMotor.MoveTo(380)) {
      vertMotor.UpdatePulse();
      }

    curState=x3convey;
    break;
    case x3convey:
    if (!P1.readDiscrete(modInput, InlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorFORWARD);
    while(P1.readDiscrete(modInput,OutlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorFORWARD);//stops
    }
      
    P1.writeDiscrete(HIGH, modOutput, 13);
    
    //tell the robart arm to supply disc
    //wait for the disc to be dropped
    while(!P1.readDiscrete(modInput,ROBARTSIGNAL)){
      delay(1500);
    }; //waits
    P1.writeDiscrete(LOW, modOutput, 13);
    //move conveyor back
    if(!P1.readDiscrete(modInput, OutlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorBACKWARD);
    while(P1.readDiscrete(modInput,InlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorBACKWARD);//stops
    }

    curState=x3putback;
    break;
    case x3putback:
    //raise basket up
    while (!vertMotor.MoveTo(0)) {
      vertMotor.UpdatePulse();
      }
      //stuck arm in
    P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
     // delay(200);
    
      //move to x1 home
    while (!horzMotor.MoveTo(395)) {
      horzMotor.UpdatePulse();
      }
     // delay(300);

      //stick arm out
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
     // delay(200);

    //place
    while (!vertMotor.MoveTo(50)) {
      vertMotor.UpdatePulse();
      }
//suck arm back in
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);

      //home
      vertMotor.Home();
      horzMotor.Home();
      //delay(300);

      curState=x4;
    break;
    case x4:
    //delay(300);
      while (!horzMotor.MoveTo(975)) {
      horzMotor.UpdatePulse();
      }
      //delay(300);
      while (!vertMotor.MoveTo(230)) {
      vertMotor.UpdatePulse();
      }
      //move arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
     // delay(200);
      //PICKUP
      while (!vertMotor.MoveTo(200)) {///////////////////////////////////////////////////////////////200 test
      vertMotor.UpdatePulse();
      }
      //MOVE cant arm BACK
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
     // delay(200);
      //MOVE TO CONVEYOR
      vertMotor.Home();
      horzMotor.Home();
     // delay(300);
      //home is inline with conveyor on horizontal
      while (!horzMotor.MoveTo(22)) {
      horzMotor.UpdatePulse();
      }
      while (!vertMotor.MoveTo(320)) {
      vertMotor.UpdatePulse();
      }
     // delay(200);
      //arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
     // delay(200);
      //drop down onto veyor
      while (!vertMotor.MoveTo(380)) {
      vertMotor.UpdatePulse();
      }

    curState=x4convey;
    break;
    case x4convey:
    if (!P1.readDiscrete(modInput, InlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorFORWARD);
    while(P1.readDiscrete(modInput,OutlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorFORWARD);//stops
    }
      
    P1.writeDiscrete(HIGH, modOutput, 13);
    
    //tell the robart arm to supply disc
    //wait for the disc to be dropped
    while(!P1.readDiscrete(modInput,ROBARTSIGNAL)){
      delay(1500);
    }; //waits
    P1.writeDiscrete(LOW, modOutput, 13);
    //move conveyor back
    if(!P1.readDiscrete(modInput, OutlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorBACKWARD);
    while(P1.readDiscrete(modInput,InlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorBACKWARD);//stops
    }

    curState=x4putback;
    break;
    case x4putback:
    //raise basket up
    while (!vertMotor.MoveTo(0)) {
      vertMotor.UpdatePulse();
      }
      //stuck arm in
    P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);
    
      //move to x1 home
    while (!horzMotor.MoveTo(975)) {
      horzMotor.UpdatePulse();
      }
      //delay(300);

    while (!vertMotor.MoveTo(200)) {////////////////////////////////////200 test
      vertMotor.UpdatePulse();
      }

      //stick arm out
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      delay(200);

    //place
    while (!vertMotor.MoveTo(230)) {////////////////////////////////////230 test
      vertMotor.UpdatePulse();
      }
//suck arm back in
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);

      //home
      vertMotor.Home();
      horzMotor.Home();
      //delay(300);

      curState=x5;
    break;
    case x5:
    //delay(300);
      while (!horzMotor.MoveTo(680)) {
      horzMotor.UpdatePulse();
      }
     // delay(300);
      while (!vertMotor.MoveTo(230)) {
      vertMotor.UpdatePulse();
      }
      //move arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //PICKUP
      while (!vertMotor.MoveTo(200)) {/////////////////////////200 test
      vertMotor.UpdatePulse();
      }
      //MOVE cant arm BACK
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
     // delay(200);
      //MOVE TO CONVEYOR
      vertMotor.Home();
      horzMotor.Home();
     // delay(300);
      //home is inline with conveyor on horizontal
      while (!horzMotor.MoveTo(22)) {
      horzMotor.UpdatePulse();
      }
      while (!vertMotor.MoveTo(320)) {
      vertMotor.UpdatePulse();
      }
      //delay(200);
      //arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //drop down onto veyor
      while (!vertMotor.MoveTo(380)) {
      vertMotor.UpdatePulse();
      }

    curState=x5convey;
    break;
    case x5convey:
    if (!P1.readDiscrete(modInput, InlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorFORWARD);
    while(P1.readDiscrete(modInput,OutlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorFORWARD);//stops
    }
      
    P1.writeDiscrete(HIGH, modOutput, 13);
    
    //tell the robart arm to supply disc
    //wait for the disc to be dropped
    while(!P1.readDiscrete(modInput,ROBARTSIGNAL)){
      delay(1500);
    }; //waits
    P1.writeDiscrete(LOW, modOutput, 13);
    //move conveyor back
    if(!P1.readDiscrete(modInput, OutlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorBACKWARD);
    while(P1.readDiscrete(modInput,InlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorBACKWARD);//stops
    }

    curState=x5putback;
    break;
    case x5putback:
    //raise basket up
    while (!vertMotor.MoveTo(0)) {
      vertMotor.UpdatePulse();
      }
      //stuck arm in
    P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
     // delay(200);
    
      //move to x5 home
    while (!horzMotor.MoveTo(680)) {
      horzMotor.UpdatePulse();
      }
     // delay(300);
      while (!vertMotor.MoveTo(200)) {////////////////////////////////////200 test
      vertMotor.UpdatePulse();
      }
     // delay(200);

      //stick arm out
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
     // delay(200);

    //place
    while (!vertMotor.MoveTo(230)) {
      vertMotor.UpdatePulse();
      }
//suck arm back in
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);

      //home
      vertMotor.Home();
      horzMotor.Home();
      //delay(300);

      curState=x6;
    break;
    case x6:
    //delay(300);
      while (!horzMotor.MoveTo(395)) {
      horzMotor.UpdatePulse();
      }
      //delay(300);
      while (!vertMotor.MoveTo(230)) {
      vertMotor.UpdatePulse();
      }
      //move arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //PICKUP
      while (!vertMotor.MoveTo(200)) {
      vertMotor.UpdatePulse();
      }
      //MOVE cant arm BACK
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);
      //MOVE TO CONVEYOR
      vertMotor.Home();
      horzMotor.Home();
     // delay(300);
      //home is inline with conveyor on horizontal
      while (!horzMotor.MoveTo(22)) {
      horzMotor.UpdatePulse();
      }
      while (!vertMotor.MoveTo(320)) {
      vertMotor.UpdatePulse();
      }
      //delay(200);
      //arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //drop down onto veyor
      while (!vertMotor.MoveTo(395)) {
      vertMotor.UpdatePulse();
      }

    curState=x6convey;
    break;
    case x6convey:
    if (!P1.readDiscrete(modInput, InlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorFORWARD);
    while(P1.readDiscrete(modInput,OutlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorFORWARD);//stops
    }
      
    P1.writeDiscrete(HIGH, modOutput, 13);
    
    //tell the robart arm to supply disc
    //wait for the disc to be dropped
    while(!P1.readDiscrete(modInput,ROBARTSIGNAL)){
      delay(1500);
    }; //waits
    P1.writeDiscrete(LOW, modOutput, 13);
    //move conveyor back
    if(!P1.readDiscrete(modInput, OutlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorBACKWARD);
    while(P1.readDiscrete(modInput,InlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorBACKWARD);//stops
    }

    curState=x6putback;
    break;
    case x6putback:
    //raise basket up
    while (!vertMotor.MoveTo(0)) {
      vertMotor.UpdatePulse();
      }
      //stuck arm in
    P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
     // delay(200);
    
      //move to x6 home
    while (!horzMotor.MoveTo(395)) {
      horzMotor.UpdatePulse();
      }
     // delay(300);
      while (!vertMotor.MoveTo(200)) {////////////////////////////////////200 test
      vertMotor.UpdatePulse();
      }

      //stick arm out
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      delay(200);

    //place
    while (!vertMotor.MoveTo(230)) {
      vertMotor.UpdatePulse();
      }
//suck arm back in
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
     // delay(200);

      //home
      vertMotor.Home();
      horzMotor.Home();
     // delay(300);

      curState=x7;
    break;
    case x7:
    //delay(300);
      while (!horzMotor.MoveTo(975)) {
      horzMotor.UpdatePulse();
      }
      //delay(300);
      while (!vertMotor.MoveTo(420)) {
      vertMotor.UpdatePulse();
      }
      //move arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
     // delay(200);
      //PICKUP
      while (!vertMotor.MoveTo(390)) {/////////////////////390 test
      vertMotor.UpdatePulse();
      }
      //MOVE cant arm BACK
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
     // delay(200);
      //MOVE TO CONVEYOR
      vertMotor.Home();
      horzMotor.Home();
    //  delay(300);
      //home is inline with conveyor on horizontal
      while (!horzMotor.MoveTo(22)) {
      horzMotor.UpdatePulse();
      }
      while (!vertMotor.MoveTo(320)) {
      vertMotor.UpdatePulse();
      }
     // delay(200);
      //arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
     // delay(200);
      //drop down onto veyor
      while (!vertMotor.MoveTo(380)) {
      vertMotor.UpdatePulse();
      }

    curState=x7convey;
    break;
    case x7convey:
    if (!P1.readDiscrete(modInput, InlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorFORWARD);
    while(P1.readDiscrete(modInput,OutlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorFORWARD);//stops
    }
      
    P1.writeDiscrete(HIGH, modOutput, 13);
    
    //tell the robart arm to supply disc
    //wait for the disc to be dropped
    while(!P1.readDiscrete(modInput,ROBARTSIGNAL)){
      delay(1500);
    }; //waits
    P1.writeDiscrete(LOW, modOutput, 13);
    //move conveyor back
    if(!P1.readDiscrete(modInput, OutlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorBACKWARD);
    while(P1.readDiscrete(modInput,InlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorBACKWARD);//stops
    }

    curState=x7putback;
    break;
    case x7putback:
    //raise basket up
    while (!vertMotor.MoveTo(0)) {
      vertMotor.UpdatePulse();
      }
      //stuck arm in
    P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
     // delay(200);
    
      //move to x1 home
    while (!horzMotor.MoveTo(975)) {
      horzMotor.UpdatePulse();
      }
     // delay(300);
      while (!vertMotor.MoveTo(390)) {////////////////////////////////////390test
      vertMotor.UpdatePulse();
      }

      //stick arm out
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
     // delay(200);

    //place
    while (!vertMotor.MoveTo(420)) {
      vertMotor.UpdatePulse();
      }
//suck arm back in
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
     // delay(200);

      //home
      vertMotor.Home();
      horzMotor.Home();
     // delay(300);

      curState=x8;
    break;
    case x8:
    //delay(300);
      while (!horzMotor.MoveTo(680)) {
      horzMotor.UpdatePulse();
      }
     // delay(300);
      while (!vertMotor.MoveTo(420)) {
      vertMotor.UpdatePulse();
      }
      //move arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //PICKUP
      while (!vertMotor.MoveTo(390)) {/////////////////390 test
      vertMotor.UpdatePulse();
      }
      //MOVE cant arm BACK
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);
      //MOVE TO CONVEYOR
      vertMotor.Home();
      horzMotor.Home();
      //delay(300);
      //home is inline with conveyor on horizontal
      while (!horzMotor.MoveTo(22)) {
      horzMotor.UpdatePulse();
      }
      while (!vertMotor.MoveTo(320)) {
      vertMotor.UpdatePulse();
      }
      //delay(200);
      //arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //drop down onto veyor
      while (!vertMotor.MoveTo(380)) {
      vertMotor.UpdatePulse();
      }

    curState=x8convey;
    break;
    case x8convey:
    if (!P1.readDiscrete(modInput, InlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorFORWARD);
    while(P1.readDiscrete(modInput,OutlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorFORWARD);//stops
    }
      
    P1.writeDiscrete(HIGH, modOutput, 13);
    
    //tell the robart arm to supply disc
    //wait for the disc to be dropped
    while(!P1.readDiscrete(modInput,ROBARTSIGNAL)){
      delay(1500);
    }; //waits
    
    //move conveyor back
    if(!P1.readDiscrete(modInput, OutlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorBACKWARD);
    while(P1.readDiscrete(modInput,InlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorBACKWARD);//stops
    }
    P1.writeDiscrete(LOW, modOutput, 13);
    curState=x8putback;
    break;
    case x8putback:
    //raise basket up
    while (!vertMotor.MoveTo(0)) {
      vertMotor.UpdatePulse();
      }
      //stuck arm in
    P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);
    
      //move to x1 home
    while (!horzMotor.MoveTo(680)) {
      horzMotor.UpdatePulse();
      }
      //delay(300);

      while (!vertMotor.MoveTo(390)) {////////////////////////////////////200 test
      vertMotor.UpdatePulse();
      }

      //stick arm out
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);

    //place
    while (!vertMotor.MoveTo(420)) {
      vertMotor.UpdatePulse();
      }
//suck arm back in
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);

      //home
      vertMotor.Home();
      horzMotor.Home();
      //delay(300);

      curState=x9;
    break;
    case x9:
    //delay(300);
      while (!horzMotor.MoveTo(395)) {
      horzMotor.UpdatePulse();
      }
     // delay(300);
      while (!vertMotor.MoveTo(420)) {
      vertMotor.UpdatePulse();
      }
      //move arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //PICKUP
      while (!vertMotor.MoveTo(390)) {//////////////////////390 test
      vertMotor.UpdatePulse();
      }
      //MOVE cant arm BACK
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);
      //MOVE TO CONVEYOR
      vertMotor.Home();
      horzMotor.Home();
      //delay(300);
      //home is inline with conveyor on horizontal
      while (!horzMotor.MoveTo(22)) {
      horzMotor.UpdatePulse();
      }
      while (!vertMotor.MoveTo(320)) {
      vertMotor.UpdatePulse();
      }
      //delay(200);
      //arm forward
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);
      //drop down onto veyor
      while (!vertMotor.MoveTo(380)) {
      vertMotor.UpdatePulse();
      }

    curState=x9convey;
    break;
    case x9convey:
    if (!P1.readDiscrete(modInput, InlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorFORWARD);
    while(P1.readDiscrete(modInput,OutlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorFORWARD);//stops
    }
      
    P1.writeDiscrete(HIGH, modOutput, 13);
    
    //tell the robart arm to supply disc
    //wait for the disc to be dropped
    while(!P1.readDiscrete(modInput,ROBARTSIGNAL)){
      delay(1500);
    }; //waits
    P1.writeDiscrete(LOW, modOutput, 13);
    //move conveyor back
    if(!P1.readDiscrete(modInput, OutlightBar)){
    P1.writeDiscrete(true, modOutput, conveyorBACKWARD);
    while(P1.readDiscrete(modInput,InlightBar)){}; //waits
    P1.writeDiscrete(false, modOutput, conveyorBACKWARD);//stops
    }

    curState=x9putback;
    break;
    case x9putback:
    //raise basket up
    while (!vertMotor.MoveTo(0)) {
      vertMotor.UpdatePulse();
      }
      //stuck arm in
    P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);
    
      //move to x1 home
    while (!horzMotor.MoveTo(395)) {
      horzMotor.UpdatePulse();
      }
     // delay(300);

      while (!vertMotor.MoveTo(390)) {////////////////////////////////////200 test
      vertMotor.UpdatePulse();
      }
      //stick arm out
      P1.writeDiscrete(false, modOutput, CANTBACK);
      P1.writeDiscrete(true,  modOutput, CANTFORD);
      while (!P1.readDiscrete(modInput, CANTFrontbutt)) {}//waits
      P1.writeDiscrete(false, modOutput, CANTFORD);//stops
      //delay(200);

    //place
    while (!vertMotor.MoveTo(420)) {
      vertMotor.UpdatePulse();
      }
//suck arm back in
      P1.writeDiscrete(false, modOutput, CANTFORD);
      P1.writeDiscrete(true,  modOutput, CANTBACK);
      while (!P1.readDiscrete(modInput, CANTRearbutt)) {}//WAITS
      P1.writeDiscrete(false, modOutput, CANTBACK); // stop
      //delay(200);

      //home
      vertMotor.Home();
      horzMotor.Home();
     // delay(300);

      curState=Home;
    break;
    default:
    break;
    
  }
};