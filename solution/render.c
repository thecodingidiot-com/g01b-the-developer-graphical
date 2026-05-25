#include <stdio.h>
#include <string.h>
#include "game.h"

#define PAD     16
#define LINE_H  22

/* left panel zone tops */
#define Q_Y     10
#define ANS_Y   188
#define HELP_Y  424

/* right ladder panel */
#define LDDR_X  572
#define LDDR_Y  12
#define LDDR_H  38

static void draw_background(game_t *g)
{
    SDL_Texture *bg;
    SDL_Rect     dst;

    if (g->state == STATE_CORRECT || g->state == STATE_WIN)
        bg = g->bg_correct;
    else if (g->state == STATE_WRONG || g->state == STATE_GAMEOVER)
        bg = g->bg_wrong;
    else
        bg = g->bg_studio;
    dst.x = 0;
    dst.y = 0;
    dst.w = WIN_W;
    dst.h = WIN_H;
    SDL_RenderCopy(g->ren, bg, NULL, &dst);
}

static void draw_lifeline_bar(game_t *g, int y)
{
    char    buf[128];

    snprintf(buf, sizeof(buf), "%s%s%s[W] Walk away",
        (g->lifelines & 1) ? "[1] 50:50  " : "",
        (g->lifelines & 2) ? "[2] Phone  " : "",
        (g->lifelines & 4) ? "[3] Audience  " : "");
    draw_string(g, PAD, y, buf);
}

static void draw_audience(game_t *g, int y)
{
    const char  *letters;
    char         buf[64];
    int          i;
    int          j;
    int          pos;

    letters = "ABCD";
    draw_string(g, PAD, y, "Ask the Audience:");
    y += LINE_H;
    for (i = 0; i < 4; i++) {
        pos = 0;
        buf[pos++] = letters[i];
        buf[pos++] = ':';
        buf[pos++] = ' ';
        for (j = 0; j < g->audience[i] / 5 && pos < 55; j++)
            buf[pos++] = '#';
        buf[pos++] = ' ';
        if (g->audience[i] >= 100) {
            buf[pos++] = '1'; buf[pos++] = '0'; buf[pos++] = '0';
        } else if (g->audience[i] >= 10) {
            buf[pos++] = '0' + g->audience[i] / 10;
            buf[pos++] = '0' + g->audience[i] % 10;
        } else {
            buf[pos++] = '0' + g->audience[i];
        }
        buf[pos++] = '%';
        buf[pos] = '\0';
        draw_string(g, PAD, y + i * LINE_H, buf);
    }
}

static void draw_ladder(game_t *g)
{
    char    buf[48];
    int     i;
    int     y;
    char    marker;

    for (i = LEVELS - 1; i >= 0; i--) {
        y = LDDR_Y + (LEVELS - 1 - i) * LDDR_H;
        if (i == g->level)
            marker = '>';
        else if (SAFE[i])
            marker = '*';
        else if (i < g->level)
            marker = '+';
        else
            marker = ' ';
        snprintf(buf, sizeof(buf), "%c %s", marker, PRIZES[i]);
        draw_string(g, LDDR_X, y, buf);
    }
}

static int draw_wrapped(game_t *g, int x, int y, int max_w, const char *s)
{
    char    buf[512];
    char    line[512];
    char   *word;
    char   *saveptr;
    int     line_w;
    int     word_w;
    int     space_w;
    int     h;

    snprintf(buf, sizeof(buf), "%s", s);
    line[0] = '\0';
    line_w = 0;
    TTF_SizeUTF8(g->font, " ", &space_w, &h);
    word = strtok_r(buf, " ", &saveptr);
    while (word != NULL) {
        TTF_SizeUTF8(g->font, word, &word_w, &h);
        if (line_w > 0 && line_w + space_w + word_w > max_w) {
            draw_string(g, x, y, line);
            y += LINE_H;
            line[0] = '\0';
            line_w = 0;
        }
        if (line_w > 0) {
            tci_strlcat(line, " ", sizeof(line));
            line_w += space_w;
        }
        tci_strlcat(line, word, sizeof(line));
        line_w += word_w;
        word = strtok_r(NULL, " ", &saveptr);
    }
    if (line[0] != '\0') {
        draw_string(g, x, y, line);
        y += LINE_H;
    }
    return (y);
}

static void draw_title(game_t *g)
{
    (void)g;
    draw_string(g, PAD, Q_Y, "WHO WANTS TO BE A GAME DEVELOPER?");
    draw_string(g, PAD, ANS_Y, "Press Enter to start.");
    draw_string(g, PAD, ANS_Y + LINE_H, "Press Escape to quit.");
}

static void draw_question(game_t *g)
{
    question_t  *q;
    const char  *letters;
    char         buf[256];
    int          y;
    int          i;

    q = g->questions[g->level];
    letters = "ABCD";
    draw_string(g, PAD, Q_Y, PRIZES[g->level]);
    draw_wrapped(g, PAD, Q_Y + LINE_H, 560 - PAD * 2, q->text);
    y = ANS_Y;
    for (i = 0; i < 4; i++) {
        if (g->hidden[i])
            continue;
        snprintf(buf, sizeof(buf), "%c. %s", letters[i], q->opts[i]);
        draw_string(g, PAD, y, buf);
        y += LINE_H + 4;
    }
    y = HELP_Y + PAD;
    draw_lifeline_bar(g, y);
    y += LINE_H;
    if (g->phone_active) {
        if (q->hint)
            y = draw_wrapped(g, PAD, y, 560 - PAD * 2, q->hint);
        else {
            snprintf(buf, sizeof(buf), "The answer is %c.", letters[q->answer]);
            draw_string(g, PAD, y, buf);
            y += LINE_H;
        }
    }
    if (g->audience[0] || g->audience[1] || g->audience[2] || g->audience[3])
        draw_audience(g, y);
}

