
#ifndef LCD16x2_H
#define LCD16x2_H

int lcd1602Init(char *fileName, int iAddr);
int lcd1602SetCursor(int x, int y);
int lcd1602Control(int bBacklight, int bCursor, int bBlink);
int lcd1602WriteString(char *szText);
int lcd1602Clear(void);
void lcd1602Shutdown(void);

#endif // LCD16x2_H
