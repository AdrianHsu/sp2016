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
#define MAX_ROUND 20

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
void forkPlayer(int player_index, int rand_key) {
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

// The judge executes: 
// $ ./player 1 A 9 
// $ ./player 1 B 2013 
// $ ./player 1 C 10000 
// $ ./player 1 D 65535 

// run exec() to execute the player programs
// start 20 game rounds.

// The executable file of a player named "player", 
// are placed in the same directory containing "big_judge" and "judge"

      execl("./player", myjudge_id, pIdx, random_key, NULL);

   } else { //parent
      // wait(NULL);
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
int parsePicked(char message[]) {
   
   int i = 0;
   char *s = strtok(message, " ");
   s = strtok(NULL, " ");
   s = strtok(NULL, " ");
   return atoi(s);
}

void getScores(int picked[], int scores[], char mes[][MESSAGE_MAX]) {

   int is_repeat[6]; // 1, 3, 5, bool array
   for(int i = 0; i < 6; i ++)
      is_repeat[i] = 0;

   for(int i = 0; i < FOUR_PLAYER; i++) {
      scores[i] = 0;
      int player_index = mes[i][0] - 'A' + 1; // 1, 2, 3, 4
      // printf("mesg: %s\n", mes[i]);
      int p = parsePicked(mes[i]);
      picked[ player_index - 1 ] = p;
      // printf("picked[%d] = %d\n", player_index - 1, p);
      if(is_repeat[ p ]) {
         is_repeat[ p ]++;
      } else 
         is_repeat[ p ] = 1;
   }
   for(int i = 0; i < FOUR_PLAYER; i++) {
      if( is_repeat[ picked[ i ] ] > 1) {
         // do nothing
      } else 
         scores[i] += picked[ i ];
      // printf("%d, score = %d\n", i, scores[i]);
   }
}
void build_result(int picked[], char result []) {

   for(int i = 0; i < FOUR_PLAYER; i++) {
      
      char tmp[2];
      memset(tmp, 0, sizeof(tmp));
      tmp[0] = picked[i] + '0';
      tmp[1] = '\0';
      strcat(result, tmp);
      if(i != FOUR_PLAYER - 1)
         strcat(result, " ");
   }
}

void judgefifo(char first_str[]) {

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
void playerfifo(char first_str[], int player_id) {

   char str1[2];
   char str2[3];
   strcat(first_str, "./tmp/judge"); 

   if(judge_id < 10) {
      memset(str1, 0, sizeof(str1));      
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
   t[0] = 'A' + player_id - 1;
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
   // judge1_A.FIFO...etc

// The judge should read from standard input
// waiting for the big_judge to assign four players in.
   char message[MESSAGE_MAX];
   read(STDIN_FILENO, message, sizeof(message));

   int _ids[ FOUR_PLAYER ];
   for(int i = 0; i < FOUR_PLAYER; i ++)
      _ids[i] = 0;
   // The big_judge sends judge 1 
   // (judge 1 reads from standard input): 1 2 3 4  
   parse4players(message, _ids);
   // for(int i = 0; i < FOUR_PLAYER; i ++)
   //    printf("%d\n", _ids[i]);

   srand(time(NULL));
   // create a FIFO named judge[judge_id].FIFO, such as judge1.FIFO
   // to "read" responses from the players
   char my1stfifo[MESSAGE_MAX];
   memset(my1stfifo, 0, sizeof(my1stfifo));
   judgefifo(my1stfifo);
   // printf("first_str: %s\n", my1stfifo);
   mkfifo(my1stfifo, 0666);
   for(int i = 0; i < FOUR_PLAYER; i++) { // (i + 1) is player_id
// After knowing the players, 
// the judge forks four child processes
      int rand_key = ( rand() % MYRAND_MAX );
      forkPlayer(i + 1, rand_key);
   }
   char mes[FOUR_PLAYER][MESSAGE_MAX];
   for(int i = 0; i < FOUR_PLAYER; i++)
      memset(mes[i], 0, sizeof(mes[i]));

   int my1stfifo_fd = open(my1stfifo, O_RDONLY);   
   char buf[MESSAGE_MAX];
   memset(buf, 0, sizeof(buf));
   
   for(int k = 0; k < FOUR_PLAYER; k++) {
      read(my1stfifo_fd, buf, sizeof(buf));
      if(buf[0] == 'A') {
         strcpy(mes[0], buf);
         buf[0] = 0;
      }
      else if(buf[0] == 'B'){
         strcpy(mes[1], buf);
         buf[0] = 0;
      }
      else if(buf[0] == 'C'){
         strcpy(mes[2], buf);
         buf[0] = 0;
      }
      else if(buf[0] == 'D'){
         strcpy(mes[3], buf);
         buf[0] = 0;
      }
      else {
         k--;
         continue;
      }
   }

   int picked[4];
   int scores[4];
   getScores(picked, scores, mes);

   char myStrfifo[FOUR_PLAYER][MESSAGE_MAX];
   int myfifo_fd[4];
   for(int i = 0; i <  FOUR_PLAYER; i++) {
      myfifo_fd[i] = 0;
      memset(myStrfifo[i], 0, sizeof(myStrfifo[i]));
      playerfifo(myStrfifo[i], i + 1);
   // create four FIFOs named judge[judge_id]_A.FIFO...etc
   // to "write" messages to the players in the competition
      // printf("str: %s\n", myStrfifo[i]);
   }
   char result[MESSAGE_MAX];
   memset(result, 0, sizeof(result));
   build_result(picked, result);

   for(int i = 0; i < FOUR_PLAYER; i ++) {
      // mkfifo(myStrfifo[i], 0666);
      myfifo_fd[i] = open(myStrfifo[i], O_WRONLY);

      // sleep(1);
      write(myfifo_fd[i], result, sizeof(result));
   }

   for(int i = 1; i < MAX_ROUND; i++) {
// In each round, 
// except for the first round

// the judge tells every player the result of the previous round
// via specific FIFO
// help them decide what number to tell in the next round. (AH: IGNORED)
// Round 2, judge 1 sends player 1 through judge1_A.FIFO:
// 3 5 5 1 
      

   }

   for(int i = 0; i < FOUR_PLAYER; i++) {
      close(myfifo_fd[i]);
      unlink(myStrfifo[i]);
   }
   close(my1stfifo_fd);
   unlink(my1stfifo);

   return 0;
}

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
