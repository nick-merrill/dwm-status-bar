/*
 * Updates the status bar for DWM.
 */

typedef struct Countdowns {
    char title[30];
    char finalDatetime[30];
} Countdown;

#define VERSION "1.0"

#define _XOPEN_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#define MAXSTR 1024
#define SECONDS_PER_MINUTE 60
#define SECONDS_PER_HOUR (SECONDS_PER_MINUTE * 60)
#define SECONDS_PER_DAY (SECONDS_PER_HOUR * 24)
#define SECONDS_PER_WEEK (SECONDS_PER_DAY * 7)


static const char * date(void);
static const char * getCountdown(Countdown c);
static const char * getuname(void);
static const char * ram(void);
static float batteryPercent(void);
static const char * batteryStatus(void);
static int getIntFromFile(char *fileName);
static char * getStrFromFile(char *fileName, int maxLength);
static void XSetRoot(const char *name);
static int setStatus(void);
char * getTemperature(char *base, char *sensor);
char * readfile(char *base, char *file);
char * smprintf(char *fmt, ...);
static const char * dayPercent();