static void draw_confirm(game_t *g)
{
    question_t  *q;
    const char  *letters;
    char         buf[512];
    int          y;
    int          i;

    q = g->questions[g->level];
    letters = "ABCD";
    draw_string(g, PAD, Q_Y, PRIZES[g->level]);
    draw_wrapped(g, PAD, Q_Y + LINE_H, 560 - PAD * 2, q->text);
    y = ANS_Y;
    for (i = 0; i < 4; i++) {
        if (g->hidden[i])
            continue;
        snprintf(buf, sizeof(buf), "%c. %s", letters[i], q->opts[i]);
        draw_string(g, PAD, y, buf);
        y += LINE_H + 4;
    }
    draw_string(g, PAD, HELP_Y + PAD, "Is that your final answer?");
    snprintf(buf, sizeof(buf),
        "Answer: %c  --  Enter to confirm, Backspace to go back.",
        g->pending);
    draw_string(g, PAD, HELP_Y + PAD + LINE_H, buf);
}

static void draw_correct(game_t *g)
{
    question_t  *q;
    int          y;

    q = g->questions[g->level];
    draw_string(g, PAD, Q_Y, "Correct!");
    y = draw_wrapped(g, PAD, ANS_Y, 560 - PAD * 2, q->text);
    draw_string(g, PAD, y + 4, "The answer was:");
    draw_string(g, PAD, y + 4 + LINE_H, q->opts[q->answer]);
    draw_string(g, PAD, HELP_Y + PAD, "Press Space to continue.");
}

static void draw_wrong(game_t *g)
{
    question_t  *q;
    const char  *prize;
    int          y;

    q = g->questions[g->level];
    draw_string(g, PAD, Q_Y, "Wrong!");
    y = draw_wrapped(g, PAD, ANS_Y, 560 - PAD * 2, q->text);
    draw_string(g, PAD, y + 4, "The answer was:");
    draw_string(g, PAD, y + 4 + LINE_H, q->opts[q->answer]);
    prize = g->safe_level >= 0 ? PRIZES[g->safe_level] : "£0";
    draw_string(g, PAD, HELP_Y + PAD, "You leave with:");
    draw_string(g, PAD, HELP_Y + PAD + LINE_H, prize);
    draw_string(g, PAD, HELP_Y + PAD + LINE_H * 2, "Press Escape to quit.");
}

static void draw_win(game_t *g)
{
    (void)g;
    draw_string(g, PAD, Q_Y, "YOU HAVE JUST WON £1,000,000!");
    draw_string(g, PAD, ANS_Y, "Congratulations!");
    draw_string(g, PAD, HELP_Y + PAD, "Press Escape to quit.");
}

static void draw_gameover(game_t *g)
{
    const char  *prize;

    prize = g->safe_level >= 0 ? PRIZES[g->safe_level] : "£0";
    draw_string(g, PAD, Q_Y, "You walk away with:");
    draw_string(g, PAD, ANS_Y, prize);
    draw_string(g, PAD, HELP_Y + PAD, "Press Escape to quit.");
}

int     render_init(game_t *g)
{
    g->bg_studio  = IMG_LoadTexture(g->ren, "assets/bg_studio.png");
    g->bg_correct = IMG_LoadTexture(g->ren, "assets/bg_correct.png");
    g->bg_wrong   = IMG_LoadTexture(g->ren, "assets/bg_wrong.png");
    if (!g->bg_studio || !g->bg_correct || !g->bg_wrong) {
        SDL_Log("render_init: %s", IMG_GetError());
        return (-1);
    }
    return (0);
}

void    render_free(game_t *g)
{
    if (g->bg_studio)  { SDL_DestroyTexture(g->bg_studio);  g->bg_studio  = NULL; }
    if (g->bg_correct) { SDL_DestroyTexture(g->bg_correct); g->bg_correct = NULL; }
    if (g->bg_wrong)   { SDL_DestroyTexture(g->bg_wrong);   g->bg_wrong   = NULL; }
}

void    render_frame(game_t *g)
{
    SDL_RenderClear(g->ren);
    draw_background(g);
    switch (g->state) {
        case STATE_TITLE:    draw_title(g);                    break;
        case STATE_QUESTION: draw_question(g); draw_ladder(g); break;
        case STATE_CONFIRM:  draw_confirm(g);  draw_ladder(g); break;
        case STATE_CORRECT:  draw_correct(g);  draw_ladder(g); break;
        case STATE_WRONG:    draw_wrong(g);    draw_ladder(g); break;
        case STATE_WIN:      draw_win(g);      draw_ladder(g); break;
        case STATE_GAMEOVER: draw_gameover(g); draw_ladder(g); break;
    }
    SDL_RenderPresent(g->ren);
}
