#include "voiture.h"
#include "math.h"
#include <fstream>


int i=0;
Voiture::Voiture(){
    init();
}

Voiture::~Voiture(){}

void Voiture::init(){
    //initialize speed variable
    speed.ut        = 0;
    speed.ut_1      = 0;
    speed.deltaUt   = 0;
    speed.deltaUt_1 = 0;
    speed.yt        = 0;
    speed.yt_1      = 0;
    speed.deltaYt   = 0;
    speed.deltaYt_1 = 0;
    speed.delta2Yt  = 0;
    speed.delta2Yt_1= 0;
    //init orienation variables
    orientation.ut          = 0;
    orientation.ut_1        = 0;
    orientation.deltaUt     = 0;
    orientation.deltaUt_1   = 0;
    orientation.delta2Ut    = 0;
    orientation.yt          = 0;
    orientation.yt_1        = 0;
    orientation.deltaYt     = 0;
    orientation.deltaYt_1   = 0;
    orientation.delta2Yt    = 0;
    orientation.delta2Yt_1  = 0; 

    //init Position
    position.X      = 0;
    position.Y      = 0;
    position.deltaX = 0;
    position.deltaY = 0;

    //init battery
    battery.level       = 50.0;
    battery.consumption = 0; 
    battery.lowState	= false;

    //init output
    realSpeed       = 0;
    realOrientation = 0;
    posX            = 0;
    posY            = 0;
    batteryLevel    = 0;
    alarmeBatterie10 = false;
    alarmeBatterie80 = false;

    //init input
    desiredSpeed       = 0;
    desiredOrientation = 0;
    
    //init camera
    analyseDone = false;
    analyseStart= false;


    //init recharge
    ActiveRecharge = false;
}

void Voiture::vitesse(double desiredSpeed){
    
    // Calcule de la vitesse reel
    speed.ut = desiredSpeed;
    if((speed.ut-speed.yt)<=10 && (speed.ut - speed.yt)> 1){
    	speed.yt = speed.yt + 1;
    }
    else if((speed.yt-speed.ut)<=10 && (speed.yt-speed.ut)> 1){
    	speed.yt = speed.yt - 1;
    }
    else if(abs(speed.yt-speed.ut)<= 1){
    	speed.yt = speed.ut;
    }
    else{
    	speed.deltaUt  = (speed.ut - speed.ut_1)/dt;
		speed.yt       = speed.yt_1 + dt*speed.deltaYt_1;
		speed.deltaYt  = speed.deltaYt_1 + dt*(speed.delta2Yt_1);
		speed.delta2Yt = 4*speed.deltaUt + speed.ut - 6*speed.deltaYt - speed.yt;
		speed.delta2Yt = speed.delta2Yt/10;
    }

    speed.ut_1      = speed.ut;
    speed.yt_1      = speed.yt;
    speed.deltaYt_1 = speed.deltaYt;
    speed.delta2Yt_1= speed.delta2Yt;
    realSpeed = speed.yt;
}

void Voiture::positionOrientation(double realSpeed, double desiredOrientation){

    //Calcule de l'orientation
    orientation.ut = desiredOrientation;
    orientation.deltaUt  = (orientation.ut - orientation.ut_1)/dt;
    orientation.delta2Ut = (orientation.deltaUt - orientation.deltaUt_1)/dt;
    orientation.deltaYt  = orientation.deltaYt_1 + dt*(orientation.delta2Yt_1);
    orientation.yt       = orientation.yt_1 + dt*(orientation.deltaYt_1);
    orientation.delta2Yt = 5*orientation.delta2Ut + 5*orientation.deltaUt + orientation.ut- 6*orientation.deltaYt - orientation.yt;

    orientation.ut_1      = orientation.ut;
    orientation.deltaUt_1 = orientation.deltaUt;
    orientation.yt_1      = orientation.yt;
    orientation.deltaYt_1 = orientation.deltaYt;
    orientation.delta2Yt_1= orientation.delta2Yt;

    realOrientation = orientation.yt;

    // calcul de POSX et POSY
    double orientationInRad;
    orientationInRad = (realOrientation*M_PI)/180.0;
    position.X = position.X + position.deltaX*dt;
    position.Y = position.Y + position.deltaY*dt;
    position.deltaX = (realSpeed*sin(orientationInRad))/(3.6);
    position.deltaY = (realSpeed*cos(orientationInRad))/(3.6);
    //accélérer la simulation
    position.deltaX = position.deltaX * 10;
    position.deltaY = position.deltaY * 10;

    //result
    posX = position.X;
    posY = position.Y;
}

