#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> // for open()
#include <unistd.h> // for close()
#include <time.h>    // time()
#include <sys/types.h>
#include <signal.h>

// NOTE1: Remember that every time you writes a message to a pipe or a FIFO, 
// you should use fflush() to ensure the message being correctly passed out.
#define FOUR_PLAYER 4
#define MESSAGE_MAX 20

int judge_num;
int player_num;

void myswap(char* s0, char* s1) {
   char tmp = *s0;
   *s0 = *s1;
   *s1 = tmp;   
}
void myitoa (int n,char s[])
{
   int i, j, sign;
   if((sign = n) < 0)
      n = -n;
   i = 0;
   int _size = 1;
   do {
      s[ i++ ] = n % 10 + '0';
      _size++;
   }
   while ( (n /= 10) > 0);
   if(sign < 0)
      s[ i++ ] = '-';
   s[ i ] = '\0';
   if(_size == 6) { //e.g.12345
      myswap(&s[0], &s[4]);
      myswap(&s[1], &s[3]);
   } else if(_size == 5) { //e.g. 1234
      myswap(&s[0], &s[3]);
      myswap(&s[1], &s[2]);
   } else if (_size == 4) { //e.g. 123
      myswap(&s[0], &s[2]);
   } else if(_size == 3) { //e.g. 12
      myswap(&s[0], &s[1]);
   } else if(_size == 2) {
      // do nothin
   }
}
void parse4players(char message[], int _p[]) {

   srand(time(NULL));
   int _pickedIdx = ( rand() % player_num );
   int _ids[FOUR_PLAYER];

   for(int i = 0; i < FOUR_PLAYER; i++)
      _ids[i] = 0;
   for(int i = 0; i < FOUR_PLAYER; i++) {
      while( _p[ _pickedIdx ] == 0)
         _pickedIdx = ( rand() % player_num); 

      _ids[ i ] = _pickedIdx + 1;
      _p[ _pickedIdx ] = 0;
   }


   for(int i = 0; i < FOUR_PLAYER; i++) {
      int aInt = _ids[ i ];
      char str[3];
      memset(str, 0, sizeof str);

      myitoa(aInt, str);
      strcat(message,  str );
      if(i != FOUR_PLAYER - 1)
         strcat(message, " ");     
   }
}

void forkJudge(int i, int pipefd[], int _p[]) {
   if(i == 0) return;
   pid_t cpid = fork();
   int status;
   if (cpid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);

   } else if (cpid == 0) { // child process

      char myjudge_id[3];
      memset(myjudge_id, 0, sizeof(myjudge_id));
      myitoa(i, myjudge_id);
      char w_pfd[3];
      memset(w_pfd, 0, sizeof(w_pfd));
      myitoa(pipefd[1], w_pfd);

      execl("./judge", myjudge_id, w_pfd, NULL);

      _exit(EXIT_SUCCESS);
   } else { // parent process

      char message[MESSAGE_MAX];
      memset(message, 0, sizeof message);

      parse4players(message, _p);
      write(pipefd[1], message, sizeof(message));
      forkJudge(--i, pipefd, _p);
      wait(&status);
   }
}
int main(int argc, char *argv[]) {
	
	if(argc != 3) {
		fprintf(stderr, "USAGE: ./big_judge [judge_num] [player_num]\n");
      	exit(EXIT_FAILURE);
	}

	judge_num = atoi(argv[1]);
	player_num = atoi(argv[2]);

	if(judge_num < 1 || judge_num > 12 || player_num < 4 || player_num > 20) {
		fprintf(stderr, "ERROR: invalid num\n");
      	exit(EXIT_FAILURE);		
	}
	if(player_num / judge_num != FOUR_PLAYER) {
		fprintf(stderr, "ERROR: simplified version invalid\n");
      	exit(EXIT_FAILURE);		
	}
	int _p[ player_num ]; //_p denotes available players
	int scores[ player_num ];
	for(int i = 0; i < player_num; i++) {
		_p[i] = 1; // 1 denotes available, i.e. waiting for joining a game
		scores[i] = 0;
	}
   int pipefd[2] = {0, 0};
   if (pipe(pipefd) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
   }
   int tmp_num = judge_num;
   if( dup2( pipefd[0], STDIN_FILENO ) < 0 ){
      perror( "dup2()" );
      exit(EXIT_FAILURE);
   }

   forkJudge(tmp_num, pipefd, _p);

   char res[MESSAGE_MAX];
   memset(res, 0, sizeof(res));
   read(pipefd[0], res, sizeof(res)); //pipefd[0] is STDIN
   fflush(stdout);

	return 0;
}