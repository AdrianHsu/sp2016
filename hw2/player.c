// player.c (./player [judge_id] [player_index] [random_key])
// player_index would be a character in {'A', 'B', 'C', 'D'}
// random_key would be an integer in range [0, 65536), used in this player in this competition
int main(int argc, char *argv[]) {
}
// should be randomly generated unique for four players 
// in the same competition.

// It is used to verify if a response really comes from that player.

// the player_index is NOT the same as the player ID in judge/ big_judge. 
// It just means which index the player has in this competition.

// The player should open a FIFO named judge[judge_id]_[player_index].FIFO, 
// which should be already created by the judge. 

// Round 1, player 1 sends judge 1 through judge1.FIFO: 
// A 9 3 

// The player reads messages from judge[judge_id]_[player_index].FIFO, 
// such as judge1_A.FIFO, 

// and writes responses to judge[judge_id].FIFO, such as judge1.FIFO.

// In the first round, the player should first send its response to judge
// format: [player_index] [random_key] [number_choose] 

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