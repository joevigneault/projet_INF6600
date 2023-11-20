#include "dataRecord.h"


Recorder::Recorder(){
    init();
}
Recorder::~Recorder(){
    recordCtrlData.close();
    recordCarData.close();
}

void Recorder::init(){

    recordCtrlData.open("./ctrlRecord.csv");
    recordCarData.open("./carSimulator.csv");
    recordCtrlData<<"dt"<<","<<"Vitesse"<<","<<"Orientation"<<","<<"batterie"<<endl;
	recordCarData<<"dt"<<","<<"Vitesse"<<","<<"Orientation"<<","<<"posX"<<","<<"PosY"<<","<<"batterie"<<endl;
}


void Recorder::recordControlerDatafunc(float time, float vitesse, float orientation, bool chargeBatterie){

     recordCtrlData<<time<<","<<vitesse<<","<<orientation<<","<<chargeBatterie<<endl;
}

void Recorder::recordCarDatafunc(float time,float vitesse, float orientation, float posX, float posY, float niveauBatterie){

   recordCarData<<time<<","<<vitesse<<","<<orientation<<","<<posX<<","<<posY<<","<<niveauBatterie<<endl; 
}
