
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <sys/time.h>
#include <time.h>

#include <curl/curl.h>

#include "readfile.h"

char *engine0 = "/home/lichess/src/Stockfish/src/stockfish";

#define LICHESS_CGIURL "http://lichess.org/cgi-bin/AIbot"

#define HEARTBEAT 20

enum { STDIN, STDOUT };

int main(int argc, char *argv[]) {

  char buf[4096];
  ssize_t bytes_read;

  int fd[2];
  int gd[2];

  CURL *curl;
  CURLcode res;

  pid_t pid;

  int retval;

  curl_global_init(CURL_GLOBAL_ALL);

  retval = pipe(fd);
  if (retval == -1) {
    perror("send pipe");
    return -1;
  }

  retval = pipe(gd);
  if (retval == -1) {
    perror("recv pipe");
    return -1;
  }

  curl = curl_easy_init();
  if (curl) { 

    pid = fork();

    if (pid==-1) {
      printf("Trouble with fork. Could not start UCI engine #0, leaving.\n");
      return -1;
    }

    if (!pid) {

      // go infinite
      // go movetime 5000

      dup2(STDIN, fd[0]);
      close(fd[1]);
      close(gd[0]);

      printf("Forked child process.\n");

      execl(engine0, engine0, NULL);

      printf("Invoking curl heartbeat.\n");

      curl_easy_setopt(curl, CURLOPT_URL, "http://example.org/cgi-bin/AIbot");
      /* Now specify the POST data */ 
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{ num_engines: 1 }");
 
      /* Perform the request, res will get the return code */ 
      res = curl_easy_perform(curl);
      /* Check for errors */ 
      if(res != CURLE_OK)
	fprintf(stderr, "curl_easy_perform() failed: %s\n",
		curl_easy_strerror(res));
      
    }

    else {

      struct timespec now, then, start;

      char *start_cmds[] = {
	"position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
	,"position startpos moves e2e4"
	,"setoption name Threads value 4"
	,"uci"
	,"go depth 20"
	,"quit"
      };

      long int num_cmds = sizeof(start_cmds) / sizeof(long int);

      long int n;

      close(fd[0]);
      close(gd[1]);

      printf("Using parent process. CHILD pid spawned as %d\n", pid);

      usleep(5000000);

      for (n = 0; n < num_cmds; n++) { 
	write(fd[1], start_cmds[n], strlen(start_cmds[n]));
      }

      clock_gettime(CLOCK_REALTIME, &start);

      waitpid(pid, NULL, 0);
      
      write(1, buf, bytes_read);

      for (;;) {
	
	clock_gettime(CLOCK_REALTIME, &now);

	usleep(16333);

      }

    }

    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
    
  return 0;

}
