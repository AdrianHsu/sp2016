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
int combine = 0;
int initCombi = 0;
int currentCombi = 0;

void combinationUtil(int arr[], int data[], int start, int end,
      int index, int r, char ac[][MESSAGE_MAX]) {
   if (index == r)
   {
      char message[MESSAGE_MAX];
      memset(message, 0, sizeof(message));
      for (int j = 0; j < r; j++) {
         if(data[j] < 10) {
            char tmp[2];
            memset(tmp, 0, sizeof(tmp));
            tmp[0] = data[j] + '0';
            tmp[1] = '\0';
            strcat(message, tmp); // printf("%d ", data[j]);
         } else {
            char tmp[3];
            memset(tmp, 0, sizeof(tmp));
            tmp[0] = (data[j] / 10) + '0';
            tmp[1] = (data[j] % 10) + '0';
            tmp[2] = '\0';
            strcat(message, tmp); // printf("%d ", data[j]);
         }
         if(j != r - 1)
            strcat(message, " "); 
      }
      strcpy( ac[initCombi] , message );
      initCombi++;
      return;
   }
   for (int i = start; i <= end && end - i + 1 >= r - index; i++)
   {
      data[index] = arr[i];
      combinationUtil(arr, data, i + 1, end, index + 1, r, ac);
   }
}
void setCombination(int arr[], int n, int r, char ac[][MESSAGE_MAX]) {
   int data[r];
   combinationUtil(arr, data, 0, n - 1, 0, r, ac);
}
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
void fourPlayersToIntArray(char message[], int _ids[]) {
   int i = 0;
   char *s = strtok(message, " ");
   while(s != NULL) {
      _ids[i] = atoi(s);
      i++;
      s = strtok(NULL, " ");
   }
}
int parse4players(char message[], int _p[], char ac[][MESSAGE_MAX]) {
   // int isDone[combine];
   // for(int i = 0; i < combine; i++)
   //    isDone[ i ] = 0;

   while(1) {
      int r = rand() % combine; // r == 0 means [1, 2, 3, 4]
      if(ac[ r ] == 0) continue;

      char tmp [MESSAGE_MAX];
      memset(tmp, 0, sizeof(tmp));
      strcpy(tmp, ac[ r ]);
      int arr[4] = {0};
      fourPlayersToIntArray(tmp, arr);

      if(_p[ arr[0] - 1 ] == 1 && _p[ arr[1] - 1 ] == 1 && _p[ arr[2] - 1 ] == 1 && _p[ arr[3] - 1 ] == 1) {

         strcpy(message, ac[ r ]);
         for(int i = 0; i < FOUR_PLAYER; i++)
            _p[ arr[i] - 1 ] = 0;
         memset(ac[ r ], 0, sizeof(ac[ r ]));
         return r;
      } else {
         // isDone[ r ] = 1;
         continue;
      }
   }
}
void accuScore(char res[], int t[]) {
   char *s = strtok(res, " ");
   int i = 0;
   int j = 0;
   int first = 1;
   while(s != NULL) {
      i = atoi(s);
      s = strtok(NULL, "\n");
      if(s == NULL)
         break;
      j = atoi(s);
      t[i - 1] += (4 - j);
      s = strtok(NULL, " ");
   }
}
void forkJudge(pid_t cpid, int i, int pipefd[], int _p[], char ac[][MESSAGE_MAX], int t[]) {
   if(i == 0) return;
   int status;
   if (cpid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);

   } else if (cpid == 0) { // child process
   if( dup2( pipefd[0], STDIN_FILENO ) < 0 ){
      perror( "dup2()" );
      exit(EXIT_FAILURE);
   }
      char myjudge_id[3];
      memset(myjudge_id, 0, sizeof(myjudge_id));
      myitoa(i, myjudge_id);
      char w_pfd[3];
      memset(w_pfd, 0, sizeof(w_pfd));
      myitoa(pipefd[1], w_pfd);

      execl("./judge", myjudge_id, w_pfd, NULL);

      _exit(EXIT_SUCCESS);
   } else { // parent process
      if( dup2( pipefd[0], STDIN_FILENO ) < 0 ){
         perror( "dup2()" );
         exit(EXIT_FAILURE);
      }
      char message[MESSAGE_MAX];
      int r = 0;
      char res[MESSAGE_MAX];
      while(1) {
         memset(message, 0, sizeof message);
         if(currentCombi == initCombi) {
            strcat(message, "-1 -1 -1 -1\n");
            write(pipefd[1], message, sizeof(message));
            break;
         }
         r = parse4players(message, _p, ac);
         write(pipefd[1], message, sizeof(message));
         // forkJudge(--i, pipefd, _p, ac);
         // wait(&status);
         sleep(1);
         memset(res, 0, sizeof(res));
         read(pipefd[0], res, sizeof(res)); //pipefd[0] is STDIN
         printf("res:\n%s\n", res);
         accuScore(res, t);

         memset(ac[ r ], 0, sizeof(ac[ r ]));
         currentCombi++;

         int arr[4] = {0};
         fourPlayersToIntArray(message, arr);
         memset(message, 0, sizeof(message));

         for(int j = 0; j < FOUR_PLAYER; j++)
            _p[ arr[j] - 1 ] = 1;
         printf("curr: %d / init: %d\n", currentCombi, initCombi);
      }
   }
}
int main(int argc, char *argv[]) {

   if(argc != 3) {
      fprintf(stderr, "USAGE: ./big_judge [judge_num] [player_num]\n");
      exit(EXIT_FAILURE);
   }
   srand(time(NULL));

   judge_num = atoi(argv[1]);
   player_num = atoi(argv[2]);

   if(judge_num < 1 || judge_num > 12 || player_num < 4 || player_num > 20) {
      fprintf(stderr, "ERROR: invalid num\n");
      exit(EXIT_FAILURE);		
   }
   // if(player_num / judge_num != FOUR_PLAYER) {
   //    fprintf(stderr, "ERROR: simplified version invalid\n");
   //    exit(EXIT_FAILURE);		
   // }

   int arr[player_num];
   int totalScore[player_num];

   for(int i = 0; i < player_num; i++) {
      arr[i] = i + 1;
      totalScore[i] = 0;
   }
   int r = FOUR_PLAYER;
   int n = sizeof(arr)/sizeof(arr[0]);
   combine = ( (player_num) * (player_num - 1) * (player_num - 2) * (player_num - 3 ) ) / 24;
   char all_combine[combine][MESSAGE_MAX];

   for(int i = 0; i < combine; i++)
      memset(all_combine[i], 0, sizeof(all_combine[i]));
   setCombination(arr, n, r, all_combine);
   // for(int i = 0; i < initCombi; i++)
   //    printf("%s\n", all_combine[i]);

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

   for(int i = 1; i <= tmp_num; i++) {
      pid_t cpid = fork();
      forkJudge(cpid, i, pipefd, _p, all_combine, totalScore);
   }

   // end
   printf("===result===\n");
   for(int i = 0; i < player_num; i++) {
      printf("%d %d\n", i+1, totalScore[i]);
   }


   return 0;
}
