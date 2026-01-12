#include <emscripten/emscripten.h>

// cs_snake.c
// Written by Aakarsh Singh z5709521 on <INSERT DATE>
//
// Description: <INSERT DESCRIPTION OF PROGRAM>

// Provided Libraries
#include <stdio.h>

// Add your own #include statements below this line
#include <stdbool.h>

// Provided constants
#define ROWS 10
#define COLS 10

#define NO_SNAKE -1

// Add your own #define constants below this line
#define EXIT_MAP_SETUP 's'
#define ADD_WALL 'w'
#define ADD_EXIT_LOCKED 'e'
#define ADD_APPLE 'a'
#define ADD_NORMAL_APPLE 'n'
#define ADD_LONG_WALL 'W'
#define ADD_REVERSE_APPLE 'r'
#define ADD_DIRECT_PASSAGE 'p'
#define ADD_PASS_UP '^'
#define ADD_PASS_DOWN 'v'
#define ADD_PASS_LEFT '<'
#define ADD_PASS_RIGHT '>'
#define ADD_SPLIT_APPLE 's'
#define ADD_PORTAL 't'
#define ADD_EXPLODING_APPLE 'e'

#define FIRST_ROW 0
#define FIRST_COL 0

#define VERTICAL 'v'
#define HORIZONTAL 'h'

#define KEY_UP 'w'
#define KEY_LEFT 'a'
#define KEY_DOWN 's'
#define KEY_RIGHT 'd'
#define PRINT_STATS 'p'
#define RESET_MAP 'r'

#define NONE_REMAINING 0

#define HUNDRED 100.0

#define NORMAL_APPLE_PTS 5
#define REVERSE_APPLE_PTS 10
#define SPLIT_APPLE_PTS 20
#define EXPLODING_APPLE_PTS 20

#define MAX_BOARD_CELLS 100

#define FIRST_SNAKE_CELL 0

#define MAX_PORTAL_PAIRS 5

#define NUM_PAIRS 4
#define NUM_COORDS 2
#define UP_ROW -1
#define UP_COL 0
#define DOWN_ROW 1
#define DOWN_COL 0
#define LEFT_ROW 0
#define LEFT_COL -1
#define RIGHT_ROW 0
#define RIGHT_COL 1
#define ELEM_1_OF_PAIR 0
#define ELEM_2_OF_PAIR 1

#define INIT_RING_OFFSET 2
#define FINAL_RING_OFFSET 1

#define LAST_TILE_OFFSET 1

#define EXCLUDE_HEAD_OFFSET 1
#define SPLIT_DIVISOR 2
#define ROUND_UP_OFFSET 1

// Provided enums
// Enum for features on the game board
enum entity
{
    BODY_SEGMENT,
    EXIT_LOCKED,
    EXIT_UNLOCKED,
    WALL,
    APPLE_NORMAL,
    APPLE_REVERSE,
    APPLE_SPLIT,
    APPLE_EXPLODE,
    EXPLOSION,
    PASSAGE_UP,
    PASSAGE_DOWN,
    PASSAGE_LEFT,
    PASSAGE_RIGHT,
    PORTAL,
    EMPTY
};

// Add your own enums below this line
enum snake_move
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// Provided structs
// Represents a tile/cell on the game board
struct tile
{
    enum entity entity;
};

// Add your own structs below this line
struct apple_data
{
    int num_apples_eaten;
    int num_apples_remaining;
    double apple_percentage;
    int num_initial_apples;
    int num_exploded_apples;
};

struct snake_cells
{
    int row;
    int col;
};

struct snake_data
{
    int head_row;
    int head_col;
    int tail_row;
    int tail_col;
    enum snake_move move;
    struct snake_cells cells[MAX_BOARD_CELLS];
    int cell_count;
};

struct portal_pair
{
    int portal_1_row;
    int portal_1_col;
    int portal_2_row;
    int portal_2_col;
    bool is_pair_disabled;
};

struct exploding_apple
{
    int row;
    int col;
    int radius;
};

struct explosion_data
{
    bool is_explosion_active;
    int explosion_radius;
    int explosion_centre_row;
    int explosion_centre_col;
    int explosion_turn;
};

struct game_data
{
    int num_points;
    int num_moves;
    int num_points_remaining;
    struct snake_data snake_info;
    struct apple_data apple_info;
    struct portal_pair portals[MAX_PORTAL_PAIRS];
    int portal_count;
    struct exploding_apple expl_apples[MAX_BOARD_CELLS];
    int exploding_apple_count;
    struct explosion_data explosion_info;
};

// Global game state
static struct tile board[ROWS][COLS];
static struct tile init_board[ROWS][COLS];
static struct game_data game_info = {0};
static struct game_data init_game_info = {0};
static bool is_game_over = false;

// Provided Function Prototypes
void initialise_board
(
    struct tile board[ROWS][COLS],
    struct tile init_board[ROWS][COLS]
);
void print_board
(
    struct tile board[ROWS][COLS], 
    int snake_row, 
    int snake_col,
    struct game_data *game_info
);
void print_game_statistics(struct game_data *game_info);
void print_game_statistics_with_rival
(
    int original_points,
    int original_moves_made,
    int original_num_apples_eaten,
    int rival_points,
    int rival_moves_made,
    int rival_num_apples_eaten,
    int num_apples_remaining,
    double completion_percentage,
    int maximum_points_remaining
);
void print_board_line(void);
void print_tile_spacer(void);
void print_board_header(void);