void Voiture::batterie(double realSpeed){

    battery.consumption = (realSpeed*100)/(3600*CONSUMPTION_KM);
    battery.level = battery.level - battery.consumption;

    //Vérification du niveau de la batterie et déclanchement de l'alarme pour une ittération seulement
	if(battery.level <= 10 && !battery.lowState){
		battery.lowState = true;
		alarmeBatterie10 = true;
	}
	else if(battery.level >= 80 && battery.lowState){
		battery.lowState = false;
		alarmeBatterie80 = true;
		ActiveRecharge = false;
	}
	else{
		alarmeBatterie10 = false;
		alarmeBatterie80 = false;
	}
	//Recharge de la batterie
	if(ActiveRecharge && realSpeed == 0){
		battery.level = battery.level + 0.5;
	}
    batteryLevel = battery.level;
}    

void Voiture::camera(double posX, double posY, bool startAnalyse){
    coord_t position {posX, posY};
    if(startAnalyse){
        pathMap->takePhoto(position);
        analyseDone = true;
    }
    else{
        analyseDone = false;
    }
}

void Voiture::queueRead(nsCommon::Queue<uint32_t>& commandQueue){
	uint32_t speedRead, orientationRead, batterieRead, takePictureRead;
    double desiredSpeed2, desiredOrientation2;

    desiredSpeed2= desiredSpeed;
	desiredOrientation2= desiredOrientation;

	speedRead = commandQueue.pop();
	orientationRead = commandQueue.pop();
	batterieRead = commandQueue.pop();
	takePictureRead = commandQueue.pop();
	while(!commandQueue.empty()) //delete everything in the queue (old data which is obsolete now)
	{
		commandQueue.pop();
	}
	commandQueue.push(speedRead);
	commandQueue.push(orientationRead);
	commandQueue.push(batterieRead);
	commandQueue.push(takePictureRead);

	desiredSpeed = static_cast<double>(speedRead)/1000-1000;
    if(desiredSpeed != 80 && desiredSpeed != 50 && desiredSpeed != 0){
        desiredSpeed = desiredSpeed2;
    }
	desiredOrientation = static_cast<double>(orientationRead)/1000-1000;
    if(desiredOrientation>360){
        desiredOrientation = desiredOrientation2;
    }
	ActiveRecharge = static_cast<double>(batterieRead)/1000-1000;
	analyseStart = static_cast<double>(takePictureRead)/1000-1000;
}

void Voiture::queueWrite(nsCommon::Queue<uint32_t>& actualQueue){
	while (!actualQueue.empty()) //delete everything in the queue (old data which is obsolete now)
	{
		actualQueue.pop();
	}
	actualQueue.push((batteryLevel+1000)*1000);
	actualQueue.push((analyseDone+1000)*1000);
	actualQueue.push((posX+1000)*1000);
	actualQueue.push((posY+1000)*1000);
	//insertion d'une erreur au 500 ittération
	if(i==500){
		actualQueue.push((realSpeed+20000)*1000);
		i=0;
	}
	else{
		actualQueue.push((realSpeed+1000)*1000);
		i++;
	}
	actualQueue.push((realOrientation+1000)*1000);
}
