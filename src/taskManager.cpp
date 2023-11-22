#include "taskManager.h"

/******************************************************************************
 * Initialize the queues
 *****************************************************************************/
nsCommon::Queue<uint32_t> actualQueue(6);
nsCommon::Queue<uint32_t> commandQueue(4);

sem_t simulator1_sync, simulator2_sync, simulator3_sync;
sem_t genAleatoire_sync, ctrlDest_sync, alarmeLow_sync, alarmeHigh_sync;

pthread_mutex_t sync_actualQueue, sync_commandQueue;

Recorder record;
/*******************************************************
 * Timer
 *****************************************************/
int32_t init_timer(struct sigevent* event, struct itimerspec* itime,
		           timer_t* timer, const int32_t chanel_id,
				   const uint32_t period) {
	int32_t error;
	int32_t period_s;
	int32_t period_ns;

	/* Set event as pulse and attach to channel */
	event->sigev_notify = SIGEV_PULSE;
	event->sigev_coid   = ConnectAttach(ND_LOCAL_NODE, 0, chanel_id,
									    _NTO_SIDE_CHANNEL, 0);
	/* Set basic priority and set event code */
	event->sigev_priority = 0;
	event->sigev_code     = TASK_PULSE_CODE;

	/* Create timer and associate to event */
	error = timer_create(CLOCK_MONOTONIC, event, timer);
	if(0 != error) {
		printf("Error creating timer\n");
		return error;
	}

	/* Set the itime structure */
	period_s  = period / 1000;
	period_ns = (1000000 * period) - (period_s * 1000000000);
	itime->it_value.tv_sec = period_s;
	itime->it_value.tv_nsec = period_ns;
	itime->it_interval.tv_sec = period_s;
	itime->it_interval.tv_nsec = period_ns;

	/* Set the timer period */
	return timer_settime(*timer, 0, itime, NULL);
}
/*******************************************************
 * pulse handler 
 *****************************************************/
void* pulse_handler(void* args) {

	sem_t*      sync_sem;
	int32_t     rcvid;
	pulse_msg_t msg;
	int32_t     task_chid;

	/* Get the arguments */
	sync_sem  = ((vitesseThread_arg*)args)->semaphore;
	task_chid = ((vitesseThread_arg*)args)->chid;

	while(1<2) {
		/* Get the pulse message */
		rcvid = MsgReceive(task_chid, &msg, sizeof(pulse_msg_t), NULL);
		if (0 == rcvid) {
			if (TASK_PULSE_CODE == msg.pulse.code) {
				/* Release semaphore */
				if(0 != sem_post(sync_sem)) {
					/* Print error */
					printf("Could not post semaphore: %d\n", errno);
				}
			}
			else {
				/* Print error */
				printf("Unknown message received: %d\n", rcvid);
			}
		}
		else {
			/* Print error */
			printf("Message receive failed: %d (%d)\n", rcvid, errno);
		}
	}
}
/*******************************************************
 * initialiser les connexions E/S 
 *****************************************************/
 void initSignal(){

	actualQueue.push((1000)*1000);
	actualQueue.push((1000)*1000);
	actualQueue.push((1000)*1000);
	actualQueue.push((1000)*1000);
	actualQueue.push((1000)*1000);
	actualQueue.push((1000)*1000);
	commandQueue.push((1000)*1000);
	commandQueue.push((1000)*1000);
	commandQueue.push((1000)*1000);
	commandQueue.push((1000)*1000);
 }
/**********************************************************************
 *               ROUTINE DE LA PARTIE CONTINUE
 ***********************************************************************/
/*******************************************************
 * Routine de la vitesse reel
 *****************************************************/
