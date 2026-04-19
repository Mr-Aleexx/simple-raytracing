#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 900
#define HEIGHT 600

#define COLOR_BLACK 0xFF000000
#define COLOR_WHITE 0xffffffff
#define COLOR_GRAY 0xefefefef
#define COLOR_YELLOW 0xFFFFFF00

#define COLOR_RED 0x00FF0000
#define COLOR_GREEN 0x0000FF00
#define COLOR_BLUE 0x000000FF

#define RAYS_NUMBER 1000
#define CIRCLE_NUMBER 10

//TODO : implement multiple shadow_circles
#define SHADOW_CIRCLE_NUMBER 1

typedef struct {
  double x;
  double y;
  double r;
} Circle;

typedef struct {
  double x_start, y_start;
  double x_end, y_end;
  double angle;
} Ray;

void FillCircle(SDL_Surface *surface, Circle *circle) {
  double radius_squared =circle->r * circle->r;

  for (double x = circle->x - circle->r; x <= circle->x + circle->r; x++) {
    for (double y = circle->y - circle->r; y <= circle->y + circle->r; y++) {

      double distance_squared_center = pow(x - circle->x, 2) + pow(y - circle->y, 2);

      if (distance_squared_center < radius_squared) {
        SDL_Rect pixel = (SDL_Rect){x, y, 1, 1};
        SDL_FillRect(surface, &pixel, COLOR_WHITE);
      }
    }
  }
}

