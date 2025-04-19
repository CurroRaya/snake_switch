#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <switch.h>

// Maximizamos el tamaño para una pantalla estándar de Switch
#define GRID_WIDTH  75   // Ampliado pero ajustado al ancho visible
#define GRID_HEIGHT 35   // Ampliado pero dejando espacio para mensajes
#define OFFSET_X    2    // espacio para el marcador
#define OFFSET_Y    4    // espacio para el título y puntuación - aumentado de 3 a 4
#define BORDER_CHAR '#'
#define SNAKE_BODY  'o'  // Cambiado a minúscula para el cuerpo
#define FOOD_CHAR   '*'
#define TICKS_PER_MS 19200ULL

// Caracteres para la cabeza según la dirección
#define SNAKE_HEAD_UP    '^'
#define SNAKE_HEAD_DOWN  'v'
#define SNAKE_HEAD_LEFT  '<'
#define SNAKE_HEAD_RIGHT '>'

typedef struct {
    int x, y;
} Point;

Point snake[1024];  // Aumentado el tamaño máximo de la serpiente
int snakeLength = 3;
int dx = 1, dy = 0;
Point food;
int score = 0;

// Dibuja los bordes del recuadro de juego
void drawBorders() {
    // Dibujamos el borde superior
    for (int x = 0; x <= GRID_WIDTH + 1; x++) {
        printf("\x1b[%d;%dH%c", OFFSET_Y, OFFSET_X + x, BORDER_CHAR);
    }
    
    // Dibujamos el borde inferior
    for (int x = 0; x <= GRID_WIDTH + 1; x++) {
        printf("\x1b[%d;%dH%c", OFFSET_Y + GRID_HEIGHT + 1, OFFSET_X + x, BORDER_CHAR);
    }
    
    // Dibujamos los bordes izquierdo y derecho
    for (int y = 0; y <= GRID_HEIGHT + 1; y++) {
        printf("\x1b[%d;%dH%c", OFFSET_Y + y, OFFSET_X, BORDER_CHAR);
        printf("\x1b[%d;%dH%c", OFFSET_Y + y, OFFSET_X + GRID_WIDTH + 1, BORDER_CHAR);
    }
}

void placeFood() {
    while (1) {
        int valid = 1;
        food.x = rand() % GRID_WIDTH;
        food.y = rand() % GRID_HEIGHT;
        
        for (int i = 0; i < snakeLength; ++i) {
            if (snake[i].x == food.x && snake[i].y == food.y) {
                valid = 0;
                break;
            }
        }
        
        if (valid) break;
    }
}

void initGame() {
    snake[0] = (Point){GRID_WIDTH / 2, GRID_HEIGHT / 2};
    snake[1] = (Point){GRID_WIDTH / 2 - 1, GRID_HEIGHT / 2};
    snake[2] = (Point){GRID_WIDTH / 2 - 2, GRID_HEIGHT / 2};
    snakeLength = 3;
    dx = 1; dy = 0;
    score = 0;
    placeFood();
}

void updateDirection(u64 kDown) {
    if ((kDown & HidNpadButton_Up) && dy == 0)    { dx = 0; dy = -1; }
    if ((kDown & HidNpadButton_Down) && dy == 0)  { dx = 0; dy = 1; }
    if ((kDown & HidNpadButton_Left) && dx == 0)  { dx = -1; dy = 0; }
    if ((kDown & HidNpadButton_Right) && dx == 0) { dx = 1; dy = 0; }
    
    // También soporte para controles con stick analógico
    if ((kDown & HidNpadButton_StickLUp) && dy == 0)    { dx = 0; dy = -1; }
    if ((kDown & HidNpadButton_StickLDown) && dy == 0)  { dx = 0; dy = 1; }
    if ((kDown & HidNpadButton_StickLLeft) && dx == 0)  { dx = -1; dy = 0; }
    if ((kDown & HidNpadButton_StickLRight) && dx == 0) { dx = 1; dy = 0; }
}

char getSnakeHeadChar() {
    if (dx == 1) return SNAKE_HEAD_RIGHT;
    if (dx == -1) return SNAKE_HEAD_LEFT;
    if (dy == 1) return SNAKE_HEAD_DOWN;
    if (dy == -1) return SNAKE_HEAD_UP;
    return SNAKE_HEAD_RIGHT; // Por defecto
}

