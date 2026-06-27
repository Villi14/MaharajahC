#ifndef CUSTOM_SETUP_H_
#define CUSTOM_SETUP_H_

// Generate a reasonable custom Maharajah position directly into the global
// board state. side_to_move: 0 = white, 1 = black, any other value = random.
// Returns 1 on success.
int generate_reasonable_custom_position(int side_to_move, unsigned int seed);

// Serialize the current global board state to FEN.
// Returns 1 on success.
int custom_position_board_to_fen(char* out, int out_len);

#endif // CUSTOM_SETUP_H_
