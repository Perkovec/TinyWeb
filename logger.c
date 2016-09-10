#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUFSIZE 8096
#define ERROR 42
#define SORRY 43
#define LOG 44
#define SERVER 45

// Запись лога в файл server.log
// type - типа сообщения (ERROR, SORRY, LOG)
void logger(int type, char *s1, char *s2, int num) {
	int fd;
  char *logpath = "debug.log";
	char logbuffer[BUFSIZE * 2];

	switch (type) {
	  case ERROR: sprintf(logbuffer, "ERROR: %s:%s Errno=%d exiting pid=%d", s1, s2, errno, getpid()); break;
	  case SORRY: 
		  sprintf(logbuffer, "<HTML><BODY><H1>Web Server Sorry: %s %s</H1></BODY></HTML>\r\n", s1, s2);
  		write(num, logbuffer, strlen(logbuffer));
	  	sprintf(logbuffer, "SORRY: %s:%s", s1, s2); 
		  break;
  	case LOG: sprintf(logbuffer, "INFO: %s:%s:%d", s1, s2, num); break;
    case SERVER: 
      sprintf(logbuffer, "%s%s", s1, s2);
      logpath = "server.log";
      break;
	}	
	
  fd = open(logpath, O_CREAT| O_WRONLY | O_APPEND,0644);
	if(fd >= 0) {
		write(fd,logbuffer,strlen(logbuffer)); 
		write(fd,"\n",1);      
		close(fd);
	}
	if(type == ERROR || type == SORRY) exit(3);
}