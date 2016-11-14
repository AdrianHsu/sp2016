// judge.c (./judge [judge_id])

// big_judge called -> $ ./judge 1 
// The judge will create: 
// judge1.FIFO 
// judge1_A.FIFO 
// judge1_B.FIFO 
// judge1_C.FIFO 
// judge1_D.FIFO 

// The big_judge sends judge 1 
// (judge 1 reads from standard input): 1 2 3 4 


// create a FIFO named judge[judge_id].FIFO, such as judge1.FIFO
// to read responses from the players

// create four FIFOs named judge[judge_id]_A.FIFO...etc

// to write messages to the players in the competition

// The judge should read from standard input
// waiting for the big_judge to assign four players in.

// After knowing the players, 
// the judge forks four child processes

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