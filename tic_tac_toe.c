#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

typedef bool Player_t;

#define COMPUTER true
#define USER false

#define getPos(column, line) 3*(line)+(column)
#define COLOR_DEFAULT  "\033[0;39;40m"
#define COLOR_COMPUTER "\033[0;39;41m"
#define COLOR_USER     "\033[0;39;44m"
#define COLOR_CURSOR   "\033[0;39;47m"
#define getCursorColor(colorBuf) \
  snprintf((colorBuf), 20, COLOR_CURSOR)

void getColor(char *color, int value, int length) {
  switch (value) {
  case 0:
    snprintf(color, length, COLOR_DEFAULT);
    return;
  case 1:
    snprintf(color, length, COLOR_COMPUTER);
    return;
  case 2:
    snprintf(color, length, COLOR_USER);
    return;
  }
}

void drawBoard(int computerBoard, int userBoard, int cursor) {
  char color[20], reset[] = "\033[0m";
  int board[9], column, line, pos;
  char boardChar;
  printf("\033[2J");

  for (pos = 0; pos < 9; pos++) {
    if ((computerBoard >> pos) & 1)
      board[pos] = 1;
    else if ((userBoard >> pos) & 1) 
      board[pos] = 2;
    else 
      board[pos] = 0;
  }  

  for (line = 0; line < 3; line++) {
    for (column = 0; column < 3; column++) {
      pos = getPos(column, line);
      if (pos == cursor) 
	getCursorColor(color);
      else 
	getColor(color, board[pos], 20);

      if (column != 2)
	printf("%s%s", color, "      |");
      else
	printf("%s%s", color, "      ");
      printf("%s", reset);
    }
  
    printf("\n");

    for (column = 0; column < 3; column++) {
      pos = getPos(column, line);

      switch(board[pos]) {
      case 0:
	boardChar =  ' ';
	break;
      case 1:
	boardChar =  'X';
	break;
      case 2:
	boardChar = 'O';
	break;
      default:
	fprintf(stderr, "Error draw board\n");
	exit(EXIT_FAILURE);
      }

      if (pos == cursor) 
	getCursorColor(color);
      else 
	getColor(color, board[pos], 20);
	
      if (column != 2)
	printf("%s   %c  |", color, boardChar);
      else 
	printf("%s   %c  ", color, boardChar);

      printf("%s", reset);
    }
    printf("\n");

    for (column = 0; column < 3; column++) {
      pos = getPos(column, line);
      if (pos == cursor) 
	getCursorColor(color);
      else 
	getColor(color, board[pos], 20);

      if (line != 2) {
	if (column != 2) 
	  printf("%s%s", color, "______|");
	else 
	  printf("%s%s", color, "______");
      } else {
	if (column != 2) 
	  printf("%s%s", color, "      |");
	else 
	  printf("%s%s", color, "      ");
      }
      printf("%s", reset);
    }
    printf("\n");
  }

  printf("\n");
  printf("Use arrow keys and Enter to play, and type 'q' to quit\n\n");
}


bool isBoardFull(int computerBoard, int userBoard) {
  int total = 9;
  int pos;

  for (pos = 0; pos < 9; pos++) 
    if ( ((computerBoard >> pos) & 1) || ((userBoard >> pos) & 1))
      total--;
  
  return total == 0 ? true : false;
}

bool checkWin(int boardPos) {
  int i;
  bool board[9];
  for (i = 0; i < 9; i++) {
    board[i] = ((boardPos >> i) & 1);
  }
  return
    (board[0] && board[1] && board[2]) ||
    (board[0] && board[3] && board[6]) ||
    (board[0] && board[4] && board[8]) ||
    (board[2] && board[4] && board[6]) || 
    (board[2] && board[5] && board[8]) ||
    (board[1] && board[4] && board[7]) ||
    (board[3] && board[4] && board[5]) ||
    (board[6] && board[7] && board[8]) ;
}

int analyzeMove(Player_t player, int *returnScore,
		int computerBoard, int userBoard, int depth) {
  int pos;
  bool blank[9];
  bool boardFull = true;
  int score, bestMove;
  int maxScore = -10000, minScore = 10000;

  for (pos = 0; pos < 9; pos++) {
    if ((computerBoard>>pos) & 1 ||
	(userBoard>>pos) & 1) {
      blank[pos] = false;
    } else {
      blank[pos] = true;
      boardFull = false;
    }
  }

  if (boardFull) {
    if ((player == COMPUTER) && checkWin(computerBoard)) {
      *returnScore = 10-depth;
      return -1;
    }
    else if ((player == USER) && checkWin(userBoard)) {
      *returnScore = depth-10;
      return -1;
    }
    else {
      *returnScore = 0;
      return -1;;
    }
  }

  for (pos = 0; pos < 9; pos++) {
    if (blank[pos]) {
      if (player == COMPUTER) {
	int newBoardX = computerBoard;
	newBoardX |= (1 << pos);
	if (checkWin(newBoardX)) {
	  score = 10-depth;
	} else {
	  analyzeMove(USER, &score, newBoardX, userBoard, depth+1);
	}
	if (score > maxScore) {
	  maxScore = score;
	  bestMove = pos;
	}
      } else {
	int newBoardO = userBoard;
	newBoardO |= (1 << pos);
	if (checkWin(newBoardO)) {
	  score = depth-10;
	} else {
	  analyzeMove(COMPUTER, &score, computerBoard, newBoardO, depth+1);
	}
	if (score < minScore) {
	  minScore = score;
	  bestMove = pos;
	}
      }
    }
  }
  if (player == COMPUTER)
    *returnScore = maxScore;
  else
    *returnScore =  minScore;

  return bestMove;
}

