#ifndef GUI_H_
#define GUI_H_

int parse_move(char *move_string);
void parse_position(char* command);
void parse_go(char* command);
void uci_loop();

#endif // GUI_H_
