#include "game.h"

static void shuffle(question_t **questions, int count)
{
    int          i;
    int          j;
    question_t  *tmp;

    for (i = count - 1; i > 0; i--) {
        j = rand() % (i + 1);
        tmp = questions[i];
        questions[i] = questions[j];
        questions[j] = tmp;
    }
}

static void handle_title(game_t *g, SDL_Keycode sym)
{
    if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER)
        g->state = STATE_QUESTION;
}

static void handle_question(game_t *g, SDL_Keycode sym)
{
    if (sym == SDLK_a || sym == SDLK_b || sym == SDLK_c || sym == SDLK_d) {
        g->pending = 'A' + (sym - SDLK_a);
        g->state = STATE_CONFIRM;
    } else if (sym == SDLK_1) {
        handle_lifeline(g, 1);
    } else if (sym == SDLK_2) {
        handle_lifeline(g, 2);
    } else if (sym == SDLK_3) {
        handle_lifeline(g, 3);
    } else if (sym == SDLK_w) {
        g->state = STATE_GAMEOVER;
    }
}

static void handle_confirm(game_t *g, SDL_Keycode sym)
{
    if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER)
        evaluate_answer(g);
    else if (sym == SDLK_BACKSPACE)
        g->state = STATE_QUESTION;
}

static void handle_correct(game_t *g, SDL_Keycode sym)
{
    if (sym == SDLK_SPACE)
        next_question(g);
}

void    handle_event(game_t *g, SDL_Event *ev, int *running)
{
    SDL_Keycode sym;

    if (ev->type == SDL_QUIT) {
        *running = 0;
        return;
    }
    if (ev->type != SDL_KEYDOWN)
        return;
    sym = ev->key.keysym.sym;
    if (sym == SDLK_ESCAPE) {
        *running = 0;
        return;
    }
    switch (g->state) {
        case STATE_TITLE:    handle_title(g, sym);    break;
        case STATE_QUESTION: handle_question(g, sym); break;
        case STATE_CONFIRM:  handle_confirm(g, sym);  break;
        case STATE_CORRECT:  handle_correct(g, sym);  break;
        case STATE_WRONG:
        case STATE_WIN:
        case STATE_GAMEOVER:
            break;
    }
}

int main(int argc, char *argv[])
{
    game_t       g;
    question_t **questions;
    int          count;
    SDL_Event    ev;
    int          running;

    if (argc < 2) {
        tci_printf("usage: %s questions.txt\n", argv[0]);
        return (1);
    }
    srand((unsigned int)time(NULL));
    questions = load_questions(argv[1], &count);
    if (!questions || count < LEVELS) {
        tci_printf("need at least %d questions, got %d\n", LEVELS, count);
        free_questions(questions, count);
        return (1);
    }
    shuffle(questions, count);
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        free_questions(questions, count);
        return (1);
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        SDL_Log("IMG_Init: %s", IMG_GetError());
        SDL_Quit();
        free_questions(questions, count);
        return (1);
    }
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init: %s", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        free_questions(questions, count);
        return (1);
    }
    tci_bzero(&g, sizeof(g));
    g.win = SDL_CreateWindow("Who Wants to Be a Game Developer?",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H, 0);
    if (!g.win) {
        SDL_Log("SDL_CreateWindow: %s", SDL_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        free_questions(questions, count);
        return (1);
    }
    g.ren = SDL_CreateRenderer(g.win, -1, SDL_RENDERER_ACCELERATED);
    if (!g.ren) {
        SDL_Log("SDL_CreateRenderer: %s", SDL_GetError());
        SDL_DestroyWindow(g.win);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        free_questions(questions, count);
        return (1);
    }
    if (font_load(&g, "assets/font.ttf", 16) != 0) {
        SDL_DestroyRenderer(g.ren);
        SDL_DestroyWindow(g.win);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        free_questions(questions, count);
        return (1);
    }
    if (render_init(&g) != 0) {
        font_free(&g);
        SDL_DestroyRenderer(g.ren);
        SDL_DestroyWindow(g.win);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        free_questions(questions, count);
        return (1);
    }
    game_init(&g, questions, count);
    running = 1;
    while (running) {
        while (SDL_PollEvent(&ev))
            handle_event(&g, &ev, &running);
        render_frame(&g);
        SDL_Delay(16);
    }
    render_free(&g);
    game_free(&g);
    font_free(&g);
    SDL_DestroyRenderer(g.ren);
    SDL_DestroyWindow(g.win);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    free_questions(questions, count);
    return (0);
}