void draw() {
    consoleClear();
    
    // Dibujar título y controles - mantenlos en las primeras líneas
    printf("\x1b[1;1HSNAKE para Nintendo Switch by Curro Raya");
    printf("\x1b[2;1HPuntuacion: %d  [+ para salir]", score);
    
    // Espacio extra entre el texto y el borde - puede agregar una línea vacía
    // printf("\x1b[3;1H ");  // Esto dejaría la línea 3 vacía
    
    // Dibujar bordes
    drawBorders();
    
    // Dibujar comida
    printf("\x1b[%d;%dH%c", OFFSET_Y + food.y + 1, OFFSET_X + food.x + 1, FOOD_CHAR);
    
    // Dibujar serpiente
    // Primero la cabeza
    printf("\x1b[%d;%dH%c", OFFSET_Y + snake[0].y + 1, OFFSET_X + snake[0].x + 1, getSnakeHeadChar());
    
    // Luego el cuerpo
    for (int i = 1; i < snakeLength; ++i) {
        printf("\x1b[%d;%dH%c", OFFSET_Y + snake[i].y + 1, OFFSET_X + snake[i].x + 1, SNAKE_BODY);
    }
}

bool moveSnake() {
    Point next = { snake[0].x + dx, snake[0].y + dy };
    
    // Colisión con bordes
    if (next.x < 0 || next.x >= GRID_WIDTH || next.y < 0 || next.y >= GRID_HEIGHT)
        return false;
    
    // Colisión con sí mismo
    for (int i = 0; i < snakeLength; ++i) {
        if (snake[i].x == next.x && snake[i].y == next.y)
            return false;
    }
    
    // Comer comida
    bool ateFood = (next.x == food.x && next.y == food.y);
    
    // Si come, primero añadimos un segmento al final
    if (ateFood) {
        // Duplicamos la posición del último segmento (temporalmente)
        snake[snakeLength] = snake[snakeLength - 1];
        snakeLength++;
        score++;
    }
    
    // Mover cuerpo
    for (int i = snakeLength - 1; i > 0; --i) {
        snake[i] = snake[i - 1];
    }
    snake[0] = next;
    
    // Si comió, generamos nueva comida
    if (ateFood) {
        placeFood();
    }
    
    return true;
}

int main(int argc, char **argv) {
    consoleInit(NULL);
    srand(time(NULL));
    
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    
    bool gameRunning = true;
    bool shouldExit = false;
    
    while (!shouldExit) {
        initGame();
        gameRunning = true;
        
        u64 lastTick = armGetSystemTick();
        const u64 interval = TICKS_PER_MS * 150;  // Velocidad del juego
        
        while (appletMainLoop() && gameRunning) {
            padUpdate(&pad);
            u64 kDown = padGetButtonsDown(&pad);
            
            if (kDown & HidNpadButton_Plus) {
                shouldExit = true;
                break;  // Salir por completo con el botón +
            }
            
            updateDirection(kDown);
            
            u64 now = armGetSystemTick();
            if (now - lastTick >= interval) {
                lastTick = now;
                
                if (!moveSnake()) {
                    gameRunning = false;  // Game over
                }
            }
            
            draw();
            consoleUpdate(NULL);
        }
        
        if (!gameRunning && !shouldExit) {
            // Pantalla de Game Over
            consoleClear();
            printf("\x1b[%d;%dH#############################", GRID_HEIGHT/2 - 2, GRID_WIDTH/2 - 15);
            printf("\x1b[%d;%dH#                           #", GRID_HEIGHT/2 - 1, GRID_WIDTH/2 - 15);
            printf("\x1b[%d;%dH#        GAME OVER!         #", GRID_HEIGHT/2,     GRID_WIDTH/2 - 15);
            printf("\x1b[%d;%dH#    Puntuacion final: %3d  #", GRID_HEIGHT/2 + 1, GRID_WIDTH/2 - 15, score);
            printf("\x1b[%d;%dH#                           #", GRID_HEIGHT/2 + 2, GRID_WIDTH/2 - 15);
            printf("\x1b[%d;%dH#############################", GRID_HEIGHT/2 + 3, GRID_WIDTH/2 - 15);
            printf("\x1b[%d;%dH[A] Reiniciar - [+] Salir", GRID_HEIGHT/2 + 5, GRID_WIDTH/2 - 12);
            consoleUpdate(NULL);
            
            // Esperar a que el jugador presione A para reiniciar o + para salir
            while (appletMainLoop()) {
                padUpdate(&pad);
                u64 kDown = padGetButtonsDown(&pad);
                
                if (kDown & HidNpadButton_A) {
                    break;  // Reiniciar el juego
                }
                if (kDown & HidNpadButton_Plus) {
                    shouldExit = true;
                    break;  // Salir del juego
                }
            }
        }
    }
    
    consoleExit(NULL);
    return 0;
}