// Add your function prototypes below this line
static void setup_default_map(void);
void add_entity_to_map
(
    struct tile board[ROWS][COLS], 
    enum entity entity,
    struct game_data *game_info
);
bool is_valid_position(int row, int col);
void add_long_wall(struct tile board[ROWS][COLS]);
bool is_valid_long_wall(int row, int col, int length, char direction);
static void spawn_default_snake(void);
bool is_position_occupied(int row, int col, struct tile board[ROWS][COLS]);
static void handle_key_step(char selection);
void unlock_exits(struct tile board[ROWS][COLS]);
bool execute_move
(
    char selection, 
    struct game_data *game_info,
    bool *has_moved,
    struct game_data *init_game_info,
    struct tile board[ROWS][COLS],
    struct tile init_board[ROWS][COLS],
    bool *has_reset
);
double calc_apple_percent(struct apple_data *apple_info);
void update_stats_and_board
(
    struct game_data *game_info, 
    struct tile board[ROWS][COLS],
    bool *ignore_collision,
    bool *hit_disabled_portal
);
void check_game_over
(
    struct tile board[ROWS][COLS], 
    struct game_data *game_info,
    bool *is_game_over,
    bool ignore_collision,
    bool is_position_valid,
    bool hit_disabled_portal
);
bool is_pos_non_consumable
(
    int snake_row, 
    int snake_col, 
    struct tile board[ROWS][COLS]
);
void add_apple_or_direct_passage
(
    struct tile board[ROWS][COLS], 
    struct game_data *game_info
);
void update_apples(struct game_data *game_info, int num_pts);
void handle_passage
(
    struct tile board[ROWS][COLS], 
    struct snake_data *snake_info, 
    bool *ignore_collision,
    int row,
    int col
);
void initialise_apples(struct game_data *game_info, int num_pts);
void initialise_snake_rows_and_cols
(
    struct snake_data *snake_info,
    int snake_row,
    int snake_col
);
void update_snake_cells(struct snake_data *snake_info);
void reverse_snake
(
    struct snake_data *snake_info, 
    struct tile board[ROWS][COLS]
);
void reverse_snake_cells(struct snake_cells cells[], int cell_count);
void split_snake
(
    struct snake_data *snake_info, 
    struct tile board[ROWS][COLS]
);
void copy_board
(
    struct tile init_board[ROWS][COLS],
    struct tile final_board[ROWS][COLS]
);
void add_portal_pair
(
    struct tile board[ROWS][COLS], 
    struct game_data *game_info
);
bool check_valid_portal
(
    int portal_1_row,
    int portal_1_col,
    int portal_2_row,
    int portal_2_col,
    int portal_count,
    struct tile board[ROWS][COLS]
);
void handle_teleport
(
    struct game_data *game_info,
    bool *hit_disabled_portal
);
void teleport_snake
(
    int row_to_teleport, 
    int col_to_teleport, 
    struct snake_data *snake_info
);
void initialise_exploding_apples
(
    struct game_data *game_info,
    int row,
    int col,
    int radius
);
void handle_printing_apple_explode
(
    struct game_data *game_info,
    int apple_explode_row,
    int apple_explode_col
);
void handle_explosion
(
    struct game_data *game_info,
    int snake_head_row,
    int snake_head_col,
    struct tile board[ROWS][COLS]
);
void execute_first_explosion
(
    struct tile board[ROWS][COLS],
    struct game_data *game_info
);
void check_exploded_entity
(
    enum entity curr_entity,
    struct game_data *game_info,
    int explosion_row,
    int explosion_col
);
void update_exploded_apple
(
    struct game_data *game_info,
    int num_pts
);
void disable_portal
(
    struct game_data *game_info,
    int explode_row,
    int explode_col
);
void propagate_explosion
(
    struct game_data *game_info, 
    struct tile board[ROWS][COLS]
);
void draw_explosion_diamond
(
    struct tile board[ROWS][COLS], 
    int centre_row, 
    int centre_col,
    int ring,
    struct game_data *game_info
);
void draw_curr_explode_coords
(
    int explosion_coords[NUM_PAIRS][NUM_COORDS],
    struct tile board[ROWS][COLS],
    struct game_data *game_info
);
void check_and_draw_explosion
(
    int row, 
    int col, 
    struct tile board[ROWS][COLS], 
    struct game_data *game_info
);

// Add your function definitions below this line

static void setup_default_map(void) 
{
    // Example: a border wall + a couple apples
    for (int r = 0; r < ROWS; r++) 
    {
        board[r][0].entity = WALL;
        board[r][COLS-1].entity = WALL;
    }
    for (int c = 0; c < COLS; c++) 
    {
        board[0][c].entity = WALL;
        board[ROWS-1][c].entity = WALL;
    }

    // Place a few apples
    board[2][2].entity = APPLE_NORMAL;  initialise_apples(&game_info, NORMAL_APPLE_PTS);
    board[3][6].entity = APPLE_REVERSE; initialise_apples(&game_info, REVERSE_APPLE_PTS);
    board[6][3].entity = APPLE_SPLIT;   initialise_apples(&game_info, SPLIT_APPLE_PTS);

    // Example exit
    board[1][8].entity = EXIT_LOCKED;
}

