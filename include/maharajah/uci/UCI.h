#ifndef UCI_H_
#define UCI_H_

void parse_go(char* command);
void uci_loop(void);
int input_waiting(void);
void read_input(void);
void communicate(void);

#endif // UCI_H_
