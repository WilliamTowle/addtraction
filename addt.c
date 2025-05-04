/*  AddTraction
 * 
 *  Copyright (C) 2004 by Andre Kloss
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 */

#include "stdio.h"
#ifdef __linux
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif

#include <string.h>

#define ADDTRACTION_VERSION "0.001"
#define SIZE_X 6
#define SIZE_Y 6
#define NAN (int)((Uint32)(1<<31))

/* If not set, clear path */
#ifndef BITMAP_PATH
#define BITMAP_PATH "/usr/share/games/addt/", "bmps/"
#endif

/* helper: string concatenation */
static
char *concat(char *a, char *b) {
        char *result;
        int alen, blen;
        if (a == NULL) return b? strdup(b) : NULL;
        if (b == NULL) return a? strdup(a) : NULL;
        alen = strlen(a);
        blen = strlen(b);
        result = (char *)malloc(sizeof(char) * (alen+blen+1));
        if (result == NULL) return NULL; 
        memcpy(result, a, alen);
        memcpy(result + alen, b, blen);
        result[alen+blen] = '\0';
        return result;
}

/* sprite stuff */
static
SDL_Surface *sprite(char **search_path, char *file) {
	SDL_Surface *bmp;
	SDL_Surface *result;
	char *location;
	int i;
	for (i = 0; (search_path[i]); i++) {
		location = concat(search_path[i], file);
		bmp = SDL_LoadBMP(location);
		free(location);
		if (bmp != NULL) break;
	}
	if (bmp == NULL) {
		fprintf(stderr, "Couldn't load %s: %s\n", file,
			SDL_GetError());
		return NULL;			
	}
	SDL_SetColorKey(bmp, (SDL_SRCCOLORKEY|SDL_RLEACCEL), 0);
	result = SDL_DisplayFormat(bmp);
	SDL_FreeSurface(bmp);
	if (result == NULL) {
		fprintf(stderr, "Couldn't convert %s: %s", file, 
			SDL_GetError());
		return NULL;
	}
	return result;
}

static
void show_sprite(SDL_Surface *screen, SDL_Surface *spr, int x, int y) {
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = spr->w;
	rect.h = spr->h;
	SDL_BlitSurface(spr, NULL, screen, &rect);
}

/* number stuff */
static
SDL_Surface **number_init(char **path) {
	int i;
	char file[16];
	SDL_Surface **numbers = (SDL_Surface **)malloc(sizeof(SDL_Surface *)*10);
	for (i=0; i<10; i++) {
		(void)snprintf(file, 15, "%i.bmp", i);
		numbers[i] = sprite(path, file);
	}
	return numbers;
}

static
void exit_number(SDL_Surface **numbers) {
	int i;
	for (i=0; i<10; i++) SDL_FreeSurface(numbers[i]);
	free(numbers);
}

static
void show_number(SDL_Surface *screen, SDL_Surface **numbers, int number, int x, int y) {
	int c, w, xpos;
	int n, d;
	Uint8 digits[20]; /* think big :) */
	
	n = number;
	c = 0; w = 0;
	do {
		d = n % 10;
		digits[c++] = d;
		w += numbers[d]->w + 1;
		n = n / 10;
	} while (n > 0);
	xpos = x - (w / 2);
	for (--c; c > -1; c--) {
		show_sprite(screen, numbers[digits[c]], xpos, 
			y-numbers[digits[c]]->h / 2);
		xpos += numbers[digits[c]]->w + 1;
	}
}

/* field stuff */
typedef struct Field {
	SDL_Surface *red;
	SDL_Surface *green;
	SDL_Surface *black;
	SDL_Surface **numbers;
	int fields[SIZE_X * SIZE_Y];
	int open_fields;
	int score;
	int player;
} Field;

#define FIELD_WIDTH 64
#define FIELD_HEIGHT 64
#define GAP_X 2
#define GAP_Y 2
#define OFFSET_X (400 - (FIELD_WIDTH + GAP_X) * 3)
#define OFFSET_Y (300 - (FIELD_HEIGHT + GAP_Y) * 3)

static
int field2screen_x(int x, int y) { 
	return OFFSET_X + (x * (FIELD_WIDTH + GAP_X)); 
}

static
int field2screen_y(int x, int y) {
	return OFFSET_Y + (y * (FIELD_HEIGHT + GAP_Y));
}

static
int screen2field_x(int x, int y) {
	return (x - OFFSET_X) / (FIELD_WIDTH + GAP_X);
}

