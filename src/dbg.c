#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

// ref: http://stackoverflow.com/questions/7411301/how-to-introduce-date-and-time-in-log-file

char* get_formatted_time(void) {
	time_t rawtime;
	struct tm* timeinfo = 0;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	static char _retval[20];
	strftime(_retval, sizeof (_retval), "%Y-%m-%d %H:%M:%S", timeinfo);
	return _retval;
}

void log_pid(const char *msg) {
	char buff[128];
	int _fd_log = 0;
	//	_fd_log = open("/tmp/zeroxy.log", O_CREAT | O_RDWR | O_APPEND);
	//	sprintf(buff, "[%d] %s\n", getpid(), msg);
	//	write(_fd_log, buff, strlen(buff));
	//	close(_fd_log);
}