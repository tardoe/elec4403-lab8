#include "eyebot.h"
#include <math.h>
#include <stdio.h>
#include <bool.h>

#define SEARCH_STATE 0
#define TARGETING_STATE 1
#define RETURNING_STATE 2


#define RED_PIXEL_THRESHOLD 200

int homeLocationX = 0;
int homeLocationY = 0;
int homeLocatoinPhi = 0;

int currentState = 0;

int redFound = 0;
bool redMaximised = false;

//BYTE *currentImage;

int servoPos = 128;
int increment = 1;
BYTE img[QVGA_SIZE];

//float Hue[X_1*Y_1], Set[X_1*Y_1, Int[X_1*Y_1]];
int redThreshhold = 200;

int redFoundServoPosition = 0;


void setup()
{
	//mark this location as home
	VWGetPosition(&homeLocationX, &homeLocationY, &homeLocatoinPhi);

	//do any other initialisation
	currentState = 0;

	CAMInit(QVGA);
	SERVOSet(1, servoPos);
}

int searchForRed(BYTE img)
{
	int numRed = 0;
	int red;
	int blue;
	
	for(int i = 0; i < X_1; i++){
		for(int j = 0; j < Y_1; j++) {
			red = img[(i+j*X_1)*3];
			blue = img[(i+j*X_1)*3+2];
			if (red > 190 && blue < 40) numRed++;
			if(numRed == redThreshhold) return numRed;
		}
	}
	
	return numRed;
	
}

int scanForRed(BYTE img)
{
	int numRed = 0;
	int red;
	int blue;
	servoPos = servoPos + increment;
	SERVOSet(1, servoPos);
	
	int binary[QQVGA_X/3];
	
	for()
	
	for(int i = 0; i < X_1; i++){
		for(int j = 0; j < Y_1; j++) {
			red = img[(i+j*X_1)*3];
			blue = img[(i+j*X_1)*3+2];
			if (red > 190 && blue < 40) numRed++;
		}
	}
		
	if (servoPos == 255 || servoPos == 0) increment = increment * -1;
	
	return numRed;
	
}

int whileDriving()
{
	// while(VWDriveDone())
	// {
	// 	printf("waiting to be cleared");
	// 	LCDPrintf("waiting to be cleared \n");
	// 	LCDSetPos(0,0);
	// }

	while(!VWDriveDone())
	{
		//search for the red cans
		BYTE image;
		CAMGet(&image);


		redFound = searchForRed(image);

		if(redFound > RED_PIXEL_THRESHOLD)
		{
			currentState = TARGETING_STATE;
			break;
		}

	}
}

int drivingWait()
{
	// while(VWDriveDone())
	// {
	// 	printf("waiting to be cleared");
	// 	LCDPrintf("waiting to be cleared \n");
	// 	LCDSetPos(0,0);
	// }

	while(!VWDriveDone())
	{

	}
}

int getFrontDistance()
{
	int dist = PSDGet(2);
	int mmDist = 97.19 * exp(-0.053 * dist);

	return mmDist;
}

void searchState()
{
	while(true)
	{
		//drive forward 10cm, check for wall, repeat

		VWStraight(100, 100);
		whileDriving();

		if(currentState != SEARCH_STATE)
		{
			return; // because we need to re-start the main run loop in a new condition.
		}

		//check how close the nearest object is.
		int mmDist = getFrontDistance();

		if(mmDist < 100)	//only get 10cm away
		{
			//need to make a random turn
			int randomRad = (int)(rand() * 620); //roughly 360 degree in centi-radians
			VWTurn(randomRad, 100);
			whileDriving();
		}
		else
		{
			continue;
		}
	}
}


void targetingState()
{
	//red was found, use the servo on the camera to sweep and find the best direction.
	int maxRed = 0;

	for(int i = 0; i < 255; i++)
	{
		BYTES *image;
		CAMGet(&image);
		int numRed = scanForRed(image);

		if(numRed > maxRed)
		{
			maxRed = numRed;
		}
	}

	// we should now have the global variable containing the servo position for the most red.
	int angle = 0;


	if(redFoundServoPosition == 128)
	{
		angle = 0;

	}
	else if(redFoundServoPosition < 128)
	{
		angle = 128 / 90 * redFoundServoPosition;
		angle = 360 - angle;
	}
	else if(redFoundServoPosition > 128)
	{
		angle = 128 / 90 * redFoundServoPosition;
	}

	VWTurn(angle, 100);
	drivingWait();


	//we should now be facing the red can, do a distance check.
	int mmDist = getFrontDistance();

	// when distance is close enough (~5cm or so),
	if(mmDist > 50)
	{
		//drive the difference
		VWStraight(mmDist - 50, 100)
		drivingWait();
	}

	
	// turn on electromagnet and drive the remaining distance.
	// magentOn();


	// when confirmed that magnet is on, change state.
	currentState = RETURNING_STATE;

}


void returningState()
{

	//set course for home.
	VWSetPosition(homeLocationX, homeLocationY, homeLocatoinPhi);
	drivingWait();

	// do something to indicate that goal is completed - i dunno... beep?


}


int main()
{

	setup();

	while(true)
	{
		switch(currentState)
		{
			case 0:
				//drive and search for a wall and then avoid it.
				//scan for red objects, if found, change state to targeting()
				searchState();
				break;
			case 1:
				//calculate location and distance of red object, drive towards it.
				targetingState();
				break;
			case 2:
				//grab the can and start heading home
				returningState();
				break;
		}

	}


}


