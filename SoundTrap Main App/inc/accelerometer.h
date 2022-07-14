/*
 * accelerometer.h
 *
 *  Created on: 1/05/2014
 *      Author: jatk009
 */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

typedef enum {G2=0, G4=1, G8=2} accelRange;

typedef struct {
	Int16 x;
	Int16 y;
	Int16 z;
} TaccelData;

int	accelRead(TaccelData *d);
int accelSleep();
int accelWakeup();
int accelTest();
Ptr accelInit(int id, sensorCallback callback);
int accelTrigger( Ptr p1, Ptr p2 );

#endif /* ACCELEROMETER_H_ */