void* vitesseRoutine(void* args) {

	sem_t*          sync_sem;
	Voiture* 		smartCar;

	/* Get the arguments */
	sync_sem  = ((vitesseThread_arg*)args)->semaphore;
	smartCar  = ((vitesseThread_arg*)args)->smartCar;

	float temps = 0;

	/* Routine loop */
	while(true) {
		/* Wait for the pulse handler to release the semaphore */
		sem_wait(sync_sem);
		//Lire les donnees en provenance du controleur
		record.recordControlerDatafunc(temps,smartCar->desiredSpeed, smartCar->desiredOrientation, smartCar->ActiveRecharge);

		pthread_mutex_lock(&sync_commandQueue);
		smartCar->queueRead(commandQueue);
		pthread_mutex_unlock(&sync_commandQueue);

		smartCar->vitesse(smartCar->desiredSpeed);
		//std::cout<<"LA VITESSE désirée = "<<smartCar->desiredSpeed<<std::endl;
		sem_post(&simulator1_sync);
		temps+=dt;
	}
}

/***************************************************************
 * Routine de l'orientation et de la position reel du vehicule
 **************************************************************/
 void* orientationPositionRoutine(void* args){

	Voiture* 		smartCar;
	/* Get the arguments */
	smartCar  = ((vitesseThread_arg*)args)->smartCar;

	///* Routine loop 
	while(true) {
		sem_wait(&simulator1_sync);
		smartCar->positionOrientation(smartCar->realSpeed, smartCar->desiredOrientation);
		sem_post(&simulator2_sync);
	}
}

/***************************************************************
 *         Routine de la consomation d'energie
 **************************************************************/
void* batterieRoutine(void* args){

	Voiture* 		smartCar;
	/* Get the arguments */
	smartCar  = ((vitesseThread_arg*)args)->smartCar;

	float temps = 0;
	///* Routine loop 
	while(true) {
		sem_wait(&simulator2_sync);
		smartCar->batterie(smartCar->realSpeed);
		/* Création des alarmes */
		if(smartCar->alarmeBatterie10){
    		sem_post(&alarmeLow_sync);
    	}
    	if(smartCar->alarmeBatterie80){
			sem_post(&alarmeHigh_sync);
		}
		record.recordCarDatafunc(temps,smartCar->realSpeed,smartCar->realOrientation,
								smartCar->posX,smartCar->posY,smartCar->batteryLevel);

		/* Write the data in the queue */
		pthread_mutex_lock(&sync_actualQueue);
		smartCar->queueWrite(actualQueue);
		pthread_mutex_unlock(&sync_actualQueue);

		temps+=dt;
	}
}
/***************************************************************
 *                   Routine Camera
 **************************************************************/
void* cameraRoutine(void* args){

	sem_t*          sync_sem;
	Voiture* 		smartCar;

	/* Get the arguments */
	sync_sem  = ((vitesseThread_arg*)args)->semaphore;
	smartCar  = ((vitesseThread_arg*)args)->smartCar;

	
	while(true) {

		sem_wait(sync_sem);

		pthread_mutex_lock(&sync_commandQueue);
		smartCar->queueRead(commandQueue);
		pthread_mutex_unlock(&sync_commandQueue);

		smartCar->camera(smartCar->posX, smartCar->posY, smartCar->analyseStart);

		pthread_mutex_lock(&sync_actualQueue);
		smartCar->queueWrite(actualQueue);
		pthread_mutex_unlock(&sync_actualQueue);
		
	}
}
/**********************************************************************
 *               ROUTINE DU CONTROLLEUR
 ***********************************************************************/
/***************************************************************
 * 			Generateur de destination
 **************************************************************/
void* generateurAleatoireRoutine(void *args) {

	Controleur*    controleur;
	/* Get the arguments */
	controleur   = ((Controleur*)args);

	/* Routine loop */
	while(true) {
		sem_wait(&genAleatoire_sync);
		pthread_mutex_lock(&sync_actualQueue);
		controleur->queueRead(actualQueue);
		pthread_mutex_unlock(&sync_actualQueue);

    	controleur->generateurAleatoire(controleur->dataRead.posXReel, controleur->dataRead.posYReel);
    	controleur->taskData.pathMap->dumpImage("./pic2.bmp");
		sem_post(&ctrlDest_sync);
	}
}

/***************************************************************
 * 	      controleur de destination 
 **************************************************************/
