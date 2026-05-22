#include "game.h"

const char  *PRIZES[LEVELS] = {
    "£100", "£200", "£300", "£500", "£1,000",
    "£2,000", "£4,000", "£8,000", "£16,000", "£32,000",
    "£64,000", "£125,000", "£250,000", "£500,000", "£1,000,000"
};

const int   SAFE[LEVELS] = {
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 0
};

void    game_init(game_t *g, question_t **questions, int count)
{
    tci_bzero(g->hidden, sizeof(g->hidden));
    tci_bzero(g->audience, sizeof(g->audience));
    g->state = STATE_TITLE;
    g->questions = questions;
    g->count = count;
    g->level = 0;
    g->safe_level = -1;
    g->lifelines = 7;
    g->phone_active = 0;
    g->pending = 0;
}

void    game_free(game_t *g)
{
    (void)g;
}

void    evaluate_answer(game_t *g)
{
    question_t  *q;

    q = g->questions[g->level];
    if (g->pending - 'A' == q->answer)
        g->state = STATE_CORRECT;
    else
        g->state = STATE_WRONG;
}

void    handle_lifeline(game_t *g, int lifeline)
{
    question_t  *q;
    int          bit;
    int          removed;
    int          i;

    q = g->questions[g->level];
    bit = 1 << (lifeline - 1);
    if (!(g->lifelines & bit))
        return;
    g->lifelines &= ~bit;
    if (lifeline == 1) {
        removed = 0;
        for (i = 0; i < 4 && removed < 2; i++) {
            if (i == q->answer)
                continue;
            g->hidden[i] = 1;
            removed++;
        }
    } else if (lifeline == 2) {
        g->phone_active = 1;
    } else if (lifeline == 3) {
        int correct;
        int spread;

        correct = 55 + (rand() % 30);
        g->audience[q->answer] = correct;
        spread = 100 - correct;
        for (i = 0; i < 4; i++) {
            if (i == q->answer)
                continue;
            if (g->hidden[i]) {
                g->audience[i] = 0;
            } else {
                int portion = spread / 3;
                g->audience[i] = portion;
                spread -= portion;
            }
        }
    }
}

void    next_question(game_t *g)
{
    g->level++;
    if (g->level >= LEVELS) {
        g->state = STATE_WIN;
        return;
    }
    tci_bzero(g->hidden, sizeof(g->hidden));
    tci_bzero(g->audience, sizeof(g->audience));
    g->phone_active = 0;
    g->pending = 0;
    if (SAFE[g->level])
        g->safe_level = g->level;
    g->state = STATE_QUESTION;
}