void add_entity_to_map
(
    struct tile board[ROWS][COLS], 
    enum entity entity,
    struct game_data *game_info
)
{
    int row, col;
    int explode_radius = 0;

    if (entity == APPLE_EXPLODE)
    {
        scanf("%d", &explode_radius);
    }

    scanf("%d %d", &row, &col);

    if (!is_valid_position(row, col))
    {
        printf("ERROR: Invalid position, %d %d is out of bounds!\n", row, col);
    }
    else if (is_position_occupied(row, col, board))
    {
        printf("ERROR: Invalid tile, %d %d is occupied!\n", row, col);
    }
    else
    {
        if (entity == APPLE_NORMAL)
        {
            initialise_apples(game_info, NORMAL_APPLE_PTS);
        }
        else if (entity == APPLE_REVERSE)
        {
            initialise_apples(game_info, REVERSE_APPLE_PTS);
        }
        else if (entity == APPLE_SPLIT)
        {
            initialise_apples(game_info, SPLIT_APPLE_PTS);
        }
        else if (entity == APPLE_EXPLODE)
        {
            initialise_apples(game_info, EXPLODING_APPLE_PTS);
            initialise_exploding_apples(game_info, row, col, explode_radius);
        }

        board[row][col].entity = entity;
    }
}

bool is_valid_position(int row, int col)
{
    return (row >= FIRST_ROW && row < ROWS) && (col >= FIRST_COL && col < COLS);
}

void add_long_wall(struct tile board[ROWS][COLS])
{
    char direction;
    int row, col, length;

    scanf(" %c %d %d %d", &direction, &row, &col, &length);

    bool is_valid_wall = is_valid_long_wall(row, col, length, direction);
    int temp_row = row;
    int temp_col = col;

    for (int tile = 0; tile < length && is_valid_wall; tile++)
    {
        if (is_position_occupied(temp_row, temp_col, board))
        {
            printf("ERROR: Invalid tile, part of the wall is occupied!\n");
            is_valid_wall = false;
        }
        else
        {
            if (direction == HORIZONTAL)
            {
                temp_col++;
            }
            else if (direction == VERTICAL)
            {
                temp_row++;
            }
        }
    }

    for (int tile = 0; tile < length && is_valid_wall; tile++)
    {
        if (direction == HORIZONTAL)
        {
            board[row][col++].entity = WALL;
        }
        else if (direction == VERTICAL)
        {
            board[row++][col].entity = WALL;
        }
    }
}

bool is_valid_long_wall(int row, int col, int length, char direction)
{
    if (!is_valid_position(row, col)) 
    {
        printf("ERROR: Invalid position, %d %d is out of bounds!\n", row, col);

        return false;
    }

    int end_col = col + length - LAST_TILE_OFFSET;
    int end_row = row + length - LAST_TILE_OFFSET;

    if 
    (
        (direction == HORIZONTAL && !is_valid_position(row, end_col)) ||
        (direction == VERTICAL && !is_valid_position(end_row, col))
    )
    {
        printf("ERROR: Invalid position, part of the wall is out of bounds!\n");

        return false;
    }

    return true;
}

static void spawn_default_snake(void) 
{
    int r = 5, c = 5;
    initialise_snake_rows_and_cols(&game_info.snake_info, r, c);
}

bool is_position_occupied(int row, int col, struct tile board[ROWS][COLS])
{
    return board[row][col].entity != EMPTY;
}

static void handle_key_step(char selection) 
{
    if (is_game_over) return;

    // replicate what your loop did per command
    if (game_info.explosion_info.is_explosion_active && selection != PRINT_STATS) 
    {
        propagate_explosion(&game_info, board);
    }

    bool ignore_collision = false;
    bool has_moved = false;
    bool has_reset = false;
    bool hit_disabled_portal = false;

    int snake_row = game_info.snake_info.head_row;
    int snake_col = game_info.snake_info.head_col;

    board[snake_row][snake_col].entity = BODY_SEGMENT;

    bool is_valid_move = execute_move(
        selection, &game_info, &has_moved,
        &init_game_info, board, init_board,
        &has_reset
    );

    if (has_reset) 
    {
        // after reset, don't instantly die
        return;
    }

    if (has_moved) 
    {
        if (is_valid_move) 
        {
            update_stats_and_board(&game_info, board, &ignore_collision, &hit_disabled_portal);
        }

        check_game_over(
            board, &game_info, &is_game_over,
            ignore_collision, is_valid_move, hit_disabled_portal
        );
    }
}

void unlock_exits(struct tile board[ROWS][COLS])
{
    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            if (board[row][col].entity == EXIT_LOCKED)
            {
                board[row][col].entity = EXIT_UNLOCKED;
            }
        }
    }
}

bool execute_move
(
    char selection, 
    struct game_data *game_info,
    bool *has_moved,
    struct game_data *init_game_info,
    struct tile board[ROWS][COLS],
    struct tile init_board[ROWS][COLS],
    bool *has_reset
)
{
    if (selection == RESET_MAP)
    {
        copy_board(init_board, board);
        *game_info = *init_game_info;
        *has_reset = true;
        game_info->explosion_info.is_explosion_active = false;
    }
    else if (selection == PRINT_STATS)
    {
        print_game_statistics(game_info);
    }
    else if (selection == KEY_UP)
    {
        game_info->snake_info.head_row--;
        game_info->snake_info.move = UP;
        *has_moved = true;
    }
    else if (selection == KEY_LEFT)
    {
        game_info->snake_info.head_col--;
        game_info->snake_info.move = LEFT;
        *has_moved = true;
    }
    else if (selection == KEY_DOWN)
    {
        game_info->snake_info.head_row++;
        game_info->snake_info.move = DOWN;
        *has_moved = true;
    }
    else if (selection == KEY_RIGHT)
    {
        game_info->snake_info.head_col++;
        game_info->snake_info.move = RIGHT;
        *has_moved = true;
    }

    if (*has_moved)
    {
        game_info->num_moves++;
        update_snake_cells(&game_info->snake_info);
    }

    return is_valid_position(game_info->snake_info.head_row, 
                             game_info->snake_info.head_col);
}

