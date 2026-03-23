#ifndef UCI_H_
#define UCI_H_
#include "Perft.h"

int parse_move(const char *move_string);
void parse_position(char* command);
void parse_go(char* command);
void uci_loop();
int input_waiting();
void read_input();
void communicate();

#endif // UCI_H_
