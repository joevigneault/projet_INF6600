#ifndef DATA_RECORD_
#define DATA_RECORD_
#include <iostream>
#include <fstream>
using namespace std;

class Recorder{

    public :
        ofstream recordCtrlData;
        ofstream recordCarData;

        Recorder();
        ~Recorder();

        void init();
        void recordControlerDatafunc(float time, float vitesse, float Orientation, bool chargeBatterie);
        void recordCarDatafunc(float time, float vitesse, float orientation, float posX, float poxY, float niveauBatterie);

};

#endif