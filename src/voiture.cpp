
#include "voiture.h"
#include "math.h"
#include <fstream>


Voiture::Voiture(){
    init();
}

Voiture::~Voiture(){}

void Voiture::init(){
    //initialize speed variable
    std::cout<<"strat init"<<std::endl;
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
    battery.level       = 60.0;
    battery.consumption = 0; 

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

    std::cout<<"init done"<<std::endl;
}

/*void Voiture::navigation(double desiredSpeed, double desiredOrientation, std::vector<std::ofstream>& fileStreams){
    speed.ut = 80;
    orientation.ut = desiredOrientation;

    for(int i = 0; i < 500; i++){
        //Equation differentielle de la vitesse
        speed.deltaUt  = (speed.ut - speed.ut_1)/dt;
        speed.yt       = speed.yt_1 + dt*speed.deltaYt_1;
        speed.deltaYt  = speed.deltaYt_1 + dt*(speed.delta2Yt_1);
        speed.delta2Yt = 4*speed.deltaUt + speed.ut - 6*speed.deltaYt - speed.yt;
        speed.delta2Yt = speed.delta2Yt/10.0;

        speed.ut_1      = speed.ut;
        speed.yt_1      = speed.yt;
        speed.deltaYt_1 = speed.deltaYt;
        speed.delta2Yt_1= speed.delta2Yt;

        realSpeed = speed.yt; 

        //Equation differentielle de l'orientation

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

        //result
        posX = position.X;
        posY = position.Y;

        //consommation d'energie
        battery.consumption = (realSpeed*100)/(3600*CONSUMPTION_KM);
        battery.level = battery.level - battery.consumption;

        batteryLevel = battery.level;

        double k = (double)i*dt;

        fileStreams[0]<<realSpeed<<std::endl;
        fileStreams[1]<<realOrientation<<std::endl;
        fileStreams[2]<<posX<<std::endl;
        fileStreams[3]<<posY<<std::endl;
        fileStreams[4]<<batteryLevel<<std::endl;
        fileStreams[5]<<k<<std::endl;

        if(i == 300){
            speed.ut = 50.0;
            orientation.ut = 40.0;
        }

    }

}*/

void Voiture::alimentation(){

    if(battery.level <= 10){
        alarmeBatterie10 = true;
        alarmeBatterie80 = false;
    }
    if(battery.level >= 80){
        alarmeBatterie10 = false;
        alarmeBatterie80 = true;
        ActiveRecharge = false;
    }
    if(ActiveRecharge){
        double charge = 80/(20*60);
        battery.level = battery.level + charge;
    }
}

void Voiture::vitesse(double desiredSpeed){
    
    // Calcule de la vitesse reel
    speed.ut = desiredSpeed;
    speed.deltaUt  = (speed.ut - speed.ut_1)/dt;
    speed.yt       = speed.yt_1 + dt*speed.deltaYt_1;
    speed.deltaYt  = speed.deltaYt_1 + dt*(speed.delta2Yt_1);
    speed.delta2Yt = 4*speed.deltaUt + speed.ut - 6*speed.deltaYt - speed.yt;
    speed.delta2Yt = speed.delta2Yt/10.0;

    speed.ut_1      = speed.ut;
    speed.yt_1      = speed.yt;
    speed.deltaYt_1 = speed.deltaYt;
    speed.delta2Yt_1= speed.delta2Yt;

    realSpeed = speed.yt; 
    //std::cout<<"vitesse reel : "<<realSpeed<<std::endl;

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

    //result
    posX = position.X;
    posY = position.Y;    

}

void Voiture::batterie(double realSpeed){

    battery.consumption = (realSpeed*100)/(3600*CONSUMPTION_KM);
    battery.level = battery.level - battery.consumption;
    batteryLevel = battery.level;
}    

void Voiture::camera(double posX, double posY, bool startAnalyse){
    coord_t position {posX, posY};
    if(startAnalyse){
        //std::cout<<"Taking Picture at X = "<<posX<<" Y = "<<posY<<std::endl;
        pathMap->takePhoto(position);
        //pathMap->dumpImage("./pic.bmp");
        analyseDone = true; 
    }
    else{
        analyseDone = false;
    }
}
