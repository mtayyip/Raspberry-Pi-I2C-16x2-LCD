//
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "lcd16x2.h"


void usage(char *arg)
{
	printf("Usage : %s <I2C Device> <I2C Address>\r\n",arg);
	printf("\tExample : %s /dev/i2c-1 0x27\r\n",arg);
	printf("\tPress CTRL-C to Close the Application\r\n");
}


static void handler (int signum)
{
	if (signum == SIGINT) 
	{
		lcd1602Clear();
		lcd1602Shutdown();
		exit(1);
	}
}


void getTemp(char* output)
{
    float systemp, millideg;
    FILE *thermal;
    int n;
    

	thermal = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    n = fscanf(thermal, "%f", &millideg);
    fclose(thermal);
    systemp = (int)(millideg / 10.0f) / 100.0f;
    sprintf(output, "%.2f C", systemp);
}


void getTime(char* output)
{
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	sprintf(output, "[%02d:%02d:%02d]",timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}


int main(int argc, char *argv[])
{

	if(argc !=3)
	{
		usage(argv[0]);
		return -1;
	}

	struct sigaction act;
	memset (&act, '\0', sizeof(act));
	act.sa_handler = &handler;
	if (sigaction(SIGINT, &act, NULL) < 0) 
	{
		perror ("sigaction");
		return -1;
	}


	int retVal;
	char temp[50];
	char timeVal[50];
	retVal = lcd1602Init(argv[1], strtol(argv[2],NULL,16));

	if(retVal)
	{
		printf("LCD 16x2 init failed...\n");
		return 0;
	}


	while(1)
	{
		lcd1602SetCursor(1,0); // x,y
		lcd1602WriteString("Muhammet Tayyip");
		lcd1602SetCursor(5,1);
		lcd1602WriteString("CANKAYA");
		lcd1602Control(1,0,0);
		sleep(2); lcd1602Clear();

		lcd1602SetCursor(3,0);
		lcd1602WriteString("Raspberry");
		lcd1602SetCursor(6,1);
		lcd1602WriteString("Pi 3");
		lcd1602Control(1,0,0);
		sleep(2); lcd1602Clear();

		lcd1602SetCursor(1,0);
		lcd1602WriteString("CPU Temperature");
		lcd1602SetCursor(5,1);
		getTemp(temp);
		lcd1602WriteString(temp);
		lcd1602Control(1,0,0);
		sleep(2); lcd1602Clear();

		lcd1602SetCursor(2,0);
		lcd1602WriteString("Current Time");
		lcd1602SetCursor(3,1);
		getTime(timeVal);
		lcd1602WriteString(timeVal);
		lcd1602Control(1,0,0);
		sleep(2); lcd1602Clear();
	}

	return 0;
}