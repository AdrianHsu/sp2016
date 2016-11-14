#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

// NOTE1: Remember that every time you writes a message to a pipe or a FIFO, 
// you should use fflush() to ensure the message being correctly passed out.

void forkJudge(int i, int pipefd[]) {
	if(i == 0) return;
	pid_t cpid = fork();
    int X = open("./output_file.txt", O_WRONLY);

    if (cpid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);

    } else if (cpid == 0) { //child process

// The big judge will fork and execute:
// $ ./judge 1 
// The big_judge sends judge 1 
// (judge 1 reads from standard input): 1 2 3 4 
	char *argv[3] = {"-al", NULL};
	dup2(X, 1); /* fd 1 is standard output,
                   so this makes standard out refer to the same file as X  */
   
    execvp("ls", argv);
    close(X);
// The message coming from the judge would be
    return forkJudge(--i, pipefd);
// the competition result presided by that judge (from judge.c)

// after big_judge executes judges

// distribute every 4 players to an available judge via pipe.
// There will be C(player_num,4) competitions

// players are numbered from 1 to player_num

// the message sending to the judges are of the format
// [p1_id] [p2_id] [p3_id] [p4_id]

// If there is no available judge, 

// waits until one of the judges returns the competition result
// e.g. Judge 1 writes the result to standard output (sending to big_judge): 
// format: [p1_id] [p1_rank]
// 1 4 
// 2 2 
// 3 1 
// 4 3 


// assign another competition to that judge

// make full use of available judges but not let any available judge idle.

    } else { //parent process
    	close(X);
    	return;
    }
}

int main(int argc, char *argv[]) {

// big_judge.c (./big_judge [judge_num] [player_num]) 
// (1<=judge_num<=12)
// (4<=player_num<=20)
// $ ./big_judge 1 4 
// This will run 1 judge and 4 players. 
	
	if(argc != 3) {
		fprintf(stderr, "USAGE: ./big_judge [judge_num] [player_num]\n");
      	exit(EXIT_FAILURE);
	}

	int judge_num = atoi(argv[1]);
	int player_num = atoi(argv[2]);
	if(judge_num < 1 || judge_num > 12 || player_num < 4 || player_num > 20) {
		fprintf(stderr, "ERROR: invalid num\n");
      	exit(EXIT_FAILURE);		
	}

// At first, the big_judge should fork and execute the number of judges
// big_judge must build pipes to communicate with each of them
// with IDs from 1 to judge_num.
   int pipefd[2];
   if (pipe(pipefd) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
   }
   forkJudge(judge_num, pipefd);


	return 0;
}

// The big_judge has to keep accumulative scores of all players.

// scores of all players are initally set to 0. 

// When the judge returns the result back to the big_judge:

// the result is the rankings of the four players,

// big_judge should add scores to the players’ 
// accumulative scores according to the rankings of them.
// the first, second, third, and fourth place gets 3, 2, 1, and 0 

// The big_judge reads the result, and does the calculation


// After all competitions are done:
// big_judge should send the string “-1 -1 -1 -1\n” to all judges

// indicating that all competitions are done and judges can exit.

// he big_judge outputs all players’ ID sorted by their scores

// from the highest to the lowest, separated by spaces

//  If two players have the same score, output the one with smaller ID first.

// The big_judge outputs the result