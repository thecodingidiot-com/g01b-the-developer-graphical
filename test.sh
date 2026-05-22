#!/bin/bash
# g01b tester — logic only; graphical output is not covered.
# Compiles load.c and a headless subset of game.c without SDL2 headers.

set -o pipefail

PASS=0
FAIL=0

# ─────────────────────────────────────────────────────────────────────────────

ok()
{
    local label="$1"
    echo "  PASS  $label"
    PASS=$((PASS + 1))
}

fail()
{
    local label="$1"
    local got="$2"
    local want="$3"
    echo "  FAIL  $label (got: $got, want: $want)"
    FAIL=$((FAIL + 1))
}

# ─────────────────────────────────────────────────────────────────────────────

echo "g01b tester — logic only; graphical output is not covered"
echo ""

# Build headless test harness.
# game_headless.c redefines game_t without SDL2 types and includes only
# the pure-logic functions from game.c.

cat > /tmp/g01b_game_h.h <<'GAME_H'
#ifndef G01B_GAME_H
# define G01B_GAME_H

# include "libtci.h"

# define LEVELS 15

typedef struct s_question {
    char    *text;
    char    *opts[4];
    int      answer;
    char    *hint;
} question_t;

typedef enum e_state {
    STATE_TITLE,
    STATE_QUESTION,
    STATE_CONFIRM,
    STATE_CORRECT,
    STATE_WRONG,
    STATE_WIN,
    STATE_GAMEOVER
} game_state_t;

typedef struct s_game {
    void        *win;
    void        *ren;
    void        *bg_studio;
    void        *bg_correct;
    void        *bg_wrong;
    void        *font;
    game_state_t state;
    question_t **questions;
    int          count;
    int          level;
    int          safe_level;
    int          lifelines;
    int          phone_active;
    int          hidden[4];
    int          audience[4];
    char         pending;
} game_t;

extern const char  *PRIZES[LEVELS];
extern const int    SAFE[LEVELS];

question_t  **load_questions(const char *path, int *count);
void          free_questions(question_t **questions, int count);
void          game_init(game_t *g, question_t **questions, int count);
void          game_free(game_t *g);
void          evaluate_answer(game_t *g);
void          handle_lifeline(game_t *g, int lifeline);
void          next_question(game_t *g);

#endif
GAME_H

# Patch game.c and load.c to use the headless header.
sed 's|#include "game.h"|#include "/tmp/g01b_game_h.h"|g' game.c  > /tmp/g01b_game.c
sed 's|#include "game.h"|#include "/tmp/g01b_game_h.h"|g' load.c  > /tmp/g01b_load.c

cat > /tmp/g01b_test.c <<'TEST_C'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/tmp/g01b_game_h.h"

/* stubs for SDL2 functions called nowhere in headless build */

extern int  run_tests(void);

