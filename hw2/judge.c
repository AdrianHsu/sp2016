#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>    // time()
#include <signal.h>

#define FOUR_PLAYER 4
#define MESSAGE_MAX 20
#define MYRAND_MAX 65536

int judge_id;
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


void forkPlayer(int player_index, int myfifo, int rand_key) {
   pid_t cpid = fork();

   if (cpid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);

   } else if (cpid == 0) { // child
      char pIdx[1];
      pIdx[0] = 'A' + player_index - 1;

      char myjudge_id[3];
      memset(myjudge_id, 0, sizeof(myjudge_id));
      myitoa(judge_id, myjudge_id);
      char random_key[6];
      memset(random_key, 0, sizeof(random_key));
      myitoa(rand_key, random_key);

      // printf("myjudge_id: %s\n", myjudge_id);
      // printf("random_key: %s\n", random_key);      
      execl("./player", myjudge_id, pIdx, random_key, NULL);

   } else { //parent

   }
}


// "4 13 1 2" -> 4, 13, 1, 2
void parse4players(char message[], int _ids[]) {
   
   int i = 0;
   char *s = strtok(message, " ");
   while(s != NULL) {
      _ids[i] = atoi(s);
      i++;
      s = strtok(NULL, " ");
   }
}
void judgefifo(char first_str[], int judge_id) {

   char str1[2];
   char str2[3];
   strcat(first_str, "./tmp/judge"); 

   if(judge_id < 10) {
      str1[0] = judge_id + '0';
      str1[1] = '\0';
      strcat(first_str, str1); 
   } else {
      memset(str2, 0, sizeof(str2));
      myitoa(judge_id, str2);
      strcat(first_str, str2); 
   }

   strcat(first_str, ".FIFO"); 
   // printf("first_str: %s\n", first_str);
}
void playerfifo(char first_str[], int judge_id) {

   char str1[2];
   char str2[3];
   strcat(first_str, "./tmp/judge"); 

   if(judge_id < 10) {
      str1[0] = judge_id + '0';
      str1[1] = '\0';
      strcat(first_str, str1); 
   } else {
      memset(str2, 0, sizeof(str2));
      myitoa(judge_id, str2);
      strcat(first_str, str2); 
   }
   strcat(first_str, "_");
   char t[1];
   t[0] = 'A' + judge_id - 1;
   strcat(first_str, t);
   
   strcat(first_str, ".FIFO"); 
   // printf("first_str: %s\n", first_str);
}
int main(int argc, char *argv[]) {

   // judge.c (./judge [judge_id])
   if(argc != 1) {
      fprintf(stderr, "USAGE: ./judge [judge_id]\n");
      exit(EXIT_FAILURE);
   }
   judge_id = atoi(argv[0]);
   // big_judge called -> $ ./judge 1 
   // The judge will create: 
   // judge1.FIFO 
   // judge1_A.FIFO 
   // judge1_B.FIFO 
   // judge1_C.FIFO 
   // judge1_D.FIFO 
   int pipefd[2];
   if (pipe(pipefd) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
   }

// The judge should read from standard input
// waiting for the big_judge to assign four players in.
   char message[MESSAGE_MAX];
   if (read(STDIN_FILENO, message, sizeof message) > 0) {
      // printf("%s\n", message);
      // fflush(stdout);
      //write(STDOUT_FILENO, &buf, 20);
      // fprintf(stderr, "!!");
   }

   int _ids[ FOUR_PLAYER ];
   for(int i = 0; i < FOUR_PLAYER; i ++)
      _ids[i] = 0;
   // The big_judge sends judge 1 
   // (judge 1 reads from standard input): 1 2 3 4  
   parse4players(message, _ids);
   // for(int i = 0; i < FOUR_PLAYER; i ++)
   //    printf("%d\n", _ids[i]);

   // create a FIFO named judge[judge_id].FIFO, such as judge1.FIFO
   // to read responses from the players

   char my1stfifo[MESSAGE_MAX];
   memset(my1stfifo, 0, sizeof(my1stfifo));
   judgefifo(my1stfifo, judge_id);
   // printf("first_str: %s\n", my1stfifo);

   // create four FIFOs named judge[judge_id]_A.FIFO...etc
   // to write messages to the players in the competition

   char myStrfifo[FOUR_PLAYER][MESSAGE_MAX];
   int myfifo[4];

   for(int i = 0; i < FOUR_PLAYER; i++) {
      myfifo[i] = 0;
      memset(myStrfifo[i], 0, sizeof(myStrfifo[i]));
      playerfifo(myStrfifo[i], judge_id);
      // printf("str: %s\n", myfifo[i]);
      // myfifo[i] = mkfifo(myStrfifo[i]);
      int rand_key = ( rand() % MYRAND_MAX );

      forkPlayer(i + 1, myfifo[i], rand_key);
   }

// After knowing the players, 
// the judge forks four child processes

   return 0;
}


// The judge executes: 
// $ ./player 1 A 9 
// $ ./player 1 B 2013 
// $ ./player 1 C 10000 
// $ ./player 1 D 65535 

// run exec() to execute the player programs
// start 20 game rounds.

// The executable file of a player named "player", 
// are placed in the same directory containing "big_judge" and "judge"

// In each round, 

// except for the first round,

// Round 2, judge 1 sends player 1 through judge1_A.FIFO:
// 3 5 5 1 

// the judge tells every player the result of the previous round
// via specific FIFO

// help them decide what number to tell in the next round. (AH: IGNORED)

// After giving out the message
// the judge has to collect numbers coming from the four players.
// format: [player_index] [random_key] [number_choose] 

// The judge can ask the players in turn.

// If a player does not respond for more than 3 seconds,
// the judge just assumes that the player doesn't respond any number 
// in this round and the following rounds. 

// the judge should skip him and let him get 0 point in this round and the following rounds
// until the competition ends. (i.e. punishment)

// The judge accumulates each player’s points.

// NOTE2: For the judge, remember to clear the FIFO before a new competition begins, 
// in case any player died in last competition and didn't read all message.

// After 20 rounds, the judge should output to "standard output"
// [p1_id] [p1_rank]
// [p2_id] [p2_rank]...etc

// Judge 1 writes the result to standard output (sending to big_judge): 
// 1 4 
// 2 2 
// 3 1 
// 4 3 


// the ranks are ordered from 1 to 4.

// If multiple players get the same points in a competition, 
// all of them occupy the "lowest rank" they would have.

// After sending out the competition result

// the judge shall wait until the big_judge assigns another competition
// do what is described above again.

// when judge got ”-1 -1 -1 -1\n”from the big_judge
// the judge should exit.
