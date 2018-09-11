/*
 * You can add more of your own developed *.h files here.
 * However, you are not allowed to include driverlib, grlib, stdio or any other library in this file.
 */

#include <LED_HAL.h>
#include <Buttons_HAL.h>
#include <Timer_HAL.h>
#include <Display_HAL.h>

#define OPENING_WAIT 3000 // 3 seconds or 3000 ms
#define MOVE 50 //50 ms
#define CRASH 1000
#define UP_THRESHOLD  0x3000
#define DOWN_THRESHOLD 0x1000
#define LEFT_THRESHOLD  0x3000
#define RIGHT_THRESHOLD 0x1000

void menuCycling();
void moveCar();
void returnToMenu();
void GameOver();
void pause();
void updateScores();
void addObstacles();
void initGamestate();
void checkCollision();
unsigned randLanePicker();
void addTrees();
void newGame();
void carCrashContinue();

typedef struct{
    unsigned x;
    unsigned y;
    int lives;
    int score;
} car;

typedef struct{
    unsigned lane;
    unsigned y;
} tree;

typedef struct{
    unsigned lane;
    unsigned y
} gas;

typedef struct{
    char score1;
    char score2;
    char score3;
} scores;

typedef struct{
    car c;
    tree t1;
    tree t2;
    tree t3;
    gas gascan;
    scores scores;
    int treeCounter;
    int numTrees;
    bool t1active, t2active, t3active, gactive;
} gamestate;

//global variables
static enum states {INCEPTION, OPENING, MENU, GAME, INSTRUCTIONS, SCORES, PAUSE, GAMEOVER} state = INCEPTION;
static gamestate GS;

void DrawOpeningScreen() {
    LCDClearDisplay(MY_BLACK);
    PrintString("Zoom Zoom", 1, 3);
    PrintString("by", 3, 7);
    PrintString("Sean Jocher", 5, 2);
}

void DrawMenuScreen() {
    LCDClearDisplay(MY_BLACK);
    PrintString("GAME MENU", 0, 3);
    PrintString("Play Game", 2, 3);
    PrintString("How to play", 3, 3);
    PrintString("Score History", 4, 3);
    drawArrow(2);
}

void drawInstructions() {
    LCDClearDisplay(MY_BLACK);
    PrintString("INSTRUCTIONS", 0, 1);
    PrintString("Enjoy Yourself", 3, 1);
    PrintString("BTN1-MENU", 7, 6);
}

void drawScores() {
    LCDClearDisplay(MY_BLACK);
    PrintString("SCORES", 0, 4);
    PrintString("Score #1:", 2, 0);
    PrintString("Score #2:", 3, 0);
    PrintString("Score #3:", 4, 0);
    LCDDrawChar(2,  15, GS.scores.score1);
    LCDDrawChar(3,  15, GS.scores.score2);
    LCDDrawChar(4,  15, GS.scores.score3);
    PrintString("BTN1-MENU", 7, 6);
}

void drawGame() {
    LCDClearDisplay(MY_BLACK);
    drawRoad();
    PrintString("SCORE", 0, 7);
    PrintString("LIVES", 1, 7);
    PrintString("HIGH", 3, 7);
    PrintString("SCORE", 4, 7);
    drawCar(GS.c.x);
}

void GameOver() {
    LCDClearDisplay(MY_BLACK);
    PrintString("GAME OVER", 0, 2);
    PrintString("Score: ", 2, 0);
    LCDDrawChar(2,  15, GS.c.score);
}

void pause() {
    LCDClearDisplay(MY_BLACK);
    PrintString("GAME PAUSED", 0, 2);
    PrintString("Continue", 2, 3);
    PrintString("Menu", 3, 3);
    drawArrow(2);
}