static
int screen2field_y(int x, int y) { 
	return (y - OFFSET_Y) / (FIELD_HEIGHT + GAP_Y); 
}

static
Field *field_init(char **path, SDL_Surface **numbers) {
	int i;
	Field *field = (Field *)malloc(sizeof(Field));
	field->red = sprite(path, "red.bmp");
	field->green = sprite(path, "green.bmp");
	field->black = sprite(path, "black.bmp");
	for (i = 0; i < SIZE_X * SIZE_Y; i++) field->fields[i] = NAN;
	field->fields[0] = 1;
	field->fields[SIZE_Y * SIZE_X - 1] = (-1);
	field->numbers = numbers;
	field->open_fields = SIZE_X * SIZE_Y - 2;
	field->score = 0;
	field->player = 1;
	return field;
}

static
void exit_field(Field *field) {
	SDL_FreeSurface(field->red);
	SDL_FreeSurface(field->green);
	SDL_FreeSurface(field->black);	
	free(field);
}

static
void show_field(SDL_Surface *screen, Field *field, int num, int x, int y) {
	SDL_Surface *background;
	int xpos, ypos;
	xpos = field2screen_x(x, y);
	ypos = field2screen_y(x, y);

	if (num == 0 || num == NAN) { background = field->black; }
	else if (num < 0) { background = field->green; }
	else { /* num > 0 */ background = field->red; }
	show_sprite(screen, background, xpos, ypos);
	if (num != NAN)
		show_number(screen, field->numbers, abs(num), 
			xpos + FIELD_WIDTH / 2, ypos + FIELD_HEIGHT / 2);
	SDL_UpdateRect(screen, xpos, ypos, FIELD_WIDTH, FIELD_HEIGHT);
}

static
int get_field(Field *field, int x, int y) {
	if (x < 0 || x >= SIZE_X ||
	    y < 0 || y >= SIZE_Y) return NAN;
	return field->fields[y * SIZE_X + x];
}

static
int get_field_value(Field *field, int x, int y) { 
	int result = get_field(field, x, y);
	return (result == NAN) ? 0 : result;
}

static
int set_field(Field *field, int x, int y) {
	int num;
	if (x < 0 || x >= SIZE_X ||
	    y < 0 || y >= SIZE_Y) return NAN;
	if (get_field(field, x, y) != NAN) return NAN;
	num = 	get_field_value(field, x-1, y-1) +
		get_field_value(field, x  , y-1) +
		get_field_value(field, x+1, y-1) +
		get_field_value(field, x-1, y  ) +
		get_field_value(field, x+1, y  ) +
		get_field_value(field, x-1, y+1) +
		get_field_value(field, x  , y+1) +
		get_field_value(field, x+1, y+1);
	field->fields[y * SIZE_X + x] = num;
	field->open_fields--;
	field->score += num;
	field->player = -1 - field->player;
	return num;
}

static
int turn(SDL_Surface *screen, Field *field, int x, int y) {
	int num;
	if (field->open_fields == 0) return 1;
	num = set_field(field, x, y);
	if (num == NAN) return 0;
	show_field(screen, field, num, x, y);
	show_field(screen, field, field->score, 7, 0);		
	show_field(screen, field, field->player, 7, 5);
	return 0;
}

/* cursor stuff */
typedef struct {
	int x, y;
	SDL_Surface *visual;
} Cursor;

static
Cursor *cursor_init(char **path) {
	Cursor *result = (Cursor *)malloc(sizeof(Cursor));
	result->x = 3;
	result->y = 3;
	result->visual = sprite(path, "cursor.bmp");
	return result;
}

static
void exit_cursor(Cursor *cursor) {
	SDL_FreeSurface(cursor->visual);
	free(cursor);
}

static
void show_cursor(SDL_Surface *screen, Cursor *c) {
	int x = field2screen_x(c->x, c->y) + (FIELD_WIDTH - c->visual->w) / 2;
	int y = field2screen_y(c->x, c->y) + (FIELD_HEIGHT - c->visual->h) / 2;
	show_sprite(screen, c->visual, x, y);
	SDL_UpdateRect(screen, x, y, c->visual->w, c->visual->h);
}

static
void hide_cursor(SDL_Surface *s, Field *field, Cursor *c) {
	show_field(s, field, get_field(field, c->x, c->y), c->x, c->y);
}

