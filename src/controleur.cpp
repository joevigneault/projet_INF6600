#include "controleur.h"

Controleur::Controleur(){
    init();
}
Controleur::~Controleur(){};

void Controleur::init(){
    taskData.etatVehicule       = arret;
    taskData.destination.type   = versDestination;
    syncCtrlTask                = nop;
    taskData.destination.rechargeBattery = false;
    rechargeTermineeSync		= false;

    //init navigateur
    taskData.navigation.realPosition    = {0, 0};
    taskData.navigation.realOrientation = 0;
    taskData.navigation.realSpeed       = 0;
    taskData.navigation.batteryLevel    = 0;

    //init controlleur de destination
    taskData.destination.realPosition   = {0, 0};

    taskData.camera.lastPicturePosition = {0, 0};

}

void Controleur::ctrlNavigation(double posX,double posY, double realSpeed, double realOrientation, double batterie) {

    double droiteX, droiteY;
    
    // Lecture des entrées
    taskData.navigation.realPosition    = {posX, posY };
    taskData.navigation.realOrientation = realOrientation;
    taskData.navigation.realSpeed       = realSpeed;
    taskData.navigation.batteryLevel    = batterie;

    // Calcul de la distance entre le point souhaité et la position actuelle 
    droiteX = taskData.destination.wayPoint.x - taskData.navigation.realPosition.x;
    droiteY = taskData.destination.wayPoint.y - taskData.navigation.realPosition.y;
    taskData.navigation.distance = sqrt(pow(droiteX, 2) + pow(droiteY, 2));

    //std::cout<<"postition reel de X :"<<posX<<std::endl;
    //std::cout<<"postition reel de Y :"<<posY<<std::endl;

    switch (taskData.etatVehicule)
    {
    case marche:
        // Calcul de la vitesse souhaitée en considérant le niveau de la batterie
        if ((taskData.navigation.distance >= 100) && (taskData.navigation.batteryLevel > 10))
        {
            taskData.navigation.desiredSpeed = 80.0;
        }
        else if (((taskData.navigation.distance < 100) && (taskData.navigation.distance >= 10)) || 
                ((taskData.navigation.distance >= 10) && (taskData.navigation.batteryLevel <= 10))) {
            taskData.navigation.desiredSpeed = 50.0;
        }
        else
        {
            // Étape atteinte et envoi du signal au contrôleur de destination
            taskData.destination.alarme = wayPointReach;
            syncCtrlTask = getTowayPoint;
            std::cout<<"Reach the way point"<<std::endl;
        }
        // Calcul de l'orientation souhaitee
        taskData.navigation.desiredOrientation = (atan((droiteX) / (droiteY)) * 180.0) / M_PI;
        if (droiteY < 0) {
            taskData.navigation.desiredOrientation = 180 + taskData.navigation.desiredOrientation;
        }
        break;
    case arret:
        taskData.navigation.desiredSpeed = 0;
        break;
    default:
        break;
    }

    // Ajustement des sorties
    consigneVitesse     = taskData.navigation.desiredSpeed;  
    consigneOrientation = taskData.navigation.desiredOrientation;  
}


void Controleur::ctrlDestination(double posX, double posY){
   
        // Lecture des entrées
    taskData.destination.realPosition = {posX, posY};
        // Switch case en fonction du type d'événement reçu
    switch (taskData.destination.alarme)
    {
    // Réception d'une alarme ALARM_LOW_BATTERY
    case lowBattery:
        // Modification du type de destination
        taskData.destination.type = versStation;
        // Calcul de la station la plus proche et affection de cette station au wayPoint
        taskData.pathMap->getClosestStation(taskData.destination.realPosition, taskData.destination.wayPoint);
        printf("Closest Station at Y = %f and X = %f\n", taskData.destination.wayPoint.y, taskData.destination.wayPoint.x);
        taskData.destination.alarme = closed;
        break;
    // Réception d'une alarme ALARM_HIGH_BATTERY
    case highBattery:
        // Modification du type de destination
        taskData.destination.type = versDestination;
        taskData.destination.rechargeBattery = 0;
        printf("Recharge terminée. Reprise du trajet.\n");
        taskData.destination.alarme = wayPointReach;
        rechargeTermineeSync = true;
        break;
    case wayPointReach:
        if (taskData.destination.type == versStation)
        {
            taskData.destination.distanceStation = sqrt(pow(taskData.destination.realPosition.x - taskData.destination.wayPoint.x, 2)
                + pow(taskData.destination.realPosition.y - taskData.destination.wayPoint.y, 2));
            if (taskData.destination.distanceStation < 10) {
                //arriver a la station
                printf("Arriver la station\n");
                taskData.destination.rechargeBattery = true;
                taskData.etatVehicule = arret;
            }
        }
        else if (taskData.destination.type == versDestination)
        {
            taskData.destination.distanceDestination = sqrt(pow(taskData.destination.realPosition.x - taskData.destination.desiredDestination.x, 2)
                + pow(taskData.destination.realPosition.y - taskData.destination.desiredDestination.y, 2));
            if ((taskData.destination.distanceDestination < 10)) {
                taskData.etatVehicule = arret;
                printf("Destination Atteinte\n");
                syncCtrlTask = getToDestination;
            }
            else {
                taskData.pathMap->genWp(taskData.destination.realPosition, taskData.destination.desiredDestination, taskData.destination.wayPoint);
                taskData.etatVehicule = marche;
                std::cout<<"wayPoint X : "<<taskData.destination.wayPoint.x<<std::endl;
                std::cout<<"wayPoint Y : "<<taskData.destination.wayPoint.y<<std::endl;
            }
        }
        taskData.destination.alarme = closed;
        break;
            case closed:
        break;
    }
    // Ajustement des sorties
    chargerBatterie = taskData.destination.rechargeBattery;
}