void DrawLine(SDL_Surface *surface, int x0, int y0, int x1, int y1,
              Uint32 color) {
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  while (1) {
    if (x0 >= 0 && x0 < surface->w && y0 >= 0 && y0 < surface->h) {
      Uint32 *pixels = (Uint32 *)surface->pixels;
      pixels[(y0 * surface->w) + x0] = color;
    }
    if (x0 == x1 && y0 == y1)
      break;
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void FillRays(SDL_Surface *surface, Ray rays[RAYS_NUMBER]) {
  for (int i = 0; i < RAYS_NUMBER; i++) {
    int x0 = (int)rays[i].x_start;
    int y0 = (int)rays[i].y_start;
    int x1 = (int)rays[i].x_end;
    int y1 = (int)rays[i].y_end;
    DrawLine(surface, x0, y0, x1, y1, COLOR_BLUE);
  }
}

void generate_rays(Circle *circle, Ray rays[RAYS_NUMBER]) {
  for (int i = 0; i < RAYS_NUMBER; i++) {
    double ang = ((double)i / RAYS_NUMBER) * 2 * M_PI;
    rays[i].x_start = circle->x;
    rays[i].y_start = circle->y;
    rays[i].x_end = (int)(rays[i].x_start + cos(ang) * 2000);
    rays[i].y_end = (int)(rays[i].y_start + sin(ang) * 2000);
    rays[i].angle = ang;
  }
}

void check_collision(Circle *circle, Ray rays[], int ray_count) {
  for (int i = 0; i < ray_count; i++) {
    double dx = rays[i].x_end - rays[i].x_start;
    double dy = rays[i].y_end - rays[i].y_start;
    double fx = rays[i].x_start - circle->x;
    double fy = rays[i].y_start - circle->y;

    double a = dx * dx + dy * dy;
    double b = 2 * (fx * dx + fy * dy);
    double c = fx * fx + fy * fy - circle->r * circle->r;

    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
      continue;

    discriminant = sqrt(discriminant);
    double t1 = (-b - discriminant) / (2 * a);
    double t2 = (-b + discriminant) / (2 * a);

    if (t1 >= 0 && t1 <= 1) {
      rays[i].x_end = rays[i].x_start + t1 * dx;
      rays[i].y_end = rays[i].y_start + t1 * dy;
    } else if (t2 >= 0 && t2 <= 1) {
      rays[i].x_end = rays[i].x_start + t2 * dx;
      rays[i].y_end = rays[i].y_start + t2 * dy;
    }
  }
}


bool in_circle(Circle *circle, int xMotion, int yMotion) {
  int dx = xMotion - circle->x;
  int dy = yMotion - circle->y;
  return (dx * dx) + (dy * dy) <= circle->r * circle->r;
}

void rand_circle(Circle *c) {
    c->x = 10 + (rand() % (WIDTH - 20));
    c->y = 10 + (rand() % (HEIGHT - 20));
    c->r = 10 + (rand() % (30));
}

void print_circle(Circle* c) {
  printf("x = %f\ny = %f\nr = %f\n", c->x, c->y, c->r);
}


int main(void) {
    srand(time(NULL));

    Circle *circles = malloc(CIRCLE_NUMBER * sizeof(Circle));
    Circle *shadow_circles = malloc(SHADOW_CIRCLE_NUMBER * sizeof(Circle));
    Ray *rays = malloc(RAYS_NUMBER * sizeof(Ray));

    for (int i = 0 ; i < CIRCLE_NUMBER ; i++) {
        rand_circle(&circles[i]);
    }
    for (int i = 0 ; i < SHADOW_CIRCLE_NUMBER ; i++) {
        rand_circle(&shadow_circles[i]);
        print_circle(&shadow_circles[i]);
    }



    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window =
      SDL_CreateWindow("Raytracing", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    SDL_Rect erase_rect = {0, 0, WIDTH, HEIGHT};

    int running = 1;
    int drag = 0;
    SDL_Event e;


    int circle_index;
    int shadow_circle_index;

    while (running) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = 0;

        int xMotion = e.motion.x;
        int yMotion = e.motion.y;

        if (SDL_GetMouseState(&xMotion,&yMotion) & SDL_BUTTON_LMASK) drag = 1;
        else {
            drag = 0;
            circle_index = -1;
            shadow_circle_index = -1;
        }


        if (drag && circle_index == -1) {
            for (int i = 0 ; i < CIRCLE_NUMBER ; i++) {
                if (in_circle(&circles[i], xMotion, yMotion)) {
                    circle_index = i;
                    break;
                }  else circle_index = -1;
            }
        }

        if (drag && shadow_circle_index == -1 ) {
            for (int i = 0 ; i < SHADOW_CIRCLE_NUMBER ; i++) {
              if (in_circle(&shadow_circles[i] , xMotion, yMotion)) {
                  shadow_circle_index = i;
              } else shadow_circle_index = -1;
            }
        }

      if(e.type == SDL_MOUSEMOTION && drag) {

        xMotion = e.motion.x;
        yMotion = e.motion.y;

        if(circle_index != -1) {
            circles[circle_index].x = xMotion;
            circles[circle_index].y = yMotion;
        }

        if (shadow_circle_index != -1) {
            shadow_circles[shadow_circle_index].x = xMotion;
            shadow_circles[shadow_circle_index].y = yMotion;
        }
      }
        //printf("\nSELECTED CIRCLE : %d\n", circle_index);
        //printf("SELECTED SHADOWCIRCLE : %d\n\n", shadow_circle_index);

    }
      SDL_FillRect(surface, &erase_rect, COLOR_BLACK);

      for (int i = 0 ; i < SHADOW_CIRCLE_NUMBER ; i++) {
        generate_rays(&shadow_circles[i], rays);
      }
      for (int i = 0 ; i < CIRCLE_NUMBER ; i++) {
        FillCircle(surface,&circles[i]);
      }


      for (int i = 0 ; i < CIRCLE_NUMBER ; i++) {
        check_collision(&circles[i], rays, RAYS_NUMBER);
      }

      FillRays(surface, rays);

      for (int i = 0 ; i < SHADOW_CIRCLE_NUMBER ; i++) {
        FillCircle(surface, &shadow_circles[i]);
      }
      SDL_UpdateWindowSurface(window);
      SDL_Delay(1);
    }

    free(shadow_circles);
    free(circles);
    free(rays);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