static
void move_cursor(SDL_Surface *screen, Field *field, Cursor *cursor, 
	int dx, int dy) {
	hide_cursor(screen, field, cursor);
	cursor->x += dx;
	cursor->y += dy;
	if (cursor->x < 0) cursor->x = 0;
	if (cursor->y < 0) cursor->y = 0;
	if (cursor->x >= SIZE_X) cursor->x = SIZE_X-1;
	if (cursor->y >= SIZE_Y) cursor->y = SIZE_Y-1;
	show_cursor(screen, cursor);
	printf("(%i,%i)\n", cursor->x, cursor->y);
}

/* engine stuff */
static
SDL_Surface *engine_init(int argc, char *argv[]) {
	int videoflags = SDL_HWSURFACE | SDL_ANYFORMAT;
	int width = 800;
	int height = 600;
	int bpp = 16;
	SDL_Surface *screen;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr,"Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	screen = SDL_SetVideoMode(width, height, bpp, videoflags);
	if (!screen) {
		fprintf(stderr,"Couldn't set video mode: %s\n", SDL_GetError());
		exit(2);
	}
	SDL_WM_SetCaption("AddTraction", "AddTraction");
	return screen;
}

static
int handle_event(SDL_Event *event, SDL_Surface *screen, 
	Field *field, Cursor *cursor) {
/* returns 1 if finished, 0 otherwise */
	int x, y, num;
	switch(event->type) {
	case SDL_KEYDOWN:
		switch (event->key.keysym.sym) {
		case SDLK_q:
		case SDLK_ESCAPE:
			return 1;
		case SDLK_KP2:
		case SDLK_DOWN:
			move_cursor(screen, field, cursor, 0, 1);
			return 0;
		case SDLK_KP8:
		case SDLK_UP:
			move_cursor(screen, field, cursor, 0, -1);		
			return 0;
		case SDLK_KP4:
		case SDLK_LEFT:
			move_cursor(screen, field, cursor, -1, 0);
			return 0;
		case SDLK_KP6:
		case SDLK_RIGHT:
			move_cursor(screen, field, cursor, 1, 0);
			return 0;
		case SDLK_SPACE:
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			return turn(screen, field, cursor->x, cursor->y);
		default:
			return 0;
		}
	case SDL_MOUSEBUTTONDOWN:
		if (field->open_fields <= 0) return 1;
		x = screen2field_x(event->button.x, event->button.y);
		y = screen2field_y(event->button.x, event->button.y);
		return turn(screen, field, x, y);
	case SDL_QUIT: 
		return 1;
	default:
		return 0;
	}
}

static
int engine_loop(SDL_Surface *screen, Field *field, Cursor *cursor) {
	int finished = 0;
	SDL_Event event;
	while (!finished) {
		while(SDL_PollEvent(&event)) {
			finished = handle_event(&event, screen, field, cursor);
		}
	}
}

int main(int argc, char *argv[]) {
	int x, y;
	char *path[]		= {BITMAP_PATH, "bmps/", 0};
	SDL_Surface *screen 	= engine_init(argc, argv);
	SDL_Surface **numbers 	= number_init(path);
	SDL_Surface *player	= sprite(path, "player.bmp");
	SDL_Surface *score	= sprite(path, "score.bmp");
	SDL_Surface *title	= sprite(path, "addt.bmp");
	Field *field 		= field_init(path, numbers);
	Cursor *cursor		= cursor_init(path);

	for (x = 0; x < SIZE_X; x++) {
		for (y = 0; y < SIZE_Y; y++) {
			show_field(screen, field, get_field(field, x, y), x, y);
		}
	}
	show_field(screen, field, field->score, 7, 0);
	show_field(screen, field, field->player, 7, 5);
	show_sprite(screen, score, 
		field2screen_x(7, 1) + (FIELD_WIDTH - score->w) / 2,
		field2screen_y(7, 1) + 8);
	show_sprite(screen, player, 
		field2screen_x(7, 5) + (FIELD_WIDTH - player->w) / 2,
		field2screen_y(7, 5) - 8 - player->h);
	show_sprite(screen, title, 120, 95);
	show_cursor(screen, cursor);
	SDL_FreeSurface(score);
	SDL_FreeSurface(player);
	SDL_FreeSurface(title);

	SDL_UpdateRect(screen, 0, 0, 800, 600);
	engine_loop(screen, field, cursor);
	
	exit_cursor(cursor);
	exit_field(field);
	exit_number(numbers);
	SDL_Quit();
}