double calc_apple_percent(struct apple_data *apple_info)
{
    int num_apples_eaten = apple_info->num_apples_eaten;
    int num_initial_apples = apple_info->num_initial_apples;
    int num_exploded_apples = apple_info->num_exploded_apples;
    // Cast num_apples_removed to double to correctly perform
    // division
    double num_apples_removed = num_apples_eaten + num_exploded_apples;
    double apple_percent;

    if (num_initial_apples > 0)
    {
        apple_percent = HUNDRED * (num_apples_removed / num_initial_apples);
    }
    else
    {
        apple_percent = HUNDRED;
    }

    apple_info->apple_percentage = apple_percent;

    return apple_percent;
}

void update_stats_and_board
(
    struct game_data *game_info, 
    struct tile board[ROWS][COLS],
    bool *ignore_collision,
    bool *hit_disabled_portal
)
{
    int snake_row = game_info->snake_info.head_row;
    int snake_col = game_info->snake_info.head_col;

    if (board[snake_row][snake_col].entity == PORTAL)
    {
        handle_teleport(game_info, hit_disabled_portal);
        snake_row = game_info->snake_info.head_row;
        snake_col = game_info->snake_info.head_col;
    }

    if (board[snake_row][snake_col].entity == APPLE_NORMAL)
    {
        update_apples(game_info, NORMAL_APPLE_PTS);
    }
    else if (board[snake_row][snake_col].entity == APPLE_REVERSE)
    {
        update_apples(game_info, REVERSE_APPLE_PTS);
        reverse_snake(&game_info->snake_info, board);

        *ignore_collision = true;
    }
    else if (board[snake_row][snake_col].entity == APPLE_SPLIT)
    {
        update_apples(game_info, SPLIT_APPLE_PTS);
        split_snake(&game_info->snake_info, board);
    }
    else if (board[snake_row][snake_col].entity == APPLE_EXPLODE)
    {
        update_apples(game_info, EXPLODING_APPLE_PTS);
        handle_explosion(game_info, snake_row, snake_col, board);
    }
    else if 
    (
        board[snake_row][snake_col].entity == PASSAGE_UP ||
        board[snake_row][snake_col].entity == PASSAGE_DOWN ||
        board[snake_row][snake_col].entity == PASSAGE_LEFT ||
        board[snake_row][snake_col].entity == PASSAGE_RIGHT
    )
    {
        handle_passage(board, &game_info->snake_info, 
                       ignore_collision, snake_row,
                       snake_col);
    }

    if (game_info->apple_info.num_apples_remaining == NONE_REMAINING)
    {
        unlock_exits(board);
    }
}

void check_game_over
(
    struct tile board[ROWS][COLS], 
    struct game_data *game_info,
    bool *is_game_over,
    bool ignore_collision,
    bool is_position_valid,
    bool hit_disabled_portal
)
{
    bool has_won = false;
    int snake_row = game_info->snake_info.head_row;
    int snake_col = game_info->snake_info.head_col;

    if (!is_position_valid)
    {
        *is_game_over = true;
    }
    else if (hit_disabled_portal)
    {
        *is_game_over = true;
    }
    else if (is_pos_non_consumable(snake_row, snake_col, board))
    {
        if (!ignore_collision)
        {
            *is_game_over = true;
        }
    }
    else if (board[snake_row][snake_col].entity == EXIT_UNLOCKED)
    {
        has_won = true;
        *is_game_over = true;
    }
    else if (board[snake_row][snake_col].entity == EXPLOSION) 
    {
        *is_game_over = true;
    }

    if (*is_game_over == true)
    {
        printf("--- Game Over ---\n");

        if (has_won)
        {
            printf("Ssslithered out with a full stomach!\n");
        }
        else
        {
            printf("Guessss I was the prey today.\n");
        }

        print_game_statistics(game_info);
    }
}

bool is_pos_non_consumable
(
    int row, 
    int col, 
    struct tile board[ROWS][COLS]
)
{
    return (board[row][col].entity == BODY_SEGMENT ||
            board[row][col].entity == EXIT_LOCKED ||
            board[row][col].entity == WALL ||
            board[row][col].entity == PASSAGE_UP ||
            board[row][col].entity == PASSAGE_DOWN ||
            board[row][col].entity == PASSAGE_LEFT ||
            board[row][col].entity == PASSAGE_RIGHT);
}

void add_apple_or_direct_passage
(
    struct tile board[ROWS][COLS], 
    struct game_data *game_info
)
{
    char selection;

    scanf(" %c", &selection);

    if (selection == ADD_NORMAL_APPLE)
    {
        add_entity_to_map(board, APPLE_NORMAL, game_info);
    }
    else if (selection == ADD_REVERSE_APPLE)
    {
        add_entity_to_map(board, APPLE_REVERSE, game_info);
    }
    else if (selection == ADD_SPLIT_APPLE)
    {
        add_entity_to_map(board, APPLE_SPLIT, game_info);
    }
    else if (selection == ADD_EXPLODING_APPLE)
    {
        add_entity_to_map(board, APPLE_EXPLODE, game_info);
    }
    else if (selection == ADD_PASS_UP)
    {
        add_entity_to_map(board, PASSAGE_UP, game_info);
    }
    else if (selection == ADD_PASS_DOWN)
    {
        add_entity_to_map(board, PASSAGE_DOWN, game_info);
    }
    else if (selection == ADD_PASS_LEFT)
    {
        add_entity_to_map(board, PASSAGE_LEFT, game_info);
    }
    else if (selection == ADD_PASS_RIGHT)
    {
        add_entity_to_map(board, PASSAGE_RIGHT, game_info);
    }
}

void update_apples(struct game_data *game_info, int num_pts)
{
    game_info->apple_info.num_apples_eaten++;
    game_info->apple_info.num_apples_remaining--;
    game_info->num_points += num_pts;
    game_info->num_points_remaining -= num_pts;
}

