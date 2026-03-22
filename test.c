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
#define RAYS_NUMBER 200

struct Circle {
  double x;
  double y;
  double r;
};

struct Ray {
  double x_start, y_start;
  double x_end, y_end;
  double angle;
};

void FillCircle(SDL_Surface *surface, struct Circle circle) {
  double radius_squared = pow(circle.r, 2);
  for (double x = circle.x - circle.r; x <= circle.x + circle.r; x++) {
    for (double y = circle.y - circle.r; y <= circle.y + circle.r; y++) {
      double distance_squared_center =
          pow(x - circle.x, 2) + pow(y - circle.y, 2);
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

void FillRays(SDL_Surface *surface, struct Ray rays[RAYS_NUMBER]) {
  for (int i = 0; i < RAYS_NUMBER; i++) {
    int x0 = (int)rays[i].x_start;
    int y0 = (int)rays[i].y_start;
    int x1 = (int)rays[i].x_end;
    int y1 = (int)rays[i].y_end;
    DrawLine(surface, x0, y0, x1, y1, COLOR_YELLOW);
  }
}

void generate_rays(struct Circle circle, struct Ray rays[RAYS_NUMBER]) {
  for (int i = 0; i < RAYS_NUMBER; i++) {
    double ang = ((double)i / RAYS_NUMBER) * 2 * M_PI;
    rays[i].x_start = circle.x;
    rays[i].y_start = circle.y;
    rays[i].x_end = (int)(rays[i].x_start + cos(ang) * 2000);
    rays[i].y_end = (int)(rays[i].y_start + sin(ang) * 2000);
    rays[i].angle = ang;
  }
}

void check_collision(struct Circle circle, struct Ray rays[], int ray_count) {
  for (int i = 0; i < ray_count; i++) {
    double dx = rays[i].x_end - rays[i].x_start;
    double dy = rays[i].y_end - rays[i].y_start;
    double fx = rays[i].x_start - circle.x;
    double fy = rays[i].y_start - circle.y;

    double a = dx * dx + dy * dy;
    double b = 2 * (fx * dx + fy * dy);
    double c = fx * fx + fy * fy - circle.r * circle.r;

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

bool in_circle(struct Circle circle, int xMotion, int yMotion) {
  int dx = xMotion - circle.x;
  int dy = yMotion - circle.y;
  return (dx * dx) + (dy * dy) <= circle.r * circle.r;
}

struct Circle rand_circle() {
  struct Circle c;
  c.x = 10 + (rand() % (WIDTH - 20));
  c.y = 10 + (rand() % (HEIGHT - 20));
  c.r = 10 + (rand() % (300));
  return c;
}



int main(void) {
  srand(time(NULL));

  struct Circle c = rand_circle();

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window =
      SDL_CreateWindow("Raytracing", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

  SDL_Surface *surface = SDL_GetWindowSurface(window);

  SDL_Rect erase_rect = {0, 0, WIDTH, HEIGHT};
  struct Circle circle = {200, 200, 20};
  struct Circle shadow_circle2 = rand_circle();
  struct Circle shadow_circle = rand_circle();
  struct Ray rays[RAYS_NUMBER];

  int running = 1;
  int drag = 0;
  SDL_Event e;

  while (running) {
    while (SDL_PollEvent(&e)) {
      // SDL_GetMouseState(&xMotion, &yMotion) & SDL_BUTTON_LMASK
      if (e.type == SDL_QUIT)
        running = 0;
      // if (e.type == SDL_MOUSEMOTION && e.motion.state != 0) {
      // 	circle.x = e.motion.x;
      // 	circle.y = e.motion.y;
      // }

      if (SDL_GetMouseState(&e.button.x, &e.button.y) & SDL_BUTTON_LMASK) {
        int x = e.button.x;
        int y = e.button.y;

        if (in_circle(circle, x, y)) {
          drag = 1;
        } else if (in_circle(shadow_circle, x, y)) {
          drag = 2;
        }

        else if (in_circle(shadow_circle2, x, y)) {
          drag = 3;
        }
      }
      if (!(SDL_GetMouseState(&e.button.x, &e.button.y) & SDL_BUTTON_LMASK)) {
        drag = 0;
      }

      if (e.type == SDL_MOUSEMOTION && drag != 0) {
        int xMotion = e.motion.x;
        int yMotion = e.motion.y;

        if (drag == 1) {
          circle.x = xMotion;
          circle.y = yMotion;
        } else if (drag == 2) {
          shadow_circle.x = xMotion;
          shadow_circle.y = yMotion;
        } else if (drag == 3) {
          shadow_circle2.x = xMotion;
          shadow_circle2.y = yMotion;
        }
      }
    }

    SDL_FillRect(surface, &erase_rect, COLOR_BLACK);
    FillCircle(surface, circle);
    FillCircle(surface, shadow_circle);
    FillCircle(surface, shadow_circle2);
    generate_rays(circle, rays);
    check_collision(shadow_circle, rays, RAYS_NUMBER);
    check_collision(shadow_circle2, rays, RAYS_NUMBER);
    FillRays(surface, rays);
    SDL_UpdateWindowSurface(window);
    SDL_Delay(1);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
