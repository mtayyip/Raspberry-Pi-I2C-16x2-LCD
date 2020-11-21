
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "lcd16x2.h"
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c-dev.h>

#define DATA 1
#define BACKLIGHT 8
#define CMD_PERIOD 4100
#define PULSE_PERIOD 500


static int file_i2c = -1;
static int iBackLight = BACKLIGHT;

static void WriteCommand(unsigned char ucCMD)
{
	unsigned char uc;

	uc = (ucCMD & 0xf0) | iBackLight; // most significant nibble sent first
	write(file_i2c, &uc, 1);
	usleep(PULSE_PERIOD); // manually pulse the clock line

	uc |= 4; // enable pulse
	write(file_i2c, &uc, 1);
	usleep(PULSE_PERIOD);

	uc &= ~4; // toggle pulse
	write(file_i2c, &uc, 1);
	usleep(CMD_PERIOD);

	uc = iBackLight | (ucCMD << 4); // least significant nibble
	write(file_i2c, &uc, 1);
	usleep(PULSE_PERIOD);

	uc |= 4; // enable pulse
	write(file_i2c, &uc, 1);
	usleep(PULSE_PERIOD);

	uc &= ~4; // toggle pulse
	write(file_i2c, &uc, 1);
	usleep(CMD_PERIOD);
} 


int lcd1602Init(char *fileName, int iAddr)
{
	int rc;

	file_i2c = open(fileName, O_RDWR);
	if (file_i2c < 0)
	{
		fprintf(stderr, "Error opening i2c device : %s\n",fileName);
		return 1;
	}
	rc = ioctl(file_i2c, I2C_SLAVE, iAddr);
	if (rc < 0)
	{
		close(file_i2c);
		fprintf(stderr, "Error setting I2C device address : %d\n",iAddr);
		return 1;
	}

	iBackLight = BACKLIGHT; // turn on backlight
	WriteCommand(0x02); 	// Set 4-bit mode of the LCD controller
	WriteCommand(0x28); 	// 2 lines, 5x8 dot matrix
	WriteCommand(0x0c); 	// display on, cursor off
	WriteCommand(0x06); 	// inc cursor to right when writing and don't scroll
	WriteCommand(0x80); 	// set cursor to row 1, column 1
	lcd1602Clear();	    	// clear the memory
	return 0;
}


int lcd1602WriteString(char *text)
{
	int i = 0;
	unsigned char ucTemp[2];
	
	if (file_i2c < 0 || text == NULL)
		return 1;

	while (i<16 && *text)
	{
		ucTemp[0] = iBackLight | DATA | (*text & 0xf0);
		write(file_i2c, ucTemp, 1);
		usleep(PULSE_PERIOD);

		ucTemp[0] |= 4; // pulse E
		write(file_i2c, ucTemp, 1);
		usleep(PULSE_PERIOD);

		ucTemp[0] &= ~4;
		write(file_i2c, ucTemp, 1);
		usleep(PULSE_PERIOD);

		ucTemp[0] = iBackLight | DATA | (*text << 4);
		write(file_i2c, ucTemp, 1);
		ucTemp[0] |= 4; // pulse E
		write(file_i2c, ucTemp, 1);

		usleep(PULSE_PERIOD);
		ucTemp[0] &= ~4;
		write(file_i2c, ucTemp, 1);

		usleep(CMD_PERIOD);
		text++;
		i++;
	}
	return 0;
}


int lcd1602SetCursor(int x, int y)
{
	unsigned char cCmd;

	if (file_i2c < 0 || x < 0 || x > 15 || y < 0 || y > 1)
		return 1;

	cCmd = (y==0) ? 0x80 : 0xc0;
	cCmd |= x;
	WriteCommand(cCmd);
	return 0;
}


int lcd1602Control(int bBacklight, int bCursor, int bBlink)
{
	unsigned char ucCMD = 0xc; // display control

	if (file_i2c < 0)
		return 1;

	iBackLight = (bBacklight) ? BACKLIGHT : 0;
	if (bCursor)
		ucCMD |= 2;
	if (bBlink)
		ucCMD |= 1;

	WriteCommand(ucCMD);
	return 0;
}


int lcd1602Clear(void)
{
	if (file_i2c < 0)
		return 1;
	WriteCommand(0x01); // clear the screen
	return 0;
}


void lcd1602Shutdown(void)
{
	iBackLight = 0; 	// turn off backlight
	WriteCommand(0x08); // turn off display, cursor and blink	
	close(file_i2c);
	file_i2c = -1;
}