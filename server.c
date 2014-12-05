#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>
#include <netdb.h>      // define structures like hostent
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#define BUFSIZE 524288
#define DATESIZE 256
#define HOSTSIZE 256
#define CACHESIZE 5242880
#define OBJECTSIZE 524288
#define MEMSIZE 41943040

struct Cache{
  char buf[OBJECTSIZE];
};

int sockfd, newsockfd;
int sockfd2;
void error(char *msg);
void sigint_handler();
void current_time(char *date);

int main(int argc, char *argv[])
{


  struct Cache c[CACHESIZE];
  

  int size;
  int portno;
  int origin_portno;
  int origin_n;
  socklen_t clilen;
     
  char buffer[BUFSIZE];
  char buffer2[BUFSIZE];

  struct sockaddr_in serv_addr, cli_addr;

  struct sockaddr_in origin_addr;
  struct hostent *origin;
  char hostname[HOSTSIZE];

  int n;

  char *ptr;
  FILE *fp;
  char date[DATESIZE];
  char savebuff[BUFSIZE];

  int shmid;
  key_t keyval = 1234;
  void *shared_memory = (void *)0;
  char *cache;
  shmid = shmget(keyval, MEMSIZE, IPC_CREAT | 0666);


  if(shmid == -1)
    error("Can't not create shared memory!\n");

  if(fp = fopen("proxy.log", "a"));
  if (argc < 2) {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }
     
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");
     
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
     
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  signal(SIGINT, sigint_handler);

  while(1){
    listen(sockfd,5);
     
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
      error("ERROR on accept");

    bzero(buffer,BUFSIZE);
    n = read(newsockfd,buffer,BUFSIZE-1);
    if (n < 0) error("ERROR reading from socket");
    //    printf("\n\n****************************\nHere is the message: %s\n",buffer);

    /*
      sockfd2 = socket(AF_INET, SOCK_STREAM, 0); //create a new socket
      if(sockfd2 < 0)
      error("ERROR opening socket");
    */

    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1)
        error("shmat failed : ");
    
    cache = (char *)shared_memory;
    if(fork() == 0) {

      shmid = shmget((key_t)1234, MEMSIZE, 0);
      if(shmid == -1)
	error("Child : Can't not create shared memory!\n");
  
      shared_memory = shmat(shmid, (vodi *) 0, 0666|IPC_CRAET);
      if(shared_memory == -1)
	error("shared memory error");

      //    sprintf(savebuff, "%s", buffer);
      strcpy(savebuff, buffer);
      ptr = strtok(savebuff, "\r\n");
      ptr = strtok(NULL, "\r\n");
      sprintf(hostname, "%s", ptr);
      ptr = strtok(hostname, " ");
      ptr = strtok(NULL, " ");

      sprintf(hostname, "%s", ptr);
      sprintf(savebuff, "%s", hostname);
      ptr = strtok(savebuff, ":");
      sprintf(hostname, "%s", ptr);
      ptr = strtok(NULL, ":");
      if(ptr == NULL)
	origin_portno = 80;
      else
	origin_portno = atoi(ptr);


      sockfd2 = socket(AF_INET, SOCK_STREAM, 0); //create a new socket
      if (sockfd2 < 0) 
        error("ERROR opening socket");


      origin = gethostbyname(hostname);
      if (origin == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
      }
    
      bzero((char *) &origin_addr, sizeof(origin_addr));
      origin_addr.sin_family = AF_INET; //initialize server's address
      bcopy((char *)origin->h_addr, (char *)&origin_addr.sin_addr.s_addr, origin->h_length);
      origin_addr.sin_port = htons(origin_portno);

      if (connect(sockfd2,(struct sockaddr *)&origin_addr,sizeof(origin_addr)) < 0)
        error("ERROR connecting");
    
      origin_n = write(sockfd2, buffer, strlen(buffer));
      if(origin_n < 0)
	error("ERROR writing to socket");
    
      bzero(buffer, BUFSIZE);
      size = 0;
      while(origin_n = read(sockfd2, buffer, BUFSIZE)) {
	write(newsockfd,buffer,origin_n);
	size+=origin_n;
	printf("child buffer : %s\n", buffer);
      }
      current_time(date);
      fprintf(fp, "%sEST: %s %s %d\n", date, inet_ntoa(cli_addr.sin_addr), hostname, size);

      cache = (char *)shared_memory;
      
      if(size <= 512){
	
      }
      close(sockfd2);
      exit(0);
    }
    
    //    printf("hostname : %s\n", hostname);
    printf("buffer : %s\n", buffer);

    close(newsockfd);
  }
  close(sockfd);
  fclose(fp);

  return 0; 
}


void current_time(char *date){
  char wday[4];
  int day;
  char month[4];
  int year;
  int hour;
  int min;
  int sec;
  time_t timer;
  char tt[9];
  struct tm *t;

  timer = time(NULL);
  t = localtime(&timer);

  year = t->tm_year + 1900;
  switch(t->tm_mon+1){
  case 1: sprintf(month, "%s", "Jan"); break;
  case 2: sprintf(month, "%s", "Feb"); break;
  case 3: sprintf(month, "%s", "Mar"); break;
  case 4: sprintf(month, "%s", "Apr"); break;
  case 5: sprintf(month, "%s", "May"); break;
  case 6: sprintf(month, "%s", "Jun"); break;
  case 7: sprintf(month, "%s", "Jul"); break;
  case 8: sprintf(month, "%s", "Aug"); break;
  case 9: sprintf(month, "%s", "Sep"); break;
  case 10: sprintf(month, "%s", "Oct"); break;
  case 11: sprintf(month, "%s", "Nov"); break;
  case 12: sprintf(month, "%s", "Dec"); break;
  }

  switch(t->tm_wday){
  case 0: sprintf(wday, "%s", "Sun"); break;
  case 1: sprintf(wday, "%s", "Mon"); break;
  case 2: sprintf(wday, "%s", "Tue"); break;
  case 3: sprintf(wday, "%s", "Wed"); break;
  case 4: sprintf(wday, "%s", "Thu"); break;
  case 5: sprintf(wday, "%s", "Fri"); break;
  case 6: sprintf(wday, "%s", "Sat"); break;    
  }

  day = t->tm_mday;
  hour = t->tm_hour;
  min = t->tm_min;
  sec = t->tm_sec;
  sprintf(tt, "%d:%d:%d", hour, min, sec);
  sprintf(date, "%s %d %s %d %s ", wday, day, month, year, tt);

  /*Sun 26 Oct 2014 02:51:02 EST: 166.104.231.100 http://cnlab.hanyang.ac.kr/ 34314*/
}


void sigint_handler(){
  close(sockfd);
  close(newsockfd);
  close(sockfd2);
  exit(0);
}
void error(char *msg)
{
  perror(msg);
  exit(1);
}