void ScreensFSM() {
    static OneShotSWTimer_t OST, GT;
    InitOneShotSWTimer(&OST, TIMER32_1_BASE, OPENING_WAIT);
    InitOneShotSWTimer(&GT, TIMER32_0_BASE, MOVE);
    // Set the default outputs
    bool drawOpening = false;
    bool drawMenu = false;
    switch (state) {
    case INCEPTION:
        StartOneShotSWTimer(&OST);
        // State transition
        state = OPENING;
        // The output(s) that are affected in this transition
        drawOpening = true;
        break;
    case OPENING:
        if (OneShotSWTimerExpired(&OST)) {
            // State transition
            state = MENU;
            newGame();
            // The output(s) that are affected in this transition
            drawMenu = true;
        }
        break;
    case MENU:
        menuCycling();
        break;
    case INSTRUCTIONS:
        returnToMenu();
        break;
    case SCORES:
        returnToMenu();
        break;
    case GAME:
        LCDDrawChar(0,  15, (char)GS.c.score);
        LCDDrawChar(1,  15, (char)GS.c.lives);
        LCDDrawChar(3,  15, GS.scores.score1);
        checkCollision();
        if (OneShotSWTimerExpired(&GT)) {
            addTrees();
            moveCar();
            addObstacles();
            StartOneShotSWTimer(&GT);
            GS.treeCounter++;
        }
        pauseGame();
        if(GS.c.lives == '0') {
            state = GAMEOVER;
            updateScores();
            GameOver();
            newGame();
            StartOneShotSWTimer(&OST);
        }
        break;
    case GAMEOVER:
        if (OneShotSWTimerExpired(&OST)) {
            state = OPENING;
        }
        break;
    case PAUSE:
        pauseCycling();
        break;
    } // End of switch-case
    // Implement actions based on the outputs of the FSM
    if (drawOpening)
            DrawOpeningScreen();
    if (drawMenu)
        DrawMenuScreen();
}


void menuSelect(unsigned y) {
    if(y == 2) {
        state = GAME;
        drawGame();
    } else if(y == 3) {
        state = INSTRUCTIONS;
        drawInstructions();
    } else if(y == 4) {
        state = SCORES;
        drawScores();
    }
}

void pauseSelect(unsigned y) {
    if(y == 2) {
        drawGame();
        state = GAME;
    } else if(y == 3) {
        state = OPENING;
    }
}

void pauseCycling() {
    unsigned vx, vy;
    getSampleJoyStick(&vx, &vy);
    static unsigned yval = 2, oldVal = 2;
    static bool move = false, changed = false;
    if(move) {
        clearArrow(oldVal);
        drawArrow(yval);
        oldVal = yval;
        move = false;
    }
    if(vy > UP_THRESHOLD) {
        if(!changed) {
            if(yval == 2) {
                yval = 3;
            } else {
                yval = 2;
            }
            changed = true;
        }
    } else if(vy < DOWN_THRESHOLD) {
        if(!changed) {
            if(yval == 3) {
                yval = 2;
            } else {
                yval = 3;
            }
            changed = true;
        }
    }
    if((((vy < UP_THRESHOLD) && (vy > DOWN_THRESHOLD)) && (changed))) {
        move = true;
        changed = false;
    }
    if(Booster_Joystick_Pushed()) {
        pauseSelect(yval);
        yval = 2;
        oldVal = 2;
    }

}

void menuCycling() {
    unsigned vx, vy;
    getSampleJoyStick(&vx, &vy);
    static unsigned yval = 2, oldVal = 2;
    static bool move = false, changed = false;
    if(move) {
        clearArrow(oldVal);
        drawArrow(yval);
        oldVal = yval;
        move = false;
    }
    if(vy > UP_THRESHOLD) {
        if(!changed) {
            if(yval == 2) {
                yval = 4;
            } else {
                yval--;
            }
            changed = true;
        }
    } else if(vy < DOWN_THRESHOLD) {
        if(!changed) {
            if(yval == 4) {
                yval = 2;
            } else {
                yval++;
            }
            changed = true;
        }
    }
    if((((vy < UP_THRESHOLD) && (vy > DOWN_THRESHOLD)) && (changed))) {
        move = true;
        changed = false;
    }
    if(Booster_Joystick_Pushed()) {
        menuSelect(yval);
        yval = 2;
        oldVal = 2;
    }
}

void returnToMenu() {
    if(Booster_Top_Button_Pushed()) {
        state = OPENING;
    }
}

