#ifndef GAME_H
# define GAME_H

# include <SDL2/SDL.h>
# include <SDL2/SDL_image.h>
# include <SDL2/SDL_ttf.h>
# include <fcntl.h>
# include <unistd.h>
# include <stdlib.h>
# include <time.h>
# include "libtci.h"

# define LEVELS 15
# define WIN_W  800
# define WIN_H  600

typedef struct s_question
{
    char    *text;
    char    *opts[4];
    int      answer;
    char    *hint;
} question_t;

typedef enum e_state
{
    STATE_TITLE,
    STATE_QUESTION,
    STATE_CONFIRM,
    STATE_CORRECT,
    STATE_WRONG,
    STATE_WIN,
    STATE_GAMEOVER
} game_state_t;

typedef struct s_game
{
    SDL_Window      *win;
    SDL_Renderer    *ren;
    SDL_Texture     *bg_studio;
    SDL_Texture     *bg_correct;
    SDL_Texture     *bg_wrong;
    TTF_Font        *font;
    game_state_t     state;
    question_t     **questions;
    int              count;
    int              level;
    int              safe_level;
    int              lifelines;
    int              phone_active;
    int              hidden[4];
    int              audience[4];
    char             pending;
} game_t;

extern const char  *PRIZES[LEVELS];
extern const int    SAFE[LEVELS];

/* load.c */
question_t  **load_questions(const char *path, int *count);
void          free_questions(question_t **questions, int count);

/* game.c */
void    game_init(game_t *g, question_t **questions, int count);
void    game_free(game_t *g);
void    evaluate_answer(game_t *g);
void    handle_lifeline(game_t *g, int lifeline);
void    next_question(game_t *g);

/* font.c */
int     font_load(game_t *g, const char *path, int ptsize);
void    font_free(game_t *g);
void    draw_string(game_t *g, int x, int y, const char *s);

/* render.c */
int     render_init(game_t *g);
void    render_free(game_t *g);
void    render_frame(game_t *g);

/* main.c */
void    handle_event(game_t *g, SDL_Event *ev, int *running);

#endif