void* ctrlDestinationRoutine(void *args) {

	Controleur*    controleur;
	/* Get the arguments */
	controleur   = ((Controleur*)args);

	/* Routine loop */
	while(true) {
		sem_wait(&ctrlDest_sync);
		controleur->syncCtrlTask = nop;

		pthread_mutex_lock(&sync_actualQueue);
		controleur->queueRead(actualQueue);
		pthread_mutex_unlock(&sync_actualQueue);

		controleur->ctrlDestination(controleur->dataRead.posXReel, controleur->dataRead.posYReel);

		if(controleur->syncCtrlTask == getToDestination){
			//generation d'une nouvelle destination
			sem_post(&genAleatoire_sync);
		}
		if(controleur->rechargeTermineeSync){
			controleur->rechargeTermineeSync = false;
			sem_post(&ctrlDest_sync); //reprise du fonctionnement normal
		}

		pthread_mutex_lock(&sync_commandQueue);
		controleur->queueWrite(commandQueue);
		pthread_mutex_unlock(&sync_commandQueue);
	}
}
/***************************************************************
 * 	      controleur de Navigation
 ***************************************************************/
void* ctrlNavigationRoutine(void *args) {

	sem_t*          sync_sem;
	Controleur*    controleur;
	/* Get the arguments */
	sync_sem     = ((controlleurThread_arg*)args)->semaphore;
	controleur   = ((controlleurThread_arg*)args)->control;

	/* Routine loop */
	while(true) {
		/* Wait for the pulse handler to release the semaphore */
		if(0 == sem_wait(sync_sem)) {
			pthread_mutex_lock(&sync_actualQueue);
			controleur->queueRead(actualQueue);
			pthread_mutex_unlock(&sync_actualQueue);

			controleur->ctrlNavigation(controleur->dataRead.posXReel, controleur->dataRead.posYReel, controleur->dataRead.vitesseReel, controleur->dataRead.orientationReel, controleur->dataRead.niveauBatterie);

			if(controleur->syncCtrlTask == getTowayPoint){
				//Arrivé au wayPoint
				sem_post(&ctrlDest_sync);
			}
			pthread_mutex_lock(&sync_commandQueue);
			controleur->queueWrite(commandQueue);
			pthread_mutex_unlock(&sync_commandQueue);
		}
	}
}

/***************************************************************
 * 	      controleur de camera
 ***************************************************************/

 void* ctrlCameraRoutine(void *args) {

	sem_t*          sync_sem;
	Controleur*    controleur;

	/* Get the arguments */
	sync_sem     = ((controlleurThread_arg*)args)->semaphore;
	controleur   = ((controlleurThread_arg*)args)->control;

	/* Routine loop */
	while(true) {
		//sem_wait(&ctrlNavSync);
		sem_wait(sync_sem);

		pthread_mutex_lock(&sync_actualQueue);
		controleur->queueRead(actualQueue);
		pthread_mutex_unlock(&sync_actualQueue);

		controleur->ctrlCamera(controleur->dataRead.posXReel, controleur->dataRead.posYReel, controleur->dataRead.analyseTerminee);
		//std::cout<<"CAMERA TEST CTRL"<<std::endl;
		pthread_mutex_lock(&sync_commandQueue);
		controleur->queueWrite(commandQueue);
		pthread_mutex_unlock(&sync_commandQueue);		
	}
}

/***************************************************************
  * 	      Gestionnaire de trigger relié à ALARM_LOW_BATTERY
  ***************************************************************/
 void* alarmBattery10(void *args) {

	 Controleur*    controleur;
	 /* Get the arguments */
	 controleur   = ((Controleur*)args);

	 while(true){
		 sem_wait(&alarmeLow_sync);
		 controleur->alarmBattery10();
		 sem_post(&ctrlDest_sync);
	 }
 }

 /***************************************************************
  * 	      Gestionnaire de trigger relié à ALARM_HIGH_BATTERY
  ***************************************************************/
 void* alarmBattery80(void *args) {

	 Controleur*    controleur;
	 /* Get the arguments */
	 controleur   = ((Controleur*)args);

	 while(true){
		 sem_wait(&alarmeHigh_sync);
		 controleur->alarmBattery80();
		 sem_post(&ctrlDest_sync);
	 }
 }
