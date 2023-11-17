#ifndef CRTL_
#define CTRL_
#include "genMap.h"
#include <cmath>
#include <stdio.h>
#include <iostream>



enum Vehicule { arret, marche };
enum Alarme { lowBattery, highBattery, wayPointReach, closed };
enum Destination { versDestination, versStation };
enum systemEtat {nop , getTowayPoint, getToDestination, batterieLow};
// Structure du contrôleur de Navigation
struct ctrlNavigationData {
    coord_t realPosition;
    double realSpeed;
    double realOrientation;
    double distance;
    double desiredSpeed;
    double desiredOrientation;
    double batteryLevel;
  
};
// Structure du contrôleur de Destination

struct ctrlDestinationData {
    coord_t realPosition;
    coord_t desiredDestination;
    coord_t stationDeRecharge;
    coord_t wayPoint;
    double distanceDestination;
    double distanceStation;
    bool rechargeBattery;
    Destination type;
    Alarme alarme;
};
// Structure du contrôleur de camera

struct ctrlCameraData {
    coord_t realPosition;
    coord_t lastPicturePosition;
    double distance;
    bool takePicture;
    bool checkPicture;
    bool savePicture;
};
// Structure du générateur de destination aléatoire

struct generateurAleatroire {
    coord_t newDest;
};

struct TaskData {
    PathMap* pathMap;
    ctrlNavigationData navigation;
    ctrlDestinationData destination;
    ctrlCameraData camera;
    generateurAleatroire genDestination;
    Vehicule etatVehicule;
};
class Controleur{
    public : 
        TaskData taskData;
        double consigneVitesse;
        double consigneOrientation;
        bool chargerBatterie;
        bool demarrerCycleAnalyse;
        systemEtat  syncCtrlTask;
        bool rechargeTermineeSync;
        
        Controleur();
        ~Controleur();

        void init();
        
        void ctrlNavigation(double posX,double posY, double realSpeed, 
                            double realOrientation, double batterie);
        
        void ctrlDestination(double posX, double poxY);
        void ctrlCamera(double posX,double posY,bool analyseDone );
        void alarmBattery10();
        void alarmBattery80();
        void generateurAleatoire(double posX, double posY);

};


#endif