#include "game.h"

static char *next_field(char **p)
{
    char    *start;
    char    *dst;

    if (!*p || !**p)
        return (NULL);
    start = *p;
    dst = start;
    while (**p) {
        if (**p == '|') {
            if (*(*p + 1) == '|') {
                *dst++ = '|';
                *p += 2;
            } else {
                *dst = '\0';
                (*p)++;
                return (start);
            }
        } else {
            *dst++ = **p;
            (*p)++;
        }
    }
    *dst = '\0';
    return (start);
}

static question_t   *parse_line(char *line)
{
    question_t  *q;
    char        *p;
    char        *tok;
    int          i;

    q = tci_calloc(1, sizeof(question_t));
    if (!q)
        return (NULL);
    p = line;
    tok = next_field(&p);
    if (!tok) { free(q); return (NULL); }
    q->text = tci_strdup(tok);
    for (i = 0; i < 4; i++) {
        tok = next_field(&p);
        if (!tok) { free(q); return (NULL); }
        q->opts[i] = tci_strdup(tok);
    }
    tok = next_field(&p);
    if (!tok) { free(q); return (NULL); }
    q->answer = tci_atoi(tok);
    tok = next_field(&p);
    q->hint = tok ? tci_strdup(tok) : NULL;
    return (q);
}

question_t  **load_questions(const char *path, int *count)
{
    int          fd;
    char        *line;
    char        *nl;
    question_t **questions;
    int          cap;
    int          n;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        tci_printf("load_questions: cannot open %s\n", path);
        return (NULL);
    }
    cap = 32;
    n = 0;
    questions = tci_calloc(cap, sizeof(question_t *));
    if (!questions) { close(fd); return (NULL); }
    while ((line = tci_getline(fd)) != NULL) {
        nl = tci_strchr(line, '\n');
        if (nl)
            *nl = '\0';
        if (line[0] == '\0' || line[0] == '#') {
            free(line);
            continue;
        }
        if (n >= cap) {
            question_t **tmp;

            cap *= 2;
            tmp = realloc(questions, cap * sizeof(question_t *));
            if (!tmp) { free(line); break; }
            questions = tmp;
        }
        questions[n] = parse_line(line);
        free(line);
        if (questions[n])
            n++;
    }
    close(fd);
    *count = n;
    return (questions);
}

void    free_questions(question_t **questions, int count)
{
    int i;
    int j;

    if (!questions)
        return;
    for (i = 0; i < count; i++) {
        if (!questions[i])
            continue;
        free(questions[i]->text);
        for (j = 0; j < 4; j++)
            free(questions[i]->opts[j]);
        free(questions[i]->hint);
        free(questions[i]);
    }
    free(questions);
}