void handle_passage
(
    struct tile board[ROWS][COLS], 
    struct snake_data *snake_info, 
    bool *ignore_collision,
    int row,
    int col
)
{
    enum entity passage = board[row][col].entity;

    if 
    (
        (snake_info->move == UP && passage == PASSAGE_UP) ||
        (snake_info->move == DOWN && passage == PASSAGE_DOWN) ||
        (snake_info->move == LEFT && passage == PASSAGE_LEFT) ||
        (snake_info->move == RIGHT && passage == PASSAGE_RIGHT)
    )
    {
        board[row][col].entity = EMPTY;
        *ignore_collision = true;
    }
}

void initialise_apples(struct game_data *game_info, int num_pts)
{
    game_info->apple_info.num_apples_remaining++;
    game_info->apple_info.num_initial_apples++;
    game_info->num_points_remaining += num_pts;
}

void initialise_snake_rows_and_cols
(
    struct snake_data *snake_info,
    int snake_row,
    int snake_col
)
{
    snake_info->head_row = snake_row;
    snake_info->head_col = snake_col;
    snake_info->tail_row = snake_row;
    snake_info->tail_col = snake_col;

    update_snake_cells(snake_info);
}

void update_snake_cells(struct snake_data *snake_info)
{
    if (snake_info->cell_count < MAX_BOARD_CELLS)
    {
        snake_info->cells[snake_info->cell_count].row = snake_info->head_row;
        snake_info->cells[snake_info->cell_count].col = snake_info->head_col;
        snake_info->cell_count++;
    }
}

void reverse_snake
(
    struct snake_data *snake_info, 
    struct tile board[ROWS][COLS]
)
{
    int init_snake_row = snake_info->tail_row;
    int init_snake_col = snake_info->tail_col;

    board[snake_info->head_row][snake_info->head_col].entity = BODY_SEGMENT;

    snake_info->tail_row = snake_info->head_row;
    snake_info->tail_col = snake_info->head_col;
    snake_info->head_row = init_snake_row;
    snake_info->head_col = init_snake_col;

    reverse_snake_cells(snake_info->cells, snake_info->cell_count);
}

void reverse_snake_cells(struct snake_cells cells[], int cell_count)
{
    int start = FIRST_SNAKE_CELL;
    int end = cell_count - 1;

    while (start < end)
    {
        struct snake_cells temp = cells[start];

        cells[start] = cells[end];
        cells[end] = temp;

        start++;
        end--;
    }
}

void split_snake
(
    struct snake_data *snake_info, 
    struct tile board[ROWS][COLS]
)
{
    int actual_seg_count = snake_info->cell_count - EXCLUDE_HEAD_OFFSET;
    bool is_even_count = (actual_seg_count % SPLIT_DIVISOR == 0);
    int num_segs_removed;

    if (is_even_count)
    {
        num_segs_removed = actual_seg_count / SPLIT_DIVISOR;
    }
    else
    {
        num_segs_removed = (actual_seg_count + ROUND_UP_OFFSET) / SPLIT_DIVISOR;
    }

    for (int i = 0; i < num_segs_removed; i++)
    {
        int row = snake_info->cells[i].row;
        int col = snake_info->cells[i].col;
        board[row][col].entity = EMPTY;
    }

    int new_cell_count = snake_info->cell_count - num_segs_removed;

    for (int i = 0; i < new_cell_count; i++)
    {
        snake_info->cells[i] = snake_info->cells[num_segs_removed + i];
    }

    snake_info->cell_count = new_cell_count;
    snake_info->tail_row = snake_info->cells[FIRST_SNAKE_CELL].row;
    snake_info->tail_col = snake_info->cells[FIRST_SNAKE_CELL].col;
}

void copy_board
(
    struct tile init_board[ROWS][COLS],
    struct tile final_board[ROWS][COLS]
)
{
    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            enum entity curr_entity = init_board[row][col].entity;
            final_board[row][col].entity = curr_entity;
        }
    }
}

void add_portal_pair
(
    struct tile board[ROWS][COLS], 
    struct game_data *game_info
)
{
    int portal_1_row, portal_1_col, portal_2_row, portal_2_col;
    int portal_count = game_info->portal_count;

    scanf("%d %d %d %d", &portal_1_row, &portal_1_col, 
                         &portal_2_row, &portal_2_col);

    bool is_valid_portal = check_valid_portal(portal_1_row, portal_1_col, 
                                              portal_2_row, portal_2_col,
                                              portal_count,
                                              board);

    if (is_valid_portal)
    {
        board[portal_1_row][portal_1_col].entity = PORTAL;
        board[portal_2_row][portal_2_col].entity = PORTAL;

        game_info->portals[portal_count].portal_1_row = portal_1_row;
        game_info->portals[portal_count].portal_1_col = portal_1_col;
        game_info->portals[portal_count].portal_2_row = portal_2_row;
        game_info->portals[portal_count].portal_2_col = portal_2_col;
        game_info->portals[portal_count].is_pair_disabled = false;

        game_info->portal_count++;
    }
}

