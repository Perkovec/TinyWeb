#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "mimeTypes.h"
#include "logger.h"

#ifndef SIGCLD
#   define SIGCLD SIGCHLD
#endif

#define BUFSIZE 8096
#define ERROR 42
#define SORRY 43
#define LOG 44
#define SERVER 45

#define DEBUGLOG 0 // 1 - on, 0 - off

void web(int fd, int hit, char *ip) {
	int j, file_fd, buflen, len;
	long i, ret;
	char * fstr;
	static char buffer[BUFSIZE + 1];

	ret = read(fd, buffer, BUFSIZE); 
	if (DEBUGLOG == 1 && (ret == 0 || ret == -1)) {
		logger(SORRY, "failed to read browser request", "", fd);
	}
	if (ret > 0 && ret < BUFSIZE) {
		buffer[ret] = 0;	
  } else {
    buffer[0] = 0;
  }

  if (DEBUGLOG == 1)
	  logger(LOG, "request", buffer, hit);

	if (strncmp(buffer, "GET ", 4) && strncmp(buffer, "get ", 4) && DEBUGLOG == 1)
		logger(SORRY, "Only simple GET operation supported", buffer, fd);

	for (i = 4; i < BUFSIZE; i++) { 
		if (buffer[i] == ' ') { 
			buffer[i] = 0;
			break;
		}
	}

	for (j = 0; j < i - 1; j++) 	
		if (buffer[j] == '.' && buffer[j + 1] == '.' && DEBUGLOG == 1)
			logger(SORRY, "Parent directory (..) path names not supported", buffer, fd);

	if (!strncmp(&buffer[0], "GET /\0", 6) || !strncmp(&buffer[0], "get /\0", 6)) 
		strcpy(buffer, "GET /index.html");

	buflen = strlen(buffer);
	fstr = (char *)0;
	for (i = 0; mimeTypes[i].ext != 0; i++) {
		len = strlen(mimeTypes[i].ext);
		if (!strncmp(&buffer[buflen - len], mimeTypes[i].ext, len)) {
			fstr = mimeTypes[i].filetype;
			break;
		}
	}
	if (fstr == 0 && DEBUGLOG == 1) logger(SORRY, "file extension type not supported", buffer, fd);

  file_fd = open(&buffer[5], O_RDONLY);
	if (file_fd == -1 && DEBUGLOG == 1) 
		logger(SORRY, "failed to open file", &buffer[5], fd);

  if (DEBUGLOG == 1)
  	logger(LOG, "SEND", &buffer[5], hit);
  
  char header[100];
  for (i = 4; i < BUFSIZE; i++) { 
		if (buffer[i] == '\n') { 
			memcpy(header, &buffer[0], i);
			break;
		}
	}   

	sprintf(buffer, "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
	write(fd, buffer, strlen(buffer));

  char logoutput[200];
  char reqtime[70];
  time_t nowtime;
  time(&nowtime);
  struct tm *timeinfo = localtime (&nowtime);
  strftime(reqtime, sizeof reqtime, "%d/%b/%Y:%H:%M:%S %z", timeinfo); 
  sprintf(logoutput, " - - [%s] \"%s\" 200 %ld", reqtime, header, strlen(buffer));

  if (DEBUGLOG == 1)
    logger(SERVER, ip, logoutput, 0);

	while ((ret = read(file_fd, buffer, BUFSIZE)) > 0 ) {
	  write(fd, buffer, ret);
	}
  #ifdef LINUX
	  sleep(1);
  #endif
	exit(1);
}


int main(int argc, char **argv) {
	int i, port, pid, listenfd, socketfd, hit;
	socklen_t length;
	static struct sockaddr_in cli_addr; 
	static struct sockaddr_in serv_addr;

	if (argc < 3 || argc > 4 || !strcmp(argv[1], "-?")) {
	  printf("usage: main [port] [server directory] [--background]"
	    "\tExample: main 8080 ./\n\n"
	    "\tOnly Supports:");

		for (i = 0; mimeTypes[i].ext != 0; i++)
		  printf(" %s", mimeTypes[i].ext);

		printf("\n\tNot Supported: directories / /etc /bin /lib /tmp /usr /dev /sbin \n");
		exit(0);
	}

	if (!strncmp(argv[2], "/", 2 ) || !strncmp(argv[2], "/etc", 5) ||
	    !strncmp(argv[2], "/bin", 5) || !strncmp(argv[2], "/lib", 5) ||
	    !strncmp(argv[2], "/tmp", 5) || !strncmp(argv[2], "/usr", 5) ||
	    !strncmp(argv[2], "/dev", 5) || !strncmp(argv[2], "/sbin",6)) {
		printf("ERROR: Bad top directory %s, see server -?\n", argv[2]);
		exit(3);
	}
	if (chdir(argv[2]) == -1) { 
		printf("ERROR: Can't Change to directory %s\n", argv[2]);
		exit(4);
	}

  if (argv[3] && (strcmp(argv[3], "-b") || strcmp(argv[3], "--background"))) {
    if (fork() != 0)
	    return 0;
  }

	signal(SIGCLD, SIG_IGN); 
	signal(SIGHUP, SIG_IGN);

	for (i = 0; i < 32; i++)
		close(i);	
	setpgrp();	

  if (DEBUGLOG == 1)
  	logger(LOG, "http server starting", argv[1], getpid());

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0 && DEBUGLOG == 1)
		logger(ERROR, "system call","socket", 0);
	port = atoi(argv[1]);
	if (DEBUGLOG == 1 && (port < 0 || port > 60000))
		logger(ERROR,"Invalid port number try [1,60000]", argv[1], 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 && DEBUGLOG == 1)
		logger(ERROR,"system call","bind",0);
	if (listen(listenfd, 64) < 0 && DEBUGLOG == 1)
		logger(ERROR,"system call","listen",0);

	for (hit = 1; ;hit++) {
		length = sizeof(cli_addr);
    socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length);
		if (DEBUGLOG == 1 && socketfd < 0)
			logger(ERROR,"system call","accept",0);

		if ((pid = fork()) < 0) {
      if (DEBUGLOG == 1)
  			logger(ERROR,"system call","fork",0);
		} else {
			if (pid == 0) {
				close(listenfd);
        char ipinput[INET_ADDRSTRLEN];
        char *ip = (char *)inet_ntop(AF_INET, &(cli_addr.sin_addr), ipinput, INET_ADDRSTRLEN);
				web(socketfd, hit, ip);
			} else {
				close(socketfd);
			}
		}
	}
}