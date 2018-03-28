#include "config.h"

int main(void) {
    // Set the status every second.
    while(1) {
        setStatus();
        sleep(1);
    }
}

int setStatus(void) {
    char status[MAXSTR + 1];
    status[0] = '\0';
    for (int i = 0; i < sizeof(countdowns) / sizeof(Countdown); i++) {
        char countdownText[MAXSTR];
        Countdown c = countdowns[i];
        snprintf(countdownText, MAXSTR, "[%s%s]   ", c.title, getCountdown(c));
        strncat(status, countdownText, MAXSTR - strlen(status));
    }
    char remainingStatus[MAXSTR + 1];
    snprintf(
        remainingStatus,
        MAXSTR - strlen(status),
        "%s -- %s; [Mem: %s]   [Bat: %2.0f %s]   [Temp: %s]",
        date(), dayPercent(), ram(),
        batteryPercent() * 100, batteryStatus(),
        getTemperature(TEMPERATURE_DIR, TEMPERATURE_INPUT)
    );
    strcat(status, remainingStatus);
    XSetRoot(status);
}

/*
 * Returns a string that represents the date.
 */
static const char * date(void){
    static char date[MAXSTR];
    time_t now = time(0);
    strftime(date, MAXSTR, TIME_FORMAT, localtime(&now));
    return date;
}

static void setTimeOfDay(struct tm *time, int hour, int minute) {
    time->tm_sec = 0;
    time->tm_min = minute;
    time->tm_hour = hour;
}

static const char * dayPercent() {
    static char ret[MAXSTR];

    time_t now = time(0);

    struct tm beginDay;
    localtime_r(&now, &beginDay);
    struct tm endDay;
    localtime_r(&now, &endDay);

    setTimeOfDay(&beginDay, DAY_START_HOUR, DAY_START_MINUTE);
    setTimeOfDay(&endDay,   DAY_END_HOUR,   DAY_END_MINUTE);

    time_t begin = mktime(&beginDay);
    time_t end = mktime(&endDay);

    int totalSecondsInDay = end - begin;

    // Seconds from beginning of day
    int progress = now - begin;
    float percentProgress = (float) progress / totalSecondsInDay;

    if (percentProgress < 0) {
        return "Day not started.";
    } else if (percentProgress > 1) {
        return "Day ended.";
    }

    snprintf(ret, MAXSTR, "%.0f%%", percentProgress * 100);
    return ret;
}

static const char * getCountdown(Countdown c) {
    static char ret[MAXSTR];
    time_t now = time(0);
    struct tm finalTimeStruct;
    memset(&finalTimeStruct, 0, sizeof(struct tm));  // Set finalTime struct to all zeros
    strptime(c.finalDatetime, COUNTDOWN_TIME_FORMAT, &finalTimeStruct);
    time_t finalTime = mktime(&finalTimeStruct);

    // Differences
    int totalSeconds = finalTime - now;
    int weeks, days, hours, minutes, seconds;
    weeks = totalSeconds / SECONDS_PER_WEEK;
    totalSeconds -= weeks * SECONDS_PER_WEEK;
    days = totalSeconds / SECONDS_PER_DAY;
    totalSeconds -= days * SECONDS_PER_DAY;
    hours = totalSeconds / SECONDS_PER_HOUR;
    totalSeconds -= hours * SECONDS_PER_HOUR;
    minutes = totalSeconds / SECONDS_PER_MINUTE;
    totalSeconds -= minutes * SECONDS_PER_MINUTE;
    seconds = totalSeconds;

    snprintf(
        ret,
        MAXSTR,
        "%dw %dd %.2d:%.2d",
        weeks, days, hours, minutes
    );
    return ret;
}

/*
 * Returns a string that contains the percentage of used ram.
 */
static const char * ram(void){
    static char ret[MAXSTR + 1];
    struct sysinfo s;
    sysinfo(&s);
    double usedRam = s.totalram - s.freeram;
    double percentRam = usedRam / s.totalram;
    //double percentBufferRam = (double)s.bufferram / s.totalram;
    //double percentSharedRam = (double)s.sharedram / s.totalram;
    snprintf(ret, MAXSTR,
        "%2.0f%%",
        percentRam * 100);
    return ret;
}

static float batteryPercent(void) {
    FILE *fd;
    int charge_now = getIntFromFile(BATTERY_DIR"charge_now");
    int charge_full = getIntFromFile(BATTERY_DIR"charge_full");
    return (float)charge_now / charge_full;
}

static const char * batteryStatus() {
    char *status = getStrFromFile(BATTERY_DIR"status", 15);
    const char *ret;
    if (strncmp(status, "Discharging", 11) == 0) {
        ret = "-";
    } else if (strncmp(status, "Charging", 8) == 0) {
        ret = "+";
    } else if (strncmp(status, "Full", 4) == 0) {
        ret = "=";
    } else {
        ret = "?";
    }
    free(status);
    return ret;
}

static char * getStrFromFile(char *path, int maxLength) {
    FILE *fd;
    fd = fopen(path, "r");
    if (fd == NULL) {
        fprintf(stderr, "Could not open path: %s", path);
        return NULL;
    }
    char *val = malloc(maxLength + 1);
    if (!fgets(val, maxLength, fd)) {
        fprintf(stderr, "Unable to get string from: %s", path);
        free(val);
        val = NULL;
    }
    fclose(fd);
    return val;
}

int getIntFromFile(char *path) {
    int val = -1;
    FILE *fd;
    fd = fopen(path, "r");
    if (fd == NULL) {
        fprintf(stderr, "Could not open path: %s", path);
        return -1;
    }
    if (!fscanf(fd, "%d", &val)) {
        fprintf(stderr, "Could not scan integer from: %s", path);
    }
    fclose(fd);
    return val;
}

/*
 * getTemperature("/sys/class/hwmon/hwmon0/device", "temp1_input");
 *
 * Helper function from dwm.suckless.org
 */
char * getTemperature(char *base, char *sensor) {
	char *co;

	co = readfile(base, sensor);
	if (co == NULL)
		return smprintf("");
	return smprintf("%02.0f°C", atof(co) / 1000);
}
char * smprintf(char *fmt, ...) {
	va_list fmtargs;
	char *buf = NULL;

	va_start(fmtargs, fmt);
	if (vasprintf(&buf, fmt, fmtargs) == -1){
		fprintf(stderr, "malloc vasprintf\n");
		exit(1);
    }
	va_end(fmtargs);

	return buf;
}
char * readfile(char *base, char *file) {
	char *path, line[513];
	FILE *fd;

	memset(line, 0, sizeof(line));

	path = smprintf("%s/%s", base, file);
	fd = fopen(path, "r");
	if (fd == NULL)
		return NULL;
	free(path);

	if (fgets(line, sizeof(line)-1, fd) == NULL)
		return NULL;
	fclose(fd);

	return smprintf("%s", line);
}


/*
 * Updates the status bar for DWM.
 */
static void XSetRoot(const char *name){
        Display *display;

        if (( display = XOpenDisplay(0x0)) == NULL ) {
                fprintf(stderr, "[dwm-status] cannot open display!\n");
                exit(1);
        }

        XStoreName(display, DefaultRootWindow(display), name);
        XSync(display, 0);

        XCloseDisplay(display);
}

