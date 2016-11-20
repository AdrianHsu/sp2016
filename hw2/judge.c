#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> // for open()
#include <unistd.h> // for close()
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h> // time()
#include <signal.h>
#include <sys/time.h> // time()


#define FOUR_PLAYER 4
#define MESSAGE_MAX 20
#define MYRAND_MAX 65536
#define MAX_ROUND 3

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

   int is_repeat[6];
   for(int i = 0; i < 6; i ++)
      is_repeat[i] = 0;

   for(int i = 0; i < FOUR_PLAYER; i++) {
      int player_index = mes[i][0] - 'A' + 1; // 1, 2, 3, 4
      int p = parsePicked(mes[i]);
      picked[ player_index - 1 ] = p;
      if(is_repeat[ p ]) {
         is_repeat[ p ]++;
      } else 
         is_repeat[ p ] = 1;
   }
   for(int i = 0; i < FOUR_PLAYER; i++)
      if( is_repeat[ picked[ i ] ] <= 1)
         scores[i] += picked[ i ];
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
void getrank(char rank[], int scores[], int _ids[]) {
   
   int index[4];
   int tmp_rank[4];
   for(int i = 0; i < FOUR_PLAYER; i++) {
      index[i] = i;
      tmp_rank[i] = 4;
   }
   for(int i = 0 ; i < FOUR_PLAYER; i++) {
      for(int j = i + 1; j < FOUR_PLAYER ; j++ )
         if( scores[j] < scores[i] ) {
            int tmp = scores[i];
            scores[i] = scores[j];
            scores[j] = tmp;
            int ind = index[i];
            index[i] = index[j];
            index[j] = ind;       
         }
   }
   int k = 4;
   for(int i = 0; i < FOUR_PLAYER - 1; i++) {
      if(scores[i] < scores[i + 1])
         tmp_rank[i + 1] = --k;
      else
         tmp_rank[i + 1] = k;
   }
   for(int i = 0 ; i < FOUR_PLAYER; i++) {
      for(int j = i + 1; j < FOUR_PLAYER ; j++ )
         if( index[j] < index[i] ) {
            int tmp = tmp_rank[i];
            tmp_rank[i] = tmp_rank[j];
            tmp_rank[j] = tmp;
            int ind = index[i];
            index[i] = index[j];
            index[j] = ind;       
         }
   }
   for(int i = 0 ; i < FOUR_PLAYER; i++) {
      for(int j = i + 1; j < FOUR_PLAYER ; j++ )
         if( _ids[j] < _ids[i] ) {
            int tmp = tmp_rank[i];
            tmp_rank[i] = tmp_rank[j];
            tmp_rank[j] = tmp;
            int ind = _ids[i];
            _ids[i] = _ids[j];
            _ids[j] = ind;       
         }
   }
   for(int i = 0; i < FOUR_PLAYER; i++) {
      char ind[3];
      char ran[2];
      memset(ind, 0, sizeof(ind));
      memset(ran, 0, sizeof(ran));
      if(_ids[i] < 10) { 
         ind[0] = _ids[i] + '0';
         ind[1] = '\0';
      } else {
         ind[0] = (_ids[i] / 10) + '0';
         ind[1] = (_ids[i] % 10) + '0';
         ind[2] = '\0';         
      }
      ran[0] = tmp_rank[i] + '0';
      ran[1] = '\0';
      strcat(rank, ind);
      strcat(rank, " ");
      strcat(rank, ran);
      strcat(rank, "\n");
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
   if(argc != 2) {
      fprintf(stderr, "USAGE: ./judge [judge_id]\n");
      exit(EXIT_FAILURE);
   }
   judge_id = atoi(argv[0]);
   int w_pfd = atoi(argv[1]);
   printf("./judge %d\n", judge_id);

   char message[MESSAGE_MAX];
   read(STDIN_FILENO, message, sizeof(message));
   printf("mes: %s\n", message);

   int _ids[ FOUR_PLAYER ];
   for(int i = 0; i < FOUR_PLAYER; i ++)
      _ids[i] = 0;
   
   parse4players(message, _ids);

   int pid = getpid(); // get it as per your OS
   struct timeval t;
   gettimeofday(&t, NULL);
   srand(t.tv_usec * t.tv_sec * pid);

   // 1st round
   char my1stfifo[MESSAGE_MAX];
   memset(my1stfifo, 0, sizeof(my1stfifo));
   judgefifo(my1stfifo);
   mkfifo(my1stfifo, 0666);
   for(int i = 0; i < FOUR_PLAYER; i++) { // (i + 1) is player_id

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

   int picked[FOUR_PLAYER];
   int scores[FOUR_PLAYER];
   for(int i = 0; i < FOUR_PLAYER; i++) {
      picked[i] = 0;
      scores[i] = 0;
   }
   getScores(picked, scores, mes);

   char myStrfifo[FOUR_PLAYER][MESSAGE_MAX];
   int myfifo_fd[FOUR_PLAYER];
   for(int i = 0; i <  FOUR_PLAYER; i++) {
      myfifo_fd[i] = 0;
      memset(myStrfifo[i], 0, sizeof(myStrfifo[i]));
      playerfifo(myStrfifo[i], i + 1);
   }
   char result[MESSAGE_MAX];
   memset(result, 0, sizeof(result));
   build_result(picked, result);

   for(int i = 0; i < FOUR_PLAYER; i ++) {
      myfifo_fd[i] = open(myStrfifo[i], O_WRONLY);
      write(myfifo_fd[i], result, sizeof(result));
   }
   // printf("ROUND 1 ends\n");

   // 2 to 20 rounds
   for(int t = 2; t <= MAX_ROUND; t++) {

      for(int i = 0; i < FOUR_PLAYER; i++)
         memset(mes[i], 0, sizeof(mes[i]));
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
         for(int i = 0; i < FOUR_PLAYER; i++) {
            picked[i] = 0;
         }
         getScores(picked, scores, mes);

         for(int i = 0; i <  FOUR_PLAYER; i++) {
            memset(myStrfifo[i], 0, sizeof(myStrfifo[i]));
            playerfifo(myStrfifo[i], i + 1);
         }
         memset(result, 0, sizeof(result));
         build_result(picked, result);

         for(int i = 0; i < FOUR_PLAYER; i ++) {
            write(myfifo_fd[i], result, sizeof(result));
         }

         // printf("ROUND %d ends\n", t);
   }
   char rank[MESSAGE_MAX]; 
   memset(rank, 0, sizeof(rank));
   
   getrank(rank, scores, _ids);

   // write(STDOUT_FILENO, rank, sizeof(rank));
   write(w_pfd, rank, sizeof(rank));

   for(int i = 0; i < FOUR_PLAYER; i++) {
      close(myfifo_fd[i]);
      unlink(myStrfifo[i]);
   }
   close(my1stfifo_fd);
   unlink(my1stfifo);

   return 0;
}