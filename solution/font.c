#include "game.h"

int font_load(game_t *g, const char *path, int ptsize)
{
    g->font = TTF_OpenFont(path, ptsize);
    if (!g->font) {
        SDL_Log("font_load: %s", TTF_GetError());
        return (-1);
    }
    return (0);
}

void    font_free(game_t *g)
{
    if (g->font) {
        TTF_CloseFont(g->font);
        g->font = NULL;
    }
}

void    draw_string(game_t *g, int x, int y, const char *s)
{
    SDL_Color    white;
    SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Rect     dst;
    int          w;
    int          h;

    if (!s || !*s)
        return;
    white.r = 255;
    white.g = 255;
    white.b = 255;
    white.a = 255;
    surf = TTF_RenderUTF8_Solid(g->font, s, white);
    if (!surf)
        return;
    tex = SDL_CreateTextureFromSurface(g->ren, surf);
    SDL_FreeSurface(surf);
    if (!tex)
        return;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    dst.x = x;
    dst.y = y;
    dst.w = w;
    dst.h = h;
    SDL_RenderCopy(g->ren, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}
