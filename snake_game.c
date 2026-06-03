#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#define WORLD_X_SIZE 50
#define WORLD_Y_SIZE WORLD_X_SIZE
typedef enum { RIGHT, LEFT, UP, DOWN } moving_direction;
typedef struct {
  char x, y;
} position;
typedef struct {
  position *body_position;
  char length;
} Snake;
typedef struct {
  position pos;
  char caracter;
  char exists;
} food;
Snake new_snake() {
  Snake snake;
  snake.length = 1;
  snake.body_position = malloc(snake.length * sizeof(position));
  if (!snake.body_position) {
    printf("Could no create snake: Null ptr");
    exit(EXIT_FAILURE);
  }
  snake.body_position[0].x = WORLD_X_SIZE / 2;
  snake.body_position[0].y = WORLD_Y_SIZE / 2;
  return snake;
}
void increase_snake(Snake *snake) {
  snake->length += 1;
  position *tmp =
      realloc(snake->body_position, snake->length * sizeof(position));
  if (tmp) {
    snake->body_position = tmp;
  }
}

static inline _Bool find_food(Snake *snake, position Food_pos, moving_direction direction) {
  switch (direction) {

  case RIGHT:
    if (snake->body_position[0].x + 1 == Food_pos.x &&
        snake->body_position[0].y == Food_pos.y) {
      return 1;
    }
  case LEFT:
    if (snake->body_position[0].x - 1 == Food_pos.x &&
        snake->body_position[0].y == Food_pos.y) {
      return 1;
    }
  case UP:
    if (snake->body_position[0].x == Food_pos.x &&
        snake->body_position[0].y - 1 == Food_pos.y) {
      return 1;
    }
  case DOWN:
    if (snake->body_position[0].x == Food_pos.x &&
        snake->body_position[0].y + 1 == Food_pos.y) {
      return 1;
    }
  }
  return 0;
}
static inline _Bool colision(Snake *snake, position Food_pos, moving_direction direction) {
  switch (direction) {

  case RIGHT:
    if (snake->body_position[0].x + 1 == WORLD_X_SIZE) {
      return 1;
    }
  case LEFT:
    if (snake->body_position[0].x == 0) {
      return 1;
    }
  case UP:
    if (snake->body_position[0].y == 0) {
      return 1;
    }
  case DOWN:
    if (snake->body_position[0].y + 1 == WORLD_Y_SIZE) {
      return 1;
    }
  }
  return 0;
}
void move_Snake(Snake *snake, food *Food,
                char world[WORLD_X_SIZE][WORLD_Y_SIZE],
                moving_direction direction) {
  char X_last_pos = snake->body_position[0].x;
  char Y_last_pos = snake->body_position[0].y;
  world[snake->body_position[0].x][snake->body_position[0].y] = ' ';
  if (colision(snake, Food->pos, direction)) {
    goto game_over;
  } else if (find_food(snake, Food->pos, direction)) {
    increase_snake(snake);
    Food->exists = 0;
  }
  switch (direction) {
  case RIGHT:
    snake->body_position[0].x++;
    break;
  case LEFT:
    snake->body_position[0].x--;
    break;
  case UP:
    snake->body_position[0].y--;
    break;
  case DOWN:
    snake->body_position[0].y++;
    break;
  }
  world[snake->body_position[0].x][snake->body_position[0].y] = '0';
  if (snake->length > 1) {
    for (int i = 1; i < snake->length; i++) {
      char old_x = snake->body_position[i].x;
      char old_y = snake->body_position[i].y;
      world[snake->body_position[i].x][snake->body_position[i].y] = ' ';
      snake->body_position[i].x = X_last_pos;
      snake->body_position[i].y = Y_last_pos;
      world[snake->body_position[i].x][snake->body_position[i].y] = '0';
      X_last_pos = old_x;
      Y_last_pos = old_y;
    }
  }
  return;
game_over:
  printf("\r\n\x1b[93mGame over\x1b[0m");
  exit(0);
}
void print_matrix(char matrix[WORLD_X_SIZE][WORLD_Y_SIZE]) {
  printf("\r\n");
  for (int i = 0; i < WORLD_Y_SIZE; i++) {
    for (int j = 0; j < WORLD_X_SIZE; j++) {
      printf("%c", matrix[j][i]);
    }
    printf("\r\n");
  }
  fflush(stdout);
}
void fill_matrix(char matrix[WORLD_X_SIZE][WORLD_Y_SIZE], char c) {
  for (int i = 0; i < WORLD_X_SIZE; i++) {
    for (int j = 0; j < WORLD_Y_SIZE; j++) {
      matrix[i][j] = c;
    }
  }
}
struct termios old_tio, new_tio;
void restore_termius() { tcsetattr(STDIN_FILENO, TCSANOW, &old_tio); }
int main() {

  tcgetattr(STDIN_FILENO, &old_tio);
  new_tio = old_tio;
  cfmakeraw(&new_tio);
  new_tio.c_cc[VMIN] = 0;  //
  new_tio.c_cc[VTIME] = 1; // timeout de 0.1s
  tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
  atexit(restore_termius);
  srand(time(NULL));
  char world[WORLD_X_SIZE][WORLD_Y_SIZE];
  fill_matrix(world, ' ');
  for (int i = 0; i < WORLD_Y_SIZE; i++) {
    world[i][0] = '-';
    world[i][WORLD_Y_SIZE - 1] = '-';
  }
  for (int i = 0; i < WORLD_X_SIZE; i++) {
    world[0][i] = '|';
    world[WORLD_X_SIZE - 1][i] = '|';
  }
  food Food;
  Food.caracter = '*';
  Food.exists = 0;
  Snake snake = new_snake();
  moving_direction direction = RIGHT;
  char input[1];
  while (1) {
    print_matrix(world);
    printf("Food = %d,%d\r\n", Food.pos.x, Food.pos.y);
    printf("Snake_head = %d,%d\r\n", snake.body_position[0].x,
           snake.body_position[0].y);
    printf("Snake_lengh = %d\r\n", snake.length);
    printf("%c\n", input[0]);
    read(STDIN_FILENO, &input, 1);
    if (input[0] == 'd') {
      direction = RIGHT;
    } else if (input[0] == 'w') {
      direction = UP;
    } else if (input[0] == 'a') {
      direction = LEFT;
    } else if (input[0] == 's') {
      direction = DOWN;
    } else if (input[0] == 'q') {
      break;
    }
    move_Snake(&snake, &Food, world, direction);
    if (Food.exists == 0) {
      while (1) {
        Food.pos.x = rand() % WORLD_X_SIZE;
        Food.pos.y = rand() % WORLD_Y_SIZE;
        if (Food.pos.x > 0 && Food.pos.y > 0 && Food.pos.x < WORLD_X_SIZE - 1 &&
            Food.pos.y < WORLD_Y_SIZE - 1) {
          char valid_position = 0;
          for (int i = 0; i < snake.length; i++) {
            if (Food.pos.x == snake.body_position[i].x ||
                Food.pos.y == snake.body_position[i].y) {
              valid_position = 0;
              break;
            } else {
              valid_position = 1;
            }
          }
          if (valid_position == 0) {
            continue;
          } else {
            break;
          }
        }
      }
      Food.exists = 1;
      world[Food.pos.x][Food.pos.y] = Food.caracter;
    }
  }

  return 0;
}