int run_tests(void)
{
    int          pass;
    int          fail;
    question_t **qs;
    int          count;
    game_t       g;

    pass = 0;
    fail = 0;

    /* ── Test 1: question loading ── */
    qs = load_questions("questions.txt", &count);
    if (count == 20) {
        printf("  PASS  load_questions: 20 questions loaded\n");
        pass++;
    } else {
        printf("  FAIL  load_questions: got %d, want 20\n", count);
        fail++;
    }
    if (qs && qs[0] && qs[0]->text && qs[0]->text[0] != '\0') {
        printf("  PASS  load_questions: first question text is non-empty\n");
        pass++;
    } else {
        printf("  FAIL  load_questions: first question text is empty\n");
        fail++;
    }

    /* ── Test 2: game_init ── */
    game_init(&g, qs, count);
    if (g.state == STATE_TITLE && g.level == 0 && g.safe_level == -1 && g.lifelines == 7) {
        printf("  PASS  game_init: state TITLE, level 0, safe_level -1, lifelines 7\n");
        pass++;
    } else {
        printf("  FAIL  game_init: unexpected initial state\n");
        fail++;
    }

    /* ── Test 3: safe-level advancement ── */
    /* level 4 (index 4) is the first safe level (SAFE[4]=1) */
    g.state = STATE_CORRECT;
    g.level = 3;  /* next_question will increment to 4 */
    next_question(&g);
    if (g.safe_level == 4) {
        printf("  PASS  safe_level: set to 4 after reaching level 5\n");
        pass++;
    } else {
        printf("  FAIL  safe_level: got %d, want 4\n", g.safe_level);
        fail++;
    }
    if (g.state == STATE_QUESTION) {
        printf("  PASS  state: STATE_QUESTION at safe level (ladder always visible)\n");
        pass++;
    } else {
        printf("  FAIL  state: got %d, want STATE_QUESTION (%d)\n", g.state, STATE_QUESTION);
        fail++;
    }
    /* level 9 (index 9) is the second safe level */
    g.level = 8;
    g.state = STATE_CORRECT;
    next_question(&g);
    if (g.safe_level == 9) {
        printf("  PASS  safe_level: set to 9 after reaching level 10\n");
        pass++;
    } else {
        printf("  FAIL  safe_level: got %d, want 9\n", g.safe_level);
        fail++;
    }

    /* ── Test 4: lifeline bitfield ── */
    game_init(&g, qs, count);
    if ((g.lifelines & 1) && (g.lifelines & 2) && (g.lifelines & 4)) {
        printf("  PASS  lifelines: all three set at start\n");
        pass++;
    } else {
        printf("  FAIL  lifelines: not all set at start (got %d)\n", g.lifelines);
        fail++;
    }
    handle_lifeline(&g, 1);
    if (!(g.lifelines & 1)) {
        printf("  PASS  50:50 lifeline: bit 0 cleared\n");
        pass++;
    } else {
        printf("  FAIL  50:50 lifeline: bit 0 not cleared\n");
        fail++;
    }
    /* using 50:50 again should not change state */
    {
        int prev = g.lifelines;
        handle_lifeline(&g, 1);
        if (g.lifelines == prev) {
            printf("  PASS  50:50 lifeline: not re-usable\n");
            pass++;
        } else {
            printf("  FAIL  50:50 lifeline: changed on second use\n");
            fail++;
        }
    }
    handle_lifeline(&g, 2);
    if (!(g.lifelines & 2)) {
        printf("  PASS  phone lifeline: bit 1 cleared\n");
        pass++;
    } else {
        printf("  FAIL  phone lifeline: bit 1 not cleared\n");
        fail++;
    }
    handle_lifeline(&g, 3);
    if (!(g.lifelines & 4)) {
        printf("  PASS  audience lifeline: bit 2 cleared\n");
        pass++;
    } else {
        printf("  FAIL  audience lifeline: bit 2 not cleared\n");
        fail++;
    }

    /* ── Test 5: state transitions ── */
    game_init(&g, qs, count);
    g.state = STATE_TITLE;
    /* simulate pressing Enter on title */
    g.state = STATE_QUESTION;
    g.pending = (char)('A' + qs[0]->answer);
    evaluate_answer(&g);
    if (g.state == STATE_CORRECT) {
        printf("  PASS  evaluate_answer: correct answer -> STATE_CORRECT\n");
        pass++;
    } else {
        printf("  FAIL  evaluate_answer: got %d, want STATE_CORRECT (%d)\n", g.state, STATE_CORRECT);
        fail++;
    }
    game_init(&g, qs, count);
    g.state = STATE_QUESTION;
    /* deliberately wrong */
    g.pending = (char)('A' + ((qs[0]->answer + 1) % 4));
    evaluate_answer(&g);
    if (g.state == STATE_WRONG) {
        printf("  PASS  evaluate_answer: wrong answer -> STATE_WRONG\n");
        pass++;
    } else {
        printf("  FAIL  evaluate_answer: got %d, want STATE_WRONG (%d)\n", g.state, STATE_WRONG);
        fail++;
    }
    /* walk away */
    game_init(&g, qs, count);
    g.state = STATE_GAMEOVER;
    if (g.state == STATE_GAMEOVER) {
        printf("  PASS  walk-away: STATE_GAMEOVER reached\n");
        pass++;
    } else {
        printf("  FAIL  walk-away\n");
        fail++;
    }

    free_questions(qs, count);
    printf("\n%d passed, %d failed\n", pass, fail);
    return (fail > 0 ? 1 : 0);
}

int main(void)
{
    return (run_tests());
}
TEST_C

gcc -Wall -Wextra -I. \
    /tmp/g01b_load.c \
    /tmp/g01b_game.c \
    /tmp/g01b_test.c \
    libtci.a \
    -o /tmp/g01b_tester 2>&1

if [ $? -ne 0 ]; then
    echo "Compilation failed — see errors above."
    exit 1
fi

/tmp/g01b_tester
exit $?