void Controleur::ctrlCamera(double posX,double posY ,bool analyseDone ) {
    
        // Lecture des entrées
    taskData.camera.realPosition = {posX, posY};
    taskData.camera.checkPicture = analyseDone;

    // Calcul de la distance entre la position actuelle et la position ou la dernière photo a été prise
    taskData.camera.distance = sqrt(pow(taskData.camera.realPosition.x - taskData.camera.lastPicturePosition.x, 2)
        + pow(taskData.camera.realPosition.y - taskData.camera.lastPicturePosition.y, 2));
    taskData.camera.takePicture = 0;
    //printf("camera distance : %f\n", taskData.camera.distance);
    switch (taskData.etatVehicule)
    {
    case marche:
        // Si la distance parcourue est entre 18 et 22 mètres, prendre photo et actualiser la position de la dernière photo
        if ((taskData.camera.distance < 22.0) && (taskData.camera.distance > 18.0)) {
            taskData.camera.takePicture = 1;
            taskData.camera.savePicture = 0;
            taskData.camera.lastPicturePosition = taskData.camera.realPosition;
        }
        // Si la distance entre deux photos dépasse 22m (gestion d'erreur), réinitialisation de la position de la dernière image prise
        else if (taskData.camera.distance > 22.0) {
            taskData.camera.lastPicturePosition = taskData.camera.realPosition;
        }
        break;
    default:
        break;
    }

    // Lecture d'un flanc montant sur l'entrée ANALYSE_DONE et sauvegarde de l'image
    if ((taskData.camera.checkPicture) && !(taskData.camera.savePicture)) {
        taskData.camera.savePicture = 1;
        ///taskData.pathMap->savePhoto(d->camera.realPosition);
    }
    
            // Ajustement des sorties
    //ttAnalogOut(ANALYSE_PICTURE, d->camera.takePicture);
    //return FINISHED; 
    demarrerCycleAnalyse = taskData.camera.takePicture;
}

// Définition du gestionnaire de Trigger relié au Trigger ALARM_LOW_BATTERY
void Controleur::alarmBattery10() {
    // Création d'une alarme et appel du controleur de destination
    taskData.destination.alarme = lowBattery;
}

// Définition du gestionnaire de Trigger relié au Trigger ALARM_HIGH_BATTERY
void Controleur::alarmBattery80() {

    // Création d'une alarme et appel du controleur de destination
    taskData.destination.alarme = highBattery;
}

void Controleur::generateurAleatoire(double posX, double posY) {
    
    taskData.destination.realPosition = {posX, posY};
    // Génération aléatoire d'une destination à partir de la position actuelle
    taskData.pathMap->genDest(taskData.destination.realPosition, taskData.genDestination.newDest);
    
    // Copie de la nouvelle destination souhaitée dans le controleur de destination
    taskData.destination.desiredDestination = { taskData.genDestination.newDest.x,taskData.genDestination.newDest.y };
    printf("-----------------------------\n");
    printf("Nouvelle Destination Y : %f\n", taskData.destination.desiredDestination.y);
    printf("Nouvelle Destination X : %f\n", taskData.destination.desiredDestination.x);
    printf("-----------------------------\n");
    // Attente d'une minute avant de redémarrer le vehicule
    //ttSleep(60);
    // Création d'une alarme pour indiquer la requête du prochain waypoint
    taskData.destination.type = versDestination;
    taskData.destination.alarme = wayPointReach;
    //ttCreateJob("Controle Destination");
}