bool check_valid_portal
(
    int portal_1_row, 
    int portal_1_col, 
    int portal_2_row, 
    int portal_2_col,
    int portal_count,
    struct tile board[ROWS][COLS]
)
{
    bool is_valid_portal = false;

    if (portal_count >= MAX_PORTAL_PAIRS)
    {
        printf("ERROR: Invalid placement, maximum number of portal pairs" 
               "already reached!\n");
    }
    else if (!is_valid_position(portal_1_row, portal_1_col))
    {
        printf("ERROR: Invalid position for first portal in pair, %d " 
               "%d is out of bounds!\n", portal_1_row, portal_1_col);
    }
    else if (is_position_occupied(portal_1_row, portal_1_col, board))
    {
        printf("ERROR: Invalid tile for first portal in pair, %d " 
               "%d is occupied!\n", portal_1_row, portal_1_col);
    }
    else if (!is_valid_position(portal_2_row, portal_2_col))
    {
        printf("ERROR: Invalid position for second portal in pair, %d " 
               "%d is out of bounds!\n", portal_2_row, portal_2_col);
    }
    else if 
    (
        is_position_occupied(portal_2_row, portal_2_col, board) &&
        board[portal_2_row][portal_2_col].entity != PORTAL
    )
    {
        printf("ERROR: Invalid tile for second portal in pair, %d " 
               "%d is occupied!\n", portal_2_row, portal_2_col);
    }
    else
    {
        is_valid_portal = true;
    }

    return is_valid_portal;
}

void handle_teleport
(
    struct game_data *game_info,
    bool *hit_disabled_portal
)
{
    int head_row = game_info->snake_info.head_row;
    int head_col = game_info->snake_info.head_col;
    int row_to_teleport = head_row;
    int col_to_teleport = head_col;
    bool portal_found = false;

    for (int i = 0; i < game_info->portal_count && !portal_found; i++)
    {
        struct portal_pair ptrl_pair = game_info->portals[i];

        if 
        (
            head_row == ptrl_pair.portal_1_row && 
            head_col == ptrl_pair.portal_1_col
        )
        {
            if (ptrl_pair.is_pair_disabled)
            {
                *hit_disabled_portal = true;
                portal_found = true;
            }
            else
            {
                row_to_teleport = ptrl_pair.portal_2_row;
                col_to_teleport = ptrl_pair.portal_2_col;
                portal_found = true;
            }
        }
        else if 
        (
            head_row == ptrl_pair.portal_2_row && 
            head_col == ptrl_pair.portal_2_col
        )
        {
            if (ptrl_pair.is_pair_disabled)
            {
                *hit_disabled_portal = true;
                portal_found = true;
            }
            else
            {
                row_to_teleport = ptrl_pair.portal_1_row;
                col_to_teleport = ptrl_pair.portal_1_col;
                portal_found = true;
            }
        }
    }

    if (!(*hit_disabled_portal))
    {
        teleport_snake
        (
            row_to_teleport, 
            col_to_teleport, 
            &game_info->snake_info
        );
    }
}

void teleport_snake
(
    int row_to_teleport, 
    int col_to_teleport, 
    struct snake_data *snake_info
)
{
    enum snake_move move = snake_info->move;

    if (snake_info->cell_count > 0)
    {
        snake_info->cell_count--;
    }
    
    snake_info->head_row = row_to_teleport;
    snake_info->head_col = col_to_teleport;

    if (move == UP)
    {
        snake_info->head_row--;
    }
    else if (move == LEFT)
    {
        snake_info->head_col--;
    }
    else if (move == DOWN)
    {
        snake_info->head_row++;
    }
    else if (move == RIGHT)
    {
        snake_info->head_col++;
    }

    update_snake_cells(snake_info);
}

void initialise_exploding_apples
(
    struct game_data *game_info,
    int row,
    int col,
    int radius
)
{
    int exploding_apple_count = game_info->exploding_apple_count;

    game_info->expl_apples[exploding_apple_count].row = row;
    game_info->expl_apples[exploding_apple_count].col = col;
    game_info->expl_apples[exploding_apple_count].radius = radius;
    
    game_info->exploding_apple_count++;
}

void handle_printing_apple_explode
(
    struct game_data *game_info,
    int apple_explode_row,
    int apple_explode_col
)
{
    int apple_explode_radius = 0;
    int apple_explode_count = game_info->exploding_apple_count;
    bool found = false;

    for (int i = 0; i < apple_explode_count && !found; i++)
    {
        if
        (
            game_info->expl_apples[i].row == apple_explode_row &&
            game_info->expl_apples[i].col == apple_explode_col
        )
        {
            found = true;
            apple_explode_radius = game_info->expl_apples[i].radius;
        }
    }

    printf("(%d)", apple_explode_radius);
}

void handle_explosion
(
    struct game_data *game_info,
    int snake_head_row,
    int snake_head_col,
    struct tile board[ROWS][COLS]
)
{
    int radius = 1;
    int apple_explode_count = game_info->exploding_apple_count;
    bool found = false;

    for (int i = 0; i < apple_explode_count && !found; i++)
    {
        if 
        (
            game_info->expl_apples[i].row == snake_head_row &&
            game_info->expl_apples[i].col == snake_head_col
        )
        {
            radius = game_info->expl_apples[i].radius;
            found = true;
        }
    }

    game_info->explosion_info.is_explosion_active = true;
    game_info->explosion_info.explosion_radius = radius;
    game_info->explosion_info.explosion_centre_row = snake_head_row;
    game_info->explosion_info.explosion_centre_col = snake_head_col;
    game_info->explosion_info.explosion_turn = 0;

    board[snake_head_row][snake_head_col].entity = EMPTY;

    execute_first_explosion(board, game_info);
}

void execute_first_explosion
(
    struct tile board[ROWS][COLS],
    struct game_data *game_info
)
{
    int first_explosion_coords[NUM_PAIRS][NUM_COORDS] = {
        {UP_ROW, UP_COL}, 
        {DOWN_ROW, DOWN_COL},
        {LEFT_ROW, LEFT_COL},
        {RIGHT_ROW, RIGHT_COL}
    };
    int center_row = game_info->explosion_info.explosion_centre_row;
    int center_col = game_info->explosion_info.explosion_centre_col;

    for (int i = 0; i < NUM_PAIRS; i++)
    {
        int row = center_row + first_explosion_coords[i][ELEM_1_OF_PAIR];
        int col = center_col + first_explosion_coords[i][ELEM_2_OF_PAIR];

        check_and_draw_explosion(row, col, board, game_info);
    }
}

