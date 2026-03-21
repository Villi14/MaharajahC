#ifndef SEARCH_H_
#define SEARCH_H_

void search_position(int depth);
int negamax(int alpha, int beta, int depth);
int quiescence(int alpha, int beta);

#endif // !SEARCH_H_