void moveCar() {
    unsigned vx, vy;
    getSampleJoyStick(&vx, &vy);
    //static unsigned lane = GS.c.x;
    static bool move = false;
    if(move) {
        drawCar(GS.c.x);
        move = false;
    }
    if(vx > LEFT_THRESHOLD) {
        if(!move) {
            if(GS.c.x == 33) {
                return;
            } else {
                GS.c.x++;
            }
            move = true;
        }
    } else if(vx < RIGHT_THRESHOLD) {
        if(!move) {
            if(GS.c.x == 1) {
                return;
            } else {
                GS.c.x--;
            }
            move = true;
        }
    }
    //GS.c.x = lane;
}

void pauseGame() {
    if(Booster_Top_Button_Pushed()) {
        state = PAUSE;
        pause();
    }
}

void updateScores() {
    if(GS.c.score >= GS.scores.score1) {
        GS.scores.score3 = GS.scores.score2;
        GS.scores.score2 = GS.scores.score1;
        GS.scores.score1 = GS.c.score;
        return;
    }
    if(GS.c.score >= GS.scores.score2) {
        GS.scores.score3 = GS.scores.score2;
        GS.scores.score2 = GS.c.score;
        return;
    }
    if(GS.c.score >= GS.scores.score3) {
        GS.scores.score3 = GS.c.score;
        return;
    }
}

void addObstacles() {
    if(GS.t1active) {
        if(GS.t1.y > 127) {
            GS.t1.y = 0;
            GS.t1active = false;
            GS.c.score++;
            return;
        } else {
            GS.t1.y++;
        }
        drawTree(GS.t1.lane, GS.t1.y);
    }
    if(GS.t2active) {
        if(GS.t2.y > 127) {
            GS.t2.y = 0;
            GS.t2active = false;
            GS.c.score++;
            return;
        } else {
            GS.t2.y++;
        }
        drawTree(GS.t2.lane, GS.t2.y);
    }
    if(GS.t3active) {
        if(GS.t3.y > 127) {
            GS.t3.y = 0;
            GS.t3active = false;
            GS.c.score++;
            return;
        } else {
            GS.t3.y++;
        }
        drawTree(GS.t3.lane, GS.t3.y);
    }
    if(GS.gactive) {
        if(GS.gascan.y > 127) {
            GS.gascan.y = 0;
            GS.gactive = false;
            return;
        } else {
            GS.gascan.y++;
        }
        drawGC(GS.gascan.lane, GS.gascan.y);
    }
}



void initGamestate() {
    GS.c.y = 107;
    GS.c.x = 17;
    GS.scores.score1 = '0';
    GS.scores.score2 = '0';
    GS.scores.score3 = '0';
    GS.c.score = '0';
    GS.c.lives = '3';
    GS.treeCounter = 49;
    GS.t1active = false;
    GS.t2active = false;
    GS.t3active = false;
    GS.gactive = false;
    GS.t1.y = 0;
    GS.t1.lane = 1;
    GS.t2.y = 0;
    GS.t2.lane = 2;
    GS.t3.y = 0;
    GS.t3.lane = 3;
    GS.gascan.y = 0;
    GS.gascan.lane = 0;
    //
}

void newGame() {
    GS.c.score = '0';
    GS.c.x = 17;
    GS.c.lives = '3';
    GS.treeCounter = 49;
    GS.numTrees = 0;
    GS.t1active = false;
    GS.t2active = false;
    GS.t3active = false;
    GS.gactive = false;
    GS.t1.y = 0;
    GS.t1.lane = 1;
    GS.t2.y = 0;
    GS.t2.lane = 2;
    GS.t3.y = 0;
    GS.t3.lane = 3;
    GS.gascan.y = 0;
    GS.gascan.lane = 0;
}