void check_exploded_entity
(
    enum entity curr_entity,
    struct game_data *game_info,
    int explode_row,
    int explode_col
)
{
    if (curr_entity == APPLE_NORMAL)
    {
        update_exploded_apple(game_info, NORMAL_APPLE_PTS);
    }
    else if (curr_entity == APPLE_REVERSE)
    {
        update_exploded_apple(game_info, REVERSE_APPLE_PTS);
    }
    else if (curr_entity == APPLE_SPLIT)
    {
        update_exploded_apple(game_info, SPLIT_APPLE_PTS);
    }
    else if (curr_entity == APPLE_EXPLODE)
    {
        update_exploded_apple(game_info, EXPLODING_APPLE_PTS);
    }
    else if (curr_entity == PORTAL)
    {
        disable_portal(game_info, explode_row, explode_col);
    }
}

void update_exploded_apple
(
    struct game_data *game_info,
    int num_pts
)
{
    game_info->apple_info.num_apples_remaining--;
    game_info->num_points_remaining -= num_pts;
    game_info->apple_info.num_exploded_apples++;
}

void disable_portal
(
    struct game_data *game_info,
    int explode_row,
    int explode_col
)
{
    bool found = false;

    for (int i = 0; i < game_info->portal_count && !found; i++)
    {
        struct portal_pair *pair = &game_info->portals[i];

        if 
        (
            pair->portal_1_row == explode_row &&
            pair->portal_1_col == explode_col
        )
        {
            pair->is_pair_disabled = true;
            found = true;
        }
        else if 
        (
            pair->portal_2_row == explode_row &&
            pair->portal_2_col == explode_col
        )
        {
            pair->is_pair_disabled = true;
            found = true;
        }
    }
}

void propagate_explosion
(
    struct game_data *game_info, 
    struct tile board[ROWS][COLS]
)
{
    struct explosion_data *exp = &game_info->explosion_info;

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            if (board[row][col].entity == EXPLOSION)
            {
                board[row][col].entity = EMPTY;
            }
        }
    }

    // end explosion if radius is 1
    if (exp->explosion_radius == 1)
    {
        exp->is_explosion_active = false;
    }

    if (exp->is_explosion_active)
    {
        int centre_row = exp->explosion_centre_row;
        int centre_col = exp->explosion_centre_col;
        // offset start by 2 (after the initial cross)
        int ring = exp->explosion_turn + INIT_RING_OFFSET;

        // end explosion if diamond outline ring equals
        // valid radius + 1
        if (ring == exp->explosion_radius + FINAL_RING_OFFSET)
        {
            exp->is_explosion_active = false;
        }

        if (exp->is_explosion_active)
        {
            draw_explosion_diamond
            (
                board, 
                centre_row, 
                centre_col,
                ring,
                game_info
            );
        }
    }

    exp->explosion_turn++;
}

void draw_explosion_diamond
(
    struct tile board[ROWS][COLS], 
    int centre_row, 
    int centre_col,
    int ring,
    struct game_data *game_info
)
{
    int diam_row;

    for (diam_row = 0; diam_row <= ring; diam_row++)
    {
        int diam_col = ring - diam_row;

        // this 2D array formatting is to pass the style
        // checker, as it requires indenting the array
        // pair on a different line to the bracket
        int explosion_coords[NUM_PAIRS][NUM_COORDS] = {
            { 
                centre_row - diam_row, centre_col - diam_col 
            }, 
            { 
                centre_row - diam_row, centre_col + diam_col 
            },
            { 
                centre_row + diam_row, centre_col - diam_col 
            },
            { 
                centre_row + diam_row, centre_col + diam_col 
            }
        };

        draw_curr_explode_coords
        (
            explosion_coords,
            board,
            game_info
        );
    }
}

void draw_curr_explode_coords
(
    int explosion_coords[NUM_PAIRS][NUM_COORDS],
    struct tile board[ROWS][COLS],
    struct game_data *game_info
)
{
    for (int i = 0; i < NUM_PAIRS; i++)
    {
        int row = explosion_coords[i][ELEM_1_OF_PAIR];
        int col = explosion_coords[i][ELEM_2_OF_PAIR];

        check_and_draw_explosion(row, col, board, game_info);
    }
}

void check_and_draw_explosion
(
    int row, 
    int col, 
    struct tile board[ROWS][COLS], 
    struct game_data *game_info
)
{
    if (is_valid_position(row, col))
    {
        enum entity curr_entity = board[row][col].entity;

        if 
        (
            curr_entity != EXIT_LOCKED && 
            curr_entity != EXIT_UNLOCKED
        )
        {
            check_exploded_entity
            (
                curr_entity,
                game_info,
                row,
                col
            );

            board[row][col].entity = EXPLOSION;
        }
    }
}

// =============================================================================
// EDITABLE Provided Functions
// =============================================================================

// Given a 2D board array, initialise all tiles to EMPTY.
void initialise_board
(
    struct tile board[ROWS][COLS],
    struct tile init_board[ROWS][COLS]
)
{
    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            board[row][col].entity = EMPTY;
            init_board[row][col].entity = EMPTY;
        }
    }
}