bool isValidMove(int computerBoard, int userBoard, int move) {
  return (move >= 0 && move < 9) &&
    !((computerBoard >> move) & 1) &&
    !((userBoard >> move) & 1);
}

int getRandomMove() {
  static bool initialized = false;
  if (!initialized) {
    srand(time(NULL));
    initialized = true;
  }
  return rand()%9;
}

int getComputerMove(int computerBoard, int userBoard) {
  int score;
  return analyzeMove(COMPUTER, &score, computerBoard, userBoard, 0);
}

int getUserMove(int computerBoard, int userBoard) {
  static int cursorX, cursorY;
  char c;

  drawBoard(computerBoard, userBoard, -1);
  usleep(150000);
  drawBoard(computerBoard, userBoard, getPos(cursorX, cursorY));
  usleep(150000);
  
  do {
    switch (c) {
    case 65:
      if (cursorY != 0)
	cursorY--;
      break;
    case 66:
      if (cursorY != 2)
	cursorY++;
      break;
    case 67:
      if (cursorX != 2)
	cursorX++;
      break;
    case 68:
      if (cursorX != 0)
	cursorX--;
      break;
    case '\n':
      if (isValidMove(computerBoard, userBoard, getPos(cursorX, cursorY))) 
	return getPos(cursorX, cursorY);
      else
	break;
    default:
      break;
    }
    drawBoard(computerBoard, userBoard, getPos(cursorX, cursorY));
    usleep(10000);
  } while ((c = getchar()) != 'q');

  exit(EXIT_FAILURE);

}

void playGame(Player_t player) {
  bool firstRun = true;
  int computerBoard = 0, userBoard = 0, oldBoard = 0;
  int move;
  
  while (1) {
    if (player ==  COMPUTER) {
      if (firstRun) {
	move = getRandomMove();
	firstRun = false;
      } else {
	move = getComputerMove(computerBoard, userBoard);
      }
      if (!isValidMove(computerBoard, userBoard, move))
	continue;

      oldBoard = computerBoard;
      computerBoard |= (1 << move);

      drawBoard(computerBoard, userBoard, -1);
      usleep(150000);
      drawBoard(oldBoard, userBoard, -1);
      usleep(150000);
      drawBoard(computerBoard, userBoard, -1);

      if (checkWin(computerBoard)) {
	printf("GAME OVER: YOU LOST!\n");
	exit(EXIT_SUCCESS);
      } else if (isBoardFull(computerBoard, userBoard)) {
	printf("GAME OVER: IT'S A DRAW!\n");
	exit(EXIT_SUCCESS);
      }
    } else if (player ==  USER) {
      if (firstRun)
	firstRun = false;
      move = getUserMove(computerBoard, userBoard);
      if (!isValidMove(computerBoard, userBoard, move))
	continue;
      oldBoard = userBoard;
      userBoard |= (1 << move);
      
      drawBoard(computerBoard, userBoard, -1);
      usleep(150000);
      drawBoard(computerBoard, oldBoard, -1);
      usleep(150000);
      drawBoard(computerBoard, userBoard, -1);

      if (checkWin(userBoard)) {
	printf("GAME OVER: YOU WIN!\n");
	exit(EXIT_SUCCESS);
      } else if (isBoardFull(computerBoard, userBoard)) {
	printf("GAME OVER: IT'S A DRAW!\n");
	exit(EXIT_SUCCESS);
      }
    }
    player = !player;
  }
  
}

Player_t choosePlayer() {
  char reset[] = "\033[0m";
  int cursor = 0;
  char c;

  printf("\033[2J");
  do {
    switch (c) {
    case 67:
    case 68:
      cursor = (cursor+1) % 2;
      break;
    case '\n':
      if (cursor == 0)
	return USER;
      else if (cursor == 1)
	return COMPUTER;
    default:
      break;
    }
    printf("\033[2J");
    printf("Please choose the first player:\n");
    printf("[Using the arrow keys and Enter]\n\n");

    if (cursor == 0) 
      printf("\033[0;39;41m");
    else 
      printf("%s", reset);
    printf("I play first");
    printf("%s%s", reset, "    ");

    if (cursor == 1) 
      printf("\033[0;39;41m");
    else 
      printf("%s", reset);
    printf("You play first");
    printf("%s%s", reset, "    ");

    printf("\n");

  } while ((c = getchar()) != 'q');

  exit(EXIT_SUCCESS);
}

void setBufferedInput(bool enable) {
  static bool enabled = true;
  static struct termios old;
  struct termios new;

  if (enabled && !enable) {
    tcgetattr(STDIN_FILENO, &new);
    old = new;
    new.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
    enabled = false;
  } else if (!enabled && enable) {
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    enabled = true;
  }
}

int main () {
  setBufferedInput(false);
  playGame(choosePlayer());
  setBufferedInput(true);
}

