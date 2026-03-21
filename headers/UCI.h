#ifndef UCI_H_
#define UCI_H_

int parse_move(const char *move_string);
void parse_position(char* command);
void parse_go(char* command);
void uci_loop();

#endif // UCI_H_
