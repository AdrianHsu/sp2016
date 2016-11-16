#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>    // time()
#include <sys/time.h>

int judge_id;
int random_key;

#define FOUR_PLAYER 4
#define MESSAGE_MAX 20
#define CHOICE_RAND_MAX 3
#define MYRAND_MAX 65536
#define MAX_ROUND 20


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
void playerfifo(char first_str[], int judge_id, int player_id) {

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
void makeChoice(char ch[]) { // 1, 3, 5

   int rand_key = ( rand() % CHOICE_RAND_MAX );
   // printf("%d\n", rand_key);

   ch[1] = '\0';
   if(rand_key == 0)
   	ch[0] = '1';
   else if (rand_key == 1)
   	ch[0] = '3';
   else if (rand_key == 2)
   	ch[0] = '5';
}
int main(int argc, char *argv[]) {
    
// player.c (./player [judge_id] [player_index] [random_key])
// player_index would be a character in {'A', 'B', 'C', 'D'}
// random_key would be an integer in range [0, 65536), used in this player in this competition
// should be randomly generated unique for four players 
// in the same competition.
// It is used to verify if a response really comes from that player.

   if(argc != 3) {
      fprintf(stderr, "USAGE: ./player [judge_id] [player_index] [random_key]\n");
      exit(EXIT_FAILURE);
   }
// the player_index is NOT the same as the player ID in judge/ big_judge. 
// It just means which index the player has in this competition.

   judge_id = atoi(argv[0]);
   char* _player_index = argv[1];
   random_key = atoi(argv[2]);
   char ch_p_index[2];
   memset(ch_p_index, 0, sizeof(ch_p_index));
   strcpy(ch_p_index, _player_index); // 'A', 'B', 'C', 'D'
   int player_index = ch_p_index[0] - 'A' + 1; // 1, 2, 3, 4
   // printf("%d+%s+%d\n", judge_id, _player_index, random_key);
   
   int pid = getpid(); // get it as per your OS
	struct timeval t;
	gettimeofday(&t, NULL);
	srand(t.tv_usec * t.tv_sec * pid);

// judge: 1) judge1.FIFO 2) judge1_A.FIFO
// player: 1) judge1.FIFO 2) judge1_A.FIFO 

   char my1stfifo[MESSAGE_MAX];
   memset(my1stfifo, 0, sizeof(my1stfifo));
   judgefifo(my1stfifo, judge_id);
   int my1stfifo_fd;
   my1stfifo_fd = open(my1stfifo, O_WRONLY);

   char choice[2];
   memset(choice, 0, sizeof(choice));
   makeChoice(choice);
// Round 1, player 1 sends judge 1 through judge1.FIFO: 
// A 9 3 
// In the first round, the player should first send its response to judge
// format: [player_index] [random_key] [number_choose] 
   char message[MESSAGE_MAX];
   memset(message, 0, sizeof(message));
   strcat(message, ch_p_index);
   strcat(message, " ");
   strcat(message, argv[2]);
   strcat(message, " ");
   strcat(message, choice);
   sleep(1);
   write(my1stfifo_fd, message, sizeof(message));
   // printf("write done, message = %s\n", message);

// The player should open a FIFO named judge[judge_id]_[player_index].FIFO, 
// which should be already created by the judge. 
// The player reads messages from judge[judge_id]_[player_index].FIFO, 
// such as judge1_A.FIFO,
// and writes responses to judge[judge_id].FIFO, such as judge1.FIFO.
   char myStrfifo[MESSAGE_MAX];
   memset(myStrfifo, 0, sizeof(myStrfifo));
   playerfifo(myStrfifo, judge_id, player_index);
   mkfifo(myStrfifo, 0666);
   int myfifo_fd = open(myStrfifo, O_RDONLY);
   char result[MESSAGE_MAX];
   memset(result, 0, sizeof(result));
   read(myfifo_fd, result, sizeof(result));
// Round 1, player 1 sends judge 1 through judge1.FIFO: 
// A 9 3 
// In the first round, the player should first send its response to judge
// format: [player_index] [random_key] [number_choose] 
// char message[MESSAGE_MAX];

   for(int t = 2; t <= MAX_ROUND; t++) {
      memset(my1stfifo, 0, sizeof(my1stfifo));
      memset(choice, 0, sizeof(choice));
      makeChoice(choice);

      memset(message, 0, sizeof(message));
      strcat(message, ch_p_index);
      strcat(message, " ");
      strcat(message, argv[2]);
      strcat(message, " ");
      strcat(message, choice);
      write(my1stfifo_fd, message, sizeof(message));

      memset(myStrfifo, 0, sizeof(myStrfifo));
      memset(result, 0, sizeof(result));
      read(myfifo_fd, result, sizeof(result));

   }





   close(my1stfifo_fd);
   unlink(my1stfifo);

}


// After the first round, the message from judge would be: 
// format: [p_A_number] [p_B_number] [p_C_number] [p_D_number] 
// indicating the responses of all players in the "previous" round.

// Each [pi_number] would be in {0,1,3,5}. 
// If the number is 0, it means that the player didn't make a response
// (because it has disconnected OR his time has run out).

// The above process will be repeated.

// The player must guarantee that it correctly gives out 20 responses, 

// or else, the judge will punish it
// (as a punishment, assuming the player not responds at all in the following rounds). 

// The player should exit after it gives out 20 responses. 

// The player would be executed again 
// when competing in another competition.
