
#ifndef VOITURE_
#define VOITURE_
//#include "batterie.h"
//#include "GPS.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "genMap.h"



#define dt  0.1
#define CONSUMPTION_KM  150.0


struct SpeedVar{

    // input function
    double ut;
    double ut_1;
    double deltaUt;
    double deltaUt_1;

    // output function
    double yt;
    double yt_1;
    double deltaYt;
    double deltaYt_1;
    double delta2Yt;
    double delta2Yt_1;
    
};

struct OrientationVar{

    // input function
    double ut;
    double ut_1;
    double deltaUt;
    double deltaUt_1;
    double delta2Ut;


    // output function
    double yt;
    double yt_1;
    double deltaYt;
    double deltaYt_1;
    double delta2Yt;
    double delta2Yt_1;

};

struct PositionXY_Var{

    // input function
    double X;
    double deltaX;
    double Y;
    double deltaY;
};

struct Battery{
    double level;
    double consumption;

};




class Voiture{
    public:
        double desiredSpeed;
        double realSpeed;
        double desiredOrientation;
        double realOrientation;
        double posX;
        double posY;
        double batteryLevel;
        bool alarmeBatterie80;
        bool alarmeBatterie10;
        bool ActiveRecharge;
        bool analyseStart;
        bool analyseDone;
        PathMap* pathMap;

        SpeedVar speed;
        OrientationVar orientation;
        PositionXY_Var position;
        Battery battery;
        
        Voiture();
        ~Voiture();
        
        void init();
        //void navigation(double desiredSpeed, double desiredOrientation, std::vector<std::ofstream>& fileStreams);
        void vitesse(double desiredSpeed);
        void positionOrientation(double realSpeed, double desiredOrientation);
        void batterie(double realSpeed);
        void camera(double posX, double posY, bool startAnalyse);
        void alimentation();
        void startAnalyse();
        
};

#endif