// NOTE1: Remember that every time you writes a message to a pipe or a FIFO, 
// you should use fflush() to ensure the message being correctly passed out.

// big_judge.c (./big_judge [judge_num] [player_num]) 
// (1<=judge_num<=12)
// (4<=player_num<=20)

// $ ./big_judge 1 4 
// This will run 1 judge and 4 players. The big judge will fork and execute:
// $ ./judge 1 
// The big_judge sends judge 1 
// (judge 1 reads from standard input): 1 2 3 4 

// At first, the big_judge should fork and execute the number of judges

// with IDs from 1 to judge_num.

// big_judge must build pipes to communicate with each of them

// and then executing them:

// The message coming from the judge 

// the competition result presided by that judge (from judge.c)

// END execute, after big_judge executes judges

// distribute every 4 players to an available judge via pipe.
// There will be C(player_num,4) competitions

// players are numbered from 1 to player_num

// the message sending to the judges are of the format
// [p1_id] [p2_id] [p3_id] [p4_id]

// If there is no available judge, 

// waits until one of the judges returns the competition result

// assign another competition to that judge

// make full use of available judges but not let any available judge idle.

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