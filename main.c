#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define FPS 60
#define TARGET_NS 1e9 / FPS

#define WIDTH 160
#define HEIGHT 44

#define CUBE_WIDTH 0.25

#define DISTANCE_FROM_CAM 1
#define BACKGROUND_CHAR '.'

char buf[WIDTH * HEIGHT];
float zbuf[WIDTH * HEIGHT];

float rX = 0;
float rY = 0;
float rZ = 0;

static inline float calculateX(float a, float b, float c) {
    register double sin_rX = sin(rX);
    register double cos_rX = cos(rX);
    register double sin_rY = sin(rY);
    register double sin_rZ = sin(rZ);
    register double cos_rZ = cos(rZ);
    return a * cos(rY) * cos_rZ +
           b * (sin_rX * sin_rY * cos_rZ + sin_rZ * cos_rX) +
           c * (sin_rX * sin_rZ - sin_rY * cos_rX * cos_rZ);
}

static inline float calculateY(float a, float b, float c) {
    register double sin_rX = sin(rX);
    register double cos_rX = cos(rX);
    register double sin_rY = sin(rY);
    register double sin_rZ = sin(rZ);
    register double cos_rZ = cos(rZ);
    return a * sin_rZ * cos(rY) +
           b * (sin_rX * sin_rY * sin_rZ - cos_rX * cos_rZ) -
           c * (sin_rX * cos_rZ + sin_rY * sin_rZ * cos_rX);
}

static inline float calculateZ(float a, float b, float c) {
    register double cos_rY = cos(rY);
    return a * sin(rY) - b * sin(rX) * cos_rY + c * cos(rX) * cos_rY;
}

static inline float screenX(float x) { return (x + 1) / 2 * WIDTH; }
static inline float screenY(float y) { return (1 - (y + 1) / 2) * HEIGHT; }

static inline void calculateForSurface(float cubeX, float cubeY, float cubeZ,
                                       char ch) {
    float x = calculateX(cubeX, cubeY, cubeZ);
    float y = calculateY(cubeX, cubeY, cubeZ);
    float z = calculateZ(cubeX, cubeY, cubeZ) + DISTANCE_FROM_CAM;

    float ooz = 1 / z;
    int xp = screenX(x * ooz);
    int yp = screenY(y * ooz * 1.75);

    int idx = xp + yp * WIDTH;
    if (idx < 0 || idx >= WIDTH * HEIGHT || ooz <= zbuf[idx]) {
        return;
    }

    zbuf[idx] = ooz;
    buf[idx] = ch;
}

void handle_interrupt(int signum) {
    fputs("\e[?1049l\x1b[?7h\x1b[?25h", stdout);
    exit(0);
}

int main(void) {
    signal(SIGINT, handle_interrupt);
    fputs("\x1b[2J\e[?1049h\x1b[?7l\x1b[?25l", stdout);
    fflush(stdout);

    struct timespec last;
    struct timespec start;
    struct timespec now;
    char output_buf[3 + (WIDTH + 1) * (HEIGHT)];
    clock_gettime(CLOCK_MONOTONIC, &last);

    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &start);

        memset(buf, BACKGROUND_CHAR, WIDTH * HEIGHT * sizeof(*buf));
        memset(zbuf, 0, WIDTH * HEIGHT * sizeof(*zbuf));

        for (float cubeX = -CUBE_WIDTH; cubeX < CUBE_WIDTH; cubeX += 0.005) {
            for (float cubeY = -CUBE_WIDTH; cubeY < CUBE_WIDTH;
                 cubeY += 0.005) {
                calculateForSurface(cubeX, cubeY, -CUBE_WIDTH, '@');
                calculateForSurface(cubeX, -CUBE_WIDTH, cubeY, ';');
                calculateForSurface(-CUBE_WIDTH, cubeX, cubeY, '$');
                calculateForSurface(CUBE_WIDTH, cubeX, cubeY, '~');
                calculateForSurface(cubeX, CUBE_WIDTH, cubeY, '+');
                calculateForSurface(cubeX, cubeY, CUBE_WIDTH, '#');
            }
        }

        size_t offset = 0;

        output_buf[offset++] = '\x1b';
        output_buf[offset++] = '[';
        output_buf[offset++] = 'H';
        for (size_t i = 0; i < WIDTH * HEIGHT; ++i) {
            if (i % WIDTH == 0) output_buf[offset++] = '\n';
            output_buf[offset++] = buf[i];
        }
        write(STDOUT_FILENO, &output_buf, sizeof(output_buf));
        fflush(stdout);

        clock_gettime(CLOCK_MONOTONIC, &now);
        double dt =
            (now.tv_sec - last.tv_sec) + (now.tv_nsec - last.tv_nsec) / 1e9;
        double elapsed =
            (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;
        if (dt > 0.05) dt = 0.05;
        last = now;

        rX += -0.25 * M_PI * dt;
        rY += 0.25 * M_PI * dt;
        rZ += 0.01 * M_PI * dt;

        if (elapsed < 1 / FPS) {
            struct timespec rem = {
                .tv_nsec = (long) (1e9 * (1 / FPS - elapsed)),
            };
            nanosleep(&rem, NULL);
        }
    }
    return 0;
}