void checkCollision() {
    if(GS.t1active && (GS.t1.y > 91 && GS.t1.y <= 122)) {
        if(GS.t1.lane == 1) {
            if(GS.c.x >= 1 && GS.c.x < 17) {
                goto collision;
            }
        } else if(GS.t1.lane == 2) {
            if(GS.c.x >= 2 && GS.c.x < 33) {
                goto collision;
            }
        }
        else if(GS.t1.lane == 3) {
            if(GS.c.x >= 18 && GS.c.x < 50) {
                goto collision;
            }
        }
    }
    if(GS.t2active && (GS.t2.y > 91 && GS.t2.y <= 122)) {
        if(GS.t2.lane == 1) {
            if(GS.c.x >= 1 && GS.c.x < 17) {
                goto collision;
            }
        } else if(GS.t2.lane == 2) {
            if(GS.c.x >= 2 && GS.c.x < 33) {
                goto collision;
            }
        }
        else if(GS.t2.lane == 3) {
            if(GS.c.x >= 18 && GS.c.x < 50) {
                goto collision;
            }
        }
    }
    if(GS.t3active && (GS.t3.y > 91 && GS.t3.y <= 122)) {
        if(GS.t3.lane == 1) {
            if(GS.c.x >= 1 && GS.c.x < 17) {
                goto collision;
            }
        } else if(GS.t3.lane == 2) {
            if(GS.c.x >= 2 && GS.c.x < 33) {
                goto collision;
            }
        }
        else if(GS.t3.lane == 3) {
            if(GS.c.x >= 18 && GS.c.x < 50) {
                goto collision;
            }
        }
    }
    if(GS.gactive && (GS.gascan.y >= 91 && GS.gascan.y <= 124)) {
        if(GS.gascan.lane == 1) {
            if(GS.c.x >= 1 && GS.c.x < 17) {
                goto gascan;
            }
        } else if(GS.gascan.lane == 2) {
            if(GS.c.x >= 2 && GS.c.x < 33) {
                goto gascan;
            }
        } else if(GS.gascan.lane == 3) {
            if(GS.c.x >= 18 && GS.c.x < 50) {
                goto gascan;
            }
        }
    }
    return;
    gascan:
        GS.c.lives++;
        GS.gactive = false;
        clearTree(GS.gascan.lane, GS.gascan.y);
        GS.gascan.y = 0;
        GS.gascan.lane = 0;
    return;
    collision:
        GS.c.lives--;
        GS.t1active = false;
        GS.t1.y = 0;
        GS.t1.lane = 0;
        if(GS.c.lives > 0) {
            carCrashContinue();
        }
    return;
}

unsigned randLanePicker() {
    unsigned vx, vy;
    char buf[2];
    getSampleJoyStick(&vx, &vy);
    int randBit = (vx%2) ^ (vy%2);
    unsigned loc = 0;
    buf[0] = randBit;
    buf[1] = buf[0];
    getSampleJoyStick(&vx, &vy);
    randBit = (vx%2) ^ (vy%2);
    buf[0] = randBit;
    loc = buf[1]*2 + buf[0]*1;
    if(loc == 0) {
        return 3;
    } else return loc;
}

void addTrees() {
    if(GS.numTrees == 8 && GS.treeCounter >= 50) {
        GS.gascan.lane = randLanePicker();
        GS.gactive = true;
        GS.numTrees = 0;
        GS.treeCounter = 0;
    } else if(GS.treeCounter >= 50) {
        if(!GS.t1active) {
            GS.t1.lane = randLanePicker();
            GS.t1active = true;
            GS.numTrees++;
        } else if(!GS.t2active) {
            GS.t2.lane = randLanePicker();
            GS.t2active = true;
            GS.numTrees++;
        }else if(!GS.t3active) {
            GS.t3.lane = randLanePicker();
            GS.t3active = true;
            GS.numTrees++;
        }
        GS.treeCounter = 0;
    }
}

void carCrashContinue() {
    GS.treeCounter = 49;
    GS.numTrees = 0;
    GS.t1active = false;
    GS.t2active = false;
    GS.t3active = false;
    GS.gactive = false;
    GS.t1.y = 0;
    GS.t1.lane = 1;
    GS.t2.y = 0;
    GS.t2.lane = 2;
    GS.t3.y = 0;
    GS.t3.lane = 3;
    GS.c.x = 17;
    drawCrash();
    drawGame();
}

int main(void) {
    WDT_A_hold(WDT_A_BASE);
    BSP_Clock_InitFastest();
    InitGraphics();
    InitHWTimers();
    InitButtons();
    InitLEDs();
    initADC_Multi();
    initJoyStick();
    startADC();
    initGamestate();
    while (1) {
        // Do not delete this statement. We will use this function to check if your program does not block on anything.
        if (Booster_Bottom_Button_Pushed())
            Toggle_Launchpad_Right_Red_LED();
        ScreensFSM();
    }
}