// Prints the game board, showing the snake's head position on the board.
void print_board
(
    struct tile board[ROWS][COLS], 
    int snake_row, 
    int snake_col,
    struct game_data *game_info
)
{
    print_board_line();
    print_board_header();
    print_board_line();

    for (int row = 0; row < ROWS; row++)
    {
        print_tile_spacer();

        for (int col = 0; col < COLS; col++)
        {
            printf(" ");
            struct tile tile = board[row][col];
            if (row == snake_row && col == snake_col)
            {
                printf("^~^");
            }
            else if (tile.entity == WALL)
            {
                printf("|||");
            }
            else if (tile.entity == BODY_SEGMENT)
            {
                printf("###");
            }
            else if (tile.entity == EXIT_LOCKED)
            {
                printf("[X]");
            }
            else if (tile.entity == EXIT_UNLOCKED)
            {
                printf("[ ]");
            }
            else if (tile.entity == APPLE_NORMAL)
            {
                printf("(`)");
            }
            else if (tile.entity == APPLE_REVERSE)
            {
                printf("(R)");
            }
            else if (tile.entity == APPLE_SPLIT)
            {
                printf("(S)");
            }
            else if (tile.entity == APPLE_EXPLODE)
            {
                handle_printing_apple_explode(game_info, row, col);
            }
            else if (tile.entity == PASSAGE_UP)
            {
                printf("^^^");
            }
            else if (tile.entity == PASSAGE_DOWN)
            {
                printf("vvv");
            }
            else if (tile.entity == PASSAGE_LEFT)
            {
                printf("<<<");
            }
            else if (tile.entity == PASSAGE_RIGHT)
            {
                printf(">>>");
            }
            else if (tile.entity == PORTAL)
            {
                printf("~O~");
            }
            else if (tile.entity == EXPLOSION)
            {
                printf("***");
            }
            else
            {
                printf("   ");
            }
        }

        printf("\n");
    }

    print_tile_spacer();
}

// =============================================================================
// DO NOT EDIT Provided Functions 
// =============================================================================

// Prints statistics about the game
void print_game_statistics(struct game_data *game_info)
{
    int points = game_info->num_points;
    int moves_made = game_info->num_moves;
    int num_apples_eaten = game_info->apple_info.num_apples_eaten;
    int num_apples_remaining = game_info->apple_info.num_apples_remaining;
    double completion_percentage = calc_apple_percent(&game_info->apple_info);
    int maximum_points_remaining = game_info->num_points_remaining;

    printf("============ Game Statistics ============\n");
    printf("Totals:\n");
    printf("  - Points: %d\n", points);
    printf("  - Moves Made: %d\n", moves_made);
    printf("  - Number of Apples Eaten: %d\n", num_apples_eaten);
    printf("Completion:\n");
    printf("  - Number of Apples Remaining: %d\n", num_apples_remaining);
    printf("  - Apple Completion Percentage: %.1f%%\n", completion_percentage);
    printf("  - Maximum Points Remaining: %d\n", maximum_points_remaining);
    printf("=========================================\n");
}

// Prints statistics about the game for both snakes when there are two players
void print_game_statistics_with_rival
(
    int original_points,
    int original_moves_made,
    int original_num_apples_eaten,
    int rival_points,
    int rival_moves_made,
    int rival_num_apples_eaten,
    int num_apples_remaining,
    double completion_percentage,
    int maximum_points_remaining
)
{
    printf("============ Game Statistics ============\n");
    printf("Original Snake Totals:\n");
    printf("  - Points: %d\n", original_points);
    printf("  - Moves Made: %d\n", original_moves_made);
    printf("  - Number of Apples Eaten: %d\n", original_num_apples_eaten);
    printf("Rival Snake Totals:\n");
    printf("  - Points: %d\n", rival_points);
    printf("  - Moves Made: %d\n", rival_moves_made);
    printf("  - Number of Apples Eaten: %d\n", rival_num_apples_eaten);
    printf("Completion:\n");
    printf("  - Number of Apples Remaining: %d\n", num_apples_remaining);
    printf("  - Apple Completion Percentage: %.1f%%\n", completion_percentage);
    printf("  - Maximum Points Remaining: %d\n", maximum_points_remaining);
    printf("=========================================\n");
}

// Helper function for print_board().
void print_board_header(void)
{
    printf("|            C S _ S N A K E            |\n");
}

// Helper function for print_board(). 
void print_board_line(void)
{
    printf("+");

    for (int col = 0; col < COLS; col++)
    {
        printf("---+");
    }

    printf("\n");
}

// Helper function for print_board(). 
void print_tile_spacer(void)
{
    printf("+");
    
    for (int col = 0; col < COLS; col++)
    {
        printf("   +");
    }

    printf("\n");
}

EMSCRIPTEN_KEEPALIVE
void game_init(void) {
    game_info = (struct game_data){0};
    init_game_info = (struct game_data){0};
    is_game_over = false;

    initialise_board(board, init_board);
    setup_default_map();
    spawn_default_snake();

    copy_board(board, init_board);
    init_game_info = game_info;
}

EMSCRIPTEN_KEEPALIVE
void game_handle_key(int ch) {
    handle_key_step((char)ch);
}

EMSCRIPTEN_KEEPALIVE
int game_get_entity(int r, int c) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return EMPTY;
    return (int)board[r][c].entity;
}

EMSCRIPTEN_KEEPALIVE
int game_get_head_row(void) {
    return game_info.snake_info.head_row;
}

EMSCRIPTEN_KEEPALIVE
int game_get_head_col(void) {
    return game_info.snake_info.head_col;
}

EMSCRIPTEN_KEEPALIVE
int game_is_over(void) {
    return is_game_over ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
int game_get_points(void) {
    return game_info.num_points;
}