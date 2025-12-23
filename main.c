#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL_mixer.h.>
#include <SDL2/SDL_ttf.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define SIZE 4
#define MAX_SCORES 5
#define diffuclty 1000

typedef struct {
    int move; 
    int score;
} MoveResult;
typedef struct {
    unsigned long score;
    char time[10]; 
} HighScore;
typedef enum {
    FONT_REGULAR,
    FONT_TITLE,
    FONT_SCORE
} FontStyle;
typedef struct {
    int matrix[4][4];
    SDL_bool matrixnotfull;
    unsigned long score;
    time_t startTime;
    time_t pauseTime;
    SDL_bool paused;
    time_t pauseStartTime;
} GameState;

typedef struct {
    SDL_Texture *image2;
    SDL_Texture *image4;
    SDL_Texture *image8;
    SDL_Texture *image16;
    SDL_Texture *image32;
    SDL_Texture *image64;
    SDL_Texture *image128;
    SDL_Texture *image256;
    SDL_Texture *image512;
    SDL_Texture *image1024;
    SDL_Texture *image2048;
} IMAGES;
void formatTime(time_t seconds, char* buffer) {
    int minutes = seconds / 60;
    seconds = seconds % 60;
    sprintf(buffer, "%02d:%02d", minutes, (int)seconds);
}
int evaluateBoard(int matrix[SIZE][SIZE]) {
    int score = 0;
    int emptyTiles = 0;
    int smoothness = 0;
    int monotonicity = 0;

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (matrix[i][j] == 0) {
                emptyTiles++;
            } else {
                score += matrix[i][j];
                if (i < SIZE - 1 && matrix[i + 1][j] != 0) {
                    smoothness += 1 / (1 + abs(matrix[i][j] - matrix[i + 1][j]));
                }
                if (j < SIZE - 1 && matrix[i][j + 1] != 0) {
                    smoothness += 1 / (1 + abs(matrix[i][j] - matrix[i][j + 1]));
                }
                
                if (i < SIZE - 1) {
                    monotonicity += (matrix[i][j] > matrix[i + 1][j]) ? 1 : 0; 
                }
                if (j < SIZE - 1) {
                    monotonicity += (matrix[i][j] > matrix[i][j + 1]) ? 1 : 0; 
                }
            }
        }
    }

    return score + (emptyTiles * 10) + (smoothness * 5) + (monotonicity * 1);
}
int readHighScores(HighScore scores[MAX_SCORES]) {
    FILE *file = fopen("GameSaves/scores.txt", "r");
    if (!file) {
        for (int i = 0; i < MAX_SCORES; i++) {
            scores[i].score = 0;
            strcpy(scores[i].time, "00:00");
        }
        return 0;
    }
    int count = 0;
    while (count < MAX_SCORES && fscanf(file, "%lu %s\n", &scores[count].score, scores[count].time) == 2) {
        count++;
    }

    while (count < MAX_SCORES) {
        scores[count].score = 0;
        strcpy(scores[count].time, "00:00");
        count++;
    }
    
    fclose(file);
    return count;
}
void saveHighScores(HighScore scores[MAX_SCORES]) {
    #ifdef _WIN32
        mkdir("GameSaves");
    #else
        mkdir("GameSaves", 0777);
    #endif

    FILE *file = fopen("GameSaves/scores.txt", "w");
    if (!file) {
        printf("Error: Could not open scores file for writing\n");
        return;
    }

    for (int i = 0; i < MAX_SCORES; i++) {
        if (scores[i].score > 0) {
            fprintf(file, "%lu %s\n", scores[i].score, scores[i].time);
        }
    }
    fclose(file);
}
void insertHighScore(HighScore scores[MAX_SCORES], GameState *game) {
    int pos = MAX_SCORES;
    for (int i = 0; i < MAX_SCORES; i++) {
        if (game->score > scores[i].score) {
            pos = i;
            break;
        }
    }
    
    if (pos < MAX_SCORES) {
        // Shift lower scores down
        for (int i = MAX_SCORES - 1; i > pos; i--) {
            scores[i] = scores[i-1];
        }
        // Insert new score
        char timeBuffer[10];
        formatTime(time(NULL) - game->startTime, timeBuffer);
        scores[pos].score = game->score;
        strncpy(scores[pos].time, timeBuffer, 9);
        scores[pos].time[9] = '\0';  // Ensure null-termination
        saveHighScores(scores);
    }
}
void renderHighScores(SDL_Renderer *renderer, TTF_Font *font, HighScore scores[MAX_SCORES]) {
    SDL_Color textColor = {217, 71, 9, 255};

    SDL_Surface* titleSurface = TTF_RenderText_Blended(font, "HIGH-SCORE", textColor);
    SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
    SDL_Rect titleRect = {(500 - titleSurface->w) / 2, 130, titleSurface->w, titleSurface->h };
    
    int totalHeight = titleRect.h + 20;  
    for (int i = 0; i < MAX_SCORES; i++) {
        if (scores[i].score > 0) {
            char scoreText[50];
            sprintf(scoreText, "%d: %lu %s", i + 1, scores[i].score, scores[i].time);
            SDL_Surface* scoreSurface = TTF_RenderText_Blended(font, scoreText, textColor);
            totalHeight += scoreSurface->h + 10; 
            SDL_FreeSurface(scoreSurface);
        }
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_Rect overlayRect = {(500 - 300) / 2,titleRect.y - 10, 300, totalHeight + 20
    };
    SDL_RenderFillRect(renderer, &overlayRect);

    SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
    SDL_FreeSurface(titleSurface);
    SDL_DestroyTexture(titleTexture);

    int yOffset = titleRect.y + titleRect.h + 20;
    for (int i = 0; i < MAX_SCORES; i++) {
        if (scores[i].score > 0) {
            char scoreText[50];
            sprintf(scoreText, "%d: %lu %s", i + 1, scores[i].score, scores[i].time);
            SDL_Surface* scoreSurface = TTF_RenderText_Blended(font, scoreText, textColor);
            SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
            SDL_Rect scoreRect = {(500 - scoreSurface->w) / 2, yOffset, scoreSurface->w, scoreSurface->h };
            
            SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
            SDL_FreeSurface(scoreSurface);
            SDL_DestroyTexture(scoreTexture);
            yOffset += scoreSurface->h + 10;
        }
    }
}
HighScore highest()
{
    HighScore scores[MAX_SCORES];
    readHighScores( scores);
    return  scores[0];
}
int calculateFontSize(int baseSize) {
    int windowW, windowH, logicalW, logicalH;
    float scaleX, scaleY;
    
    SDL_GetWindowSize(SDL_GetWindowFromID(1), &windowW, &windowH);
    SDL_RenderGetLogicalSize(SDL_GetRenderer(SDL_GetWindowFromID(1)), &logicalW, &logicalH);
    
    scaleX = (float)windowW / logicalW;
    scaleY = (float)windowH / logicalH;
    
    float scale = (scaleX < scaleY) ? scaleX : scaleY;
    
    return (int)(baseSize * scale - 31);
}
TTF_Font* getOptimizedFont(const char* fontPath, int fontSize, FontStyle style) {
    TTF_Font* font = TTF_OpenFont("DistantG.ttf", fontSize);
    if (!font) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        return NULL;
    }
    
    TTF_SetFontHinting(font, TTF_HINTING_NORMAL);
    
    TTF_SetFontKerning(font, 1);
    
    switch(style) {
        case FONT_TITLE:
            TTF_SetFontStyle(font, TTF_STYLE_BOLD);
            break;
        case FONT_SCORE:
            TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
            break;
        default:
            TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    }
    
    return font;
}
SDL_Texture* createSharperText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color) {
    TTF_SetFontHinting(font, TTF_HINTING_LIGHT);
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) {
        printf("TTF_RenderText_Blended: %s\n", TTF_GetError());
        return NULL;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if(texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
    }
    SDL_FreeSurface(surface);
    
    return texture;
}
void cleanup_resources(SDL_Window *window, SDL_Renderer *renderer, Mix_Music *titlemusic, Mix_Chunk *buttonpress, SDL_Texture *Solo, IMAGES *images) {
    SDL_DestroyTexture(images->image2);
    SDL_DestroyTexture(images->image4);
    SDL_DestroyTexture(images->image8);
    SDL_DestroyTexture(images->image16);
    SDL_DestroyTexture(images->image32);
    SDL_DestroyTexture(images->image64);
    SDL_DestroyTexture(images->image128);
    SDL_DestroyTexture(images->image256);
    SDL_DestroyTexture(images->image512);
    SDL_DestroyTexture(images->image1024);
    SDL_DestroyTexture(images->image2048);
    SDL_DestroyTexture(Solo);
    Mix_HaltChannel(-1);
    Mix_FreeChunk(buttonpress);
    Mix_FreeMusic(titlemusic);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    Mix_Quit();
    SDL_Quit();
}
SDL_Texture* chosephoto(int numbr, IMAGES images)
{
    if (numbr == 2) {
        return images.image2;
    } else if (numbr == 4) {
        return images.image4;
    } else if (numbr == 8) {
        return images.image8;
    } else if (numbr == 16) {
        return images.image16;
    } else if (numbr == 32) {
        return images.image32;
    } else if (numbr == 64){
        return images.image64;
    } else if (numbr == 128){
        return images.image128;
    } else if (numbr == 256) {
        return images.image256;
    } else if (numbr == 512) {
        return images.image512;
    } else if (numbr == 1024){
        return images.image1024;
    } else if (numbr == 2048){
        return images.image2048;
    }
}
void printimage(int i, int j, SDL_Renderer *renderer, IMAGES images, int matrix[SIZE][SIZE]) {
    SDL_Texture *image = chosephoto(matrix[i][j], images);
    SDL_Rect imde = { 0, 0, 25, 50 };
    SDL_QueryTexture(image, NULL, NULL, &imde.w, &imde.h);
    SDL_Rect where = { (j * 100) + 57, (i * 100) + 97.5, 85, 85};
    SDL_RenderCopy(renderer, image, &imde, &where);
}
SDL_bool AddRandomNumber (int matrix[SIZE][SIZE] )
{
    int i, j, k = 0, random_cell1;
    typedef struct {
        int row;
        int columns;
    }poz;
    poz listofpos[16];
    for ( i = 0; i < 4; i++)
    {
        for ( j = 0; j < 4; j++)
        {
            if (matrix[i][j] == 0)
            {
                listofpos[k].row = i;
                listofpos[k].columns=j;
                k++;
            }
        }
    }
    if (k == 0){
        return SDL_FALSE;
    }
    // srand(time(null)) inside the code not the function for random seeds
    random_cell1 = rand() % k;
    int x = listofpos[random_cell1].row;
    int y = listofpos[random_cell1].columns;
    matrix[x][y]= (rand() % 10 == 0) ? 4 : 2;

    return SDL_TRUE;

}
void updateScore(GameState *game, int mergedValue) {
game->score += mergedValue;
}
void right(GameState *game) {

    int matrix2[SIZE][SIZE] = {0}; 
    int i, j, jj,jjj;;
    for (i = 0; i< SIZE; i++) {
        j = 3;
         jjj= 3;
        while (j >-1) {
            // Move 'i' to the next non-zero element
            while (j >-1 && game->matrix[i][j] == 0 ) {
                j--;
            }
            jj = j -1;
            // Find the next non-zero element
            while (jj >-1&& game->matrix[i][jj] == 0) {
                jj--;
            }
            if (j > -1 && jj > -1 ) { // Valid elements found
                if (game->matrix[i][j] == game->matrix[i][jj]) {
                    matrix2[i][jjj] = game->matrix[i][j] + game->matrix[i][jj];
                    updateScore(game, matrix2[i][jjj]);
                    jjj--;
                    j = jj -1 ;  // Move past the matched pair
                } else {
                    matrix2[i][jjj] = game->matrix[i][j];

                    jjj = jjj -1;
                    j = jj;  // Move to the next position
                }
            } else if (j>-1) {
                // Only one valid element left
                matrix2[i][jjj] = game->matrix[i][j];
                jjj--;
                j--; // Move past the current element
            }
        }
        // Fill remaining with zeroes
        while (jjj >= 0) {
            matrix2[i][jjj] = 0;
            jjj--;
        }
    }
memcpy(game->matrix, matrix2, sizeof(int) * SIZE * SIZE);
}
void left(GameState *game) {
    int matrix2[SIZE][SIZE] = {0};  // Initialize matrix2 with zeroes
    int i, j, jj,jjj
    ;

    for (i = 0; i< SIZE; i++) {
        j = 0;
         jjj= 0;
        while (j<4) {
            // Move 'i' to the previous non-zero element
            while (j <4 && game->matrix[i][j] == 0) {
                j++;
            }
            jj = j+ 1 ;
            // Find the previous non-zero element
            while (jj < 4 && game->matrix[i][jj] == 0) {
                jj++;
            }
            if (j < 4 && jj < 4) { // Valid elements found
                if (game->matrix[i][j] == game->matrix[i][jj]) {
                    matrix2[i][jjj] = game->matrix[i][j] + game->matrix[i][jj];
                    updateScore(game, matrix2[i][jjj]);
                    jjj++;
                    j = jj + 1;  // Move past the matched pair
                } else {
                    matrix2[i][jjj] = game->matrix[i][j];
                    jjj = jjj +1;
                    j = jj ;  // Move to the next position
                }
            } else if (j< 4) {
                // Only one valid element left
                matrix2[i][jjj] = game->matrix[i][j];
                jjj++;
                j ++; // Move past the current element
            }
        }
        // Fill remaining with zeroes
        while (jjj<3) {
            matrix2[i][jjj] = 0;
            jjj++;
        }
    }

    memcpy(game->matrix, matrix2, sizeof(int) * SIZE * SIZE);

}
void down(GameState *game) {
    int matrix2[SIZE][SIZE] = {0};  // Initialize matrix2 with zeroes
    int i, j, ii, iii;

    for (j = 0; j < SIZE; j++) {
        i =  3;
        iii = SIZE - 1;
        while (i > -1) {
            // Move 'i' to the next non-zero element
            while (i >-1 && game->matrix[i][j] == 0) {
                i--;
            }
            ii = i -1;
            // Find the next non-zero element
            while (ii > -1 && game->matrix[ii][j] == 0) {
                ii--;
            }
            if (i > -1 && ii > -1) { // Valid elements found
                if (game->matrix[i][j] == game->matrix[ii][j]) {
                    matrix2[iii][j] = game->matrix[i][j] + game->matrix[ii][j];
                    updateScore(game, matrix2[iii][j]);
                    iii--;
                    i = ii-1 ;  // Move past the matched pair
                } else {
                    matrix2[iii][j] = game->matrix[i][j];
                    iii = iii - 1;
                    i = ii;  // Move to the next position
                }
            } else if (i > -1  ) {
                // Only one valid element left
                matrix2[iii][j] = game->matrix[i][j];
                iii--;
                i--; // Move past the current element
            }
        }
        // Fill remaining with zeroes
        while (iii >= 0) {
            matrix2[iii][j] = 0;
            iii--;
        }
    }

    memcpy(game->matrix, matrix2, sizeof(int) * SIZE * SIZE);

}
void up(GameState *game) {
    int matrix2[SIZE][SIZE] = {0};  // Initialize matrix2 with zeroes
    int i, j, ii, iii;

    for (j = 0; j < SIZE; j++) {  // Iterate over columns
        i = 0;
        iii = 0;
        while (i < SIZE) {
            // Move to the next non-zero element in the column
            while (i < SIZE && game->matrix[i][j] == 0) {
                i++;
            }
            ii = i + 1;
            // Find the next non-zero element below the current one
            while (ii < SIZE && game->matrix[ii][j] == 0) {
                ii++;
            }

            if (i < SIZE && ii < SIZE) {  // Valid elements found
                if (game->matrix[i][j] == game->matrix[ii][j]) {
                    // Merge matching tiles
                    matrix2[iii][j] = game->matrix[i][j] * 2;
                    updateScore(game, matrix2[iii][j]); // Update score before incrementing iii
                    iii++;
                    i = ii + 1;  // Move past the matched pair
                } else {
                    // Move current tile if no match
                    matrix2[iii][j] = game->matrix[i][j];
                    iii++;
                    i = ii;  // Move to the next non-zero position
                }
            } else if (i < SIZE) {
                // Only one valid element left in this column
                matrix2[iii][j] = game->matrix[i][j];
                iii++;
                i++;  // Move past the current element
            }
        }

        // Fill remaining spaces with zeroes
        while (iii < SIZE) {
            matrix2[iii][j] = 0;
            iii++;
        }
    }

    // Copy the result back into the original matrix
    memcpy(game->matrix, matrix2, sizeof(int) * SIZE * SIZE);
}
MoveResult expectimax(GameState *game, int depth, int isMaximizing) {
    if (depth == 0 || !game->matrixnotfull) {
        MoveResult result = {-1, evaluateBoard(game->matrix)};  // Evaluate the board
        return result;
    }

    if (isMaximizing) {
        MoveResult bestMove = {-1, INT_MIN};  // Initialize to smallest possible value
        for (int move = 0; move < 4; move++) {
            GameState tempGame = *game;
            switch (move) {
                case 0: up(&tempGame); break;
                case 1: down(&tempGame); break;
                case 2: left(&tempGame); break;
                case 3: right(&tempGame); break;
            }
            if (memcmp(tempGame.matrix, game->matrix, sizeof(int) * SIZE * SIZE) != 0) {
                MoveResult result = expectimax(&tempGame, depth - 1, 0);
                if (result.score > bestMove.score) {
                    bestMove.score = result.score;
                    bestMove.move = move;
                }
            }
        }
        if (bestMove.move == -1) {
            // No valid moves found, check if the game is over
            if (!game->matrixnotfull) {
                bestMove.score = -1;  // Game over
            }
        }
        return bestMove;
    } else {
        // Simulate random tile spawns
        int totalScore = 0;
        int validSpawns = 0;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (game->matrix[i][j] == 0) {
                    GameState tempGame = *game;
                    tempGame.matrix[i][j] = (rand() % 10 == 0) ? 4 : 2;  // Simulate tile spawn
                    MoveResult result = expectimax(&tempGame, depth - 1, 1);
                    totalScore += result.score;
                    validSpawns++;
                }
            }
        }
        MoveResult result = {-1, validSpawns > 0 ? totalScore / validSpawns : 0};
        return result;
    }
}
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* surface = SDL_LoadBMP(filePath);
    if (!surface) {
        fprintf(stderr, "SDL_LoadBMP Error (%s): %s\n", filePath, SDL_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        fprintf(stderr, "SDL_CreateTextureFromSurface Error (%s): %s\n", filePath, SDL_GetError());
    }
    return texture;
}
void renderGame(int matrix[SIZE][SIZE], SDL_Renderer *renderer, IMAGES images, SDL_Rect *boardRect, GameState *game, SDL_Texture *Solo) {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, Solo, NULL, NULL);
    int fontSize = calculateFontSize(24); // Base size of 24
    int fonts = calculateFontSize(21);
    TTF_Font* font = getOptimizedFont("DistantG.ttf", fontSize, FONT_SCORE);
    TTF_Font* highscoretext = getOptimizedFont("DistantG.ttf", fonts, FONT_SCORE);
    if (!font) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        return;
    }
    TTF_SetFontHinting(font, TTF_HINTING_LIGHT);
    SDL_Color textColor = {217, 71, 9, 255};

    // Render current score
    char scoreText[50];
    sprintf(scoreText, "Score: %lu", game->score);
    SDL_Texture* scoreTexture = createSharperText(renderer, font, scoreText, textColor);
    if (scoreTexture) {
        int textWidth, textHeight;
        SDL_QueryTexture(scoreTexture, NULL, NULL, &textWidth, &textHeight);
        SDL_Rect scoreRect = {boardRect->x + 10, boardRect->y - 28, textWidth, textHeight};
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
        SDL_DestroyTexture(scoreTexture);
    }

    // Render highest score
    HighScore highestScore = highest();
    char highestScoreText[50];
    sprintf(highestScoreText, "best score: %lu %s", highestScore.score,"     ", highestScore.time);
    SDL_Texture* highestScoreTexture = createSharperText(renderer, highscoretext, highestScoreText, textColor);
    if (highestScoreTexture) {
        int textWidth, textHeight;
        SDL_QueryTexture(highestScoreTexture, NULL, NULL, &textWidth, &textHeight);
        SDL_Rect highestScoreRect = { boardRect->x + 30, boardRect->y - 45, textWidth, textHeight};
        SDL_RenderCopy(renderer, highestScoreTexture, NULL, &highestScoreRect);
        SDL_DestroyTexture(highestScoreTexture);
    }

    // Render current time
    time_t currentTime = time(NULL);
    time_t elapsedTime = currentTime - game->startTime;
    if (game->paused) {
        elapsedTime -= (currentTime - game->pauseStartTime);
    }
    char timeText[50];
    char formattedTime[10];
    formatTime(elapsedTime, formattedTime);
    sprintf(timeText, "Time: %s", formattedTime);
    SDL_Texture* timeTexture = createSharperText(renderer, font, timeText, textColor);
    if (timeTexture) {
        int textWidth, textHeight;
        SDL_QueryTexture(timeTexture, NULL, NULL, &textWidth, &textHeight);
        SDL_Rect timeRect = { (boardRect->x + boardRect->w - textWidth) - 10, boardRect->y - 28, textWidth, textHeight };
        SDL_RenderCopy(renderer, timeTexture, NULL, &timeRect);
        SDL_DestroyTexture(timeTexture);
    }

    char highestTimeText[50];
    sprintf(highestTimeText, "Best Time: %s", highestScore.time);
    SDL_Texture* highestTimeTexture = createSharperText(renderer, highscoretext, highestTimeText, textColor);
    if (highestTimeTexture) {
        int textWidth, textHeight;
        SDL_QueryTexture(highestTimeTexture, NULL, NULL, &textWidth, &textHeight);
        SDL_Rect highestTimeRect = {(boardRect->x + boardRect->w - textWidth) - 30, boardRect->y - 45, textWidth, textHeight};
        SDL_RenderCopy(renderer, highestTimeTexture, NULL, &highestTimeRect);
        SDL_DestroyTexture(highestTimeTexture);
    }

    TTF_CloseFont(font);

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (matrix[i][j] != 0) {
                printimage(i, j, renderer, images, matrix);
            }
        }
    }
    SDL_RenderPresent(renderer);
}
int decideMove(int matrix[SIZE][SIZE], int a) {
    
    GameState game = {0};
    memcpy(game.matrix, matrix, sizeof(int) * SIZE * SIZE);
    game.matrixnotfull = SDL_TRUE;

    MoveResult result = expectimax(&game, a, 1); 
    return result.move;
}
void resetGame(GameState *game) {
    // Clear the matrix
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            game->matrix[i][j] = 0;
        }
    }
    game->score = 0;
    game->startTime = time(NULL);
    game->matrixnotfull = SDL_TRUE;
    AddRandomNumber(game->matrix);
}
void renderGameInRect(int matrix[SIZE][SIZE], SDL_Renderer *renderer, IMAGES images, SDL_Rect *boardRect, GameState *game) {
    int fontSize = calculateFontSize(20); // Base size of 24
    TTF_Font* font = getOptimizedFont("DistantG.ttf", fontSize, FONT_SCORE);
    if (!font) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        return;
    }
    TTF_SetFontHinting(font, TTF_HINTING_LIGHT);
    SDL_Color textColor = {217, 71, 9, 255}; 
    //score text
    char scoreText[50];
    sprintf(scoreText, "Score: %lu", game->score);
    SDL_Texture* scoreTexture = createSharperText(renderer, font, scoreText, textColor);
    if (scoreTexture) {
        int textWidth, textHeight;
        SDL_QueryTexture(scoreTexture, NULL, NULL, &textWidth, &textHeight);
        SDL_Rect scoreRect = {boardRect->x + 3, boardRect->y - 18, textWidth, textHeight};
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
        SDL_DestroyTexture(scoreTexture);
    }
    // render time
    time_t currentTime = time(NULL);
    time_t elapsedTime = currentTime - game->startTime;
    if (game->paused) 
    {
    elapsedTime -= (currentTime - game->pauseStartTime);
    }
    char timeText[50];
    char formattedTime[10];
    formatTime(elapsedTime, formattedTime);
    sprintf(timeText, "Time: %s", formattedTime);
    SDL_Texture* timeTexture = createSharperText(renderer, font, timeText, textColor);
    if (timeTexture) {
        int textWidth, textHeight;
        SDL_QueryTexture(timeTexture, NULL, NULL, &textWidth, &textHeight);
        SDL_Rect timeRect = {
            boardRect->x + boardRect->w - textWidth - 3,
            boardRect->y - 18,
            textWidth,
            textHeight
        };
        SDL_RenderCopy(renderer, timeTexture, NULL, &timeRect);
        SDL_DestroyTexture(timeTexture);
    }
    SDL_DestroyTexture(scoreTexture);
    SDL_DestroyTexture(timeTexture);
    TTF_CloseFont(font);
    int gap = 9; 
    int tileWidth = 42.5;
    int tileHeight = 42.5; 
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (matrix[i][j] != 0) {
                    SDL_Texture *image = chosephoto(matrix[i][j], images);
                    SDL_Rect tileRect = {boardRect->x + (j * (tileWidth + gap)) + 1, boardRect->y + (i * (tileHeight + gap)) + 2, tileWidth, tileHeight};
                    SDL_RenderCopy(renderer, image, NULL, &tileRect);
                }
            }
        }
}
void handlePlayerMove(GameState *game, SDL_Event event, SDL_Renderer *renderer, SDL_Texture *Solo, Mix_Chunk *buttonpress) {
    static int mouseDownX = 0, mouseDownY = 0;
    static SDL_bool mouseDown = SDL_FALSE;

    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouseDownX = event.button.x;
                mouseDownY = event.button.y;
                mouseDown = SDL_TRUE;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT && mouseDown) {
                int mouseUpX = event.button.x;
                int mouseUpY = event.button.y;
                int deltaX = mouseUpX - mouseDownX;
                int deltaY = mouseUpY - mouseDownY;
                if (abs(deltaX) > abs(deltaY)) {
                    if (deltaX > 165) 
                    {
                        right(game);
                        Mix_PlayChannel(1, buttonpress, 0);
                        game->matrixnotfull = AddRandomNumber(game->matrix);
                    } 
                    else if (deltaX < -165) 
                    {
                        left(game);
                        Mix_PlayChannel(1, buttonpress, 0);
                        game->matrixnotfull = AddRandomNumber(game->matrix);
                    }
                } 
                else 
                {
                if (deltaY > 165) 
                    {
                        down(game);
                        Mix_PlayChannel(1, buttonpress, 0);
                        game->matrixnotfull = AddRandomNumber(game->matrix);
                    } else if (deltaY < -165) 
                    { 
                        up(game);
                        Mix_PlayChannel(1, buttonpress, 0);
                        game->matrixnotfull = AddRandomNumber(game->matrix);
                    }
                }

                mouseDown = SDL_FALSE;
            }
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                case SDLK_w:
                case SDLK_z:
                    up(game);
                    Mix_PlayChannel(1, buttonpress, 0);
                    game->matrixnotfull = AddRandomNumber(game->matrix);
                    break;
                case SDLK_DOWN:
                case SDLK_s:
                    down(game);
                    Mix_PlayChannel(1, buttonpress, 0);
                    game->matrixnotfull = AddRandomNumber(game->matrix);
                    break;
                case SDLK_LEFT:
                case SDLK_a:
                case SDLK_q:
                    left(game);
                    Mix_PlayChannel(1, buttonpress, 0);
                    game->matrixnotfull = AddRandomNumber(game->matrix);
                    break;
                case SDLK_RIGHT:
                case SDLK_d:
                    right(game);
                    Mix_PlayChannel(1, buttonpress, 0);
                    game->matrixnotfull = AddRandomNumber(game->matrix);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
    SDL_Delay(16);
}
void handleAIMove(GameState *game, int move, SDL_Renderer *renderer, SDL_Texture *Solo) {
    switch (move) 
    {
        case 0: up(game); break;
        case 1: down(game); break;
        case 2: left(game); break;
        case 3: right(game); break;
    }
    game->matrixnotfull = AddRandomNumber(game->matrix);
}
void handlePauseMenu(SDL_Renderer *renderer, SDL_Texture *pauseMenuTexture, GameState *game, SDL_bool *pause, SDL_bool *quitToMainMenu) {
    SDL_Event event;
    SDL_bool inPauseMenu = SDL_TRUE;

    // Record the time when the game is paused
    game->pauseStartTime = time(NULL);
    game->paused = SDL_TRUE;

    // Define the buttons in the pause menu
    SDL_Rect resumeButton = {125, 137, 250.5, 68.5};  // Resume button
    SDL_Rect exitButton = {125, 282.5, 250.5, 68.5};    // Exit to main menu button

    while (inPauseMenu) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                *quitToMainMenu = SDL_TRUE;
                inPauseMenu = SDL_FALSE;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) 
            {
                int x = event.button.x;
                int y = event.button.y;
                if (x >= resumeButton.x && x <= resumeButton.x + resumeButton.w && y >= resumeButton.y && y <= resumeButton.y + resumeButton.h) {
                    inPauseMenu = SDL_FALSE;
                    *pause = SDL_FALSE;
                    game->paused = SDL_FALSE;
                    game->startTime += time(NULL) - game->pauseStartTime;
                }
                if (x >= exitButton.x && x <= exitButton.x + exitButton.w &&y >= exitButton.y && y <= exitButton.y + exitButton.h) 
                {
                    inPauseMenu = SDL_FALSE;
                    *quitToMainMenu = SDL_TRUE;
                    game->pauseStartTime = 0;      // Reset the pause start time
                    game->paused = SDL_FALSE;      // Reset the paused flag
                    game->matrixnotfull = SDL_TRUE;
                }
            }
        }

        // Render the pause menu
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, pauseMenuTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}
void handlePausePvM(SDL_Renderer *renderer, SDL_Texture *pauseMenuTexture, GameState *game, GameState *ai, SDL_bool *pause, SDL_bool *quitToMainMenu) {
    SDL_Event event;
    SDL_bool inPauseMenu = SDL_TRUE;

    // Record the time when the game is paused
    game->pauseStartTime = time(NULL);
    game->paused = SDL_TRUE;
    ai->pauseStartTime = time(NULL);  // Pause AI's time as well
    ai->paused = SDL_TRUE;            // Pause AI's state

    // Define the buttons in the pause menu
    SDL_Rect resumeButton = {125, 137, 250.5, 68.5};  // Resume button
    SDL_Rect exitButton = {125, 282.5, 250.5, 68.5};  // Exit to main menu button

    while (inPauseMenu) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                *quitToMainMenu = SDL_TRUE;
                inPauseMenu = SDL_FALSE;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = event.button.x;
                int y = event.button.y;

                // Check if the click is on the "Resume" button
                if (x >= resumeButton.x && x <= resumeButton.x + resumeButton.w &&
                    y >= resumeButton.y && y <= resumeButton.y + resumeButton.h) {
                    inPauseMenu = SDL_FALSE;
                    *pause = SDL_FALSE;
                    game->paused = SDL_FALSE;
                    ai->paused = SDL_FALSE;  // Resume AI's state
                    // Adjust the start time to account for the pause duration
                    game->startTime += time(NULL) - game->pauseStartTime;
                    ai->startTime += time(NULL) - ai->pauseStartTime;  // Adjust AI's time
                }

                // Check if the click is on the "Exit" button
                if (x >= exitButton.x && x <= exitButton.x + exitButton.w &&
                    y >= exitButton.y && y <= exitButton.y + exitButton.h) {
                    inPauseMenu = SDL_FALSE;
                    *quitToMainMenu = SDL_TRUE;
                    game->pauseStartTime = 0;  // Reset the pause start time
                    game->paused = SDL_FALSE;  // Reset the paused flag
                    game->matrixnotfull = SDL_TRUE;
                    ai->pauseStartTime = 0;  // Reset AI's pause start time
                    ai->paused = SDL_FALSE;  // Reset AI's paused flag
                    ai->matrixnotfull = SDL_TRUE;
                }
            }
        }

        // Render the pause menu
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, pauseMenuTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}
void resetAllGameStates(GameState *soloGame, GameState *playerGame, GameState *aiGame) {
    resetGame(soloGame);
    resetGame(playerGame);
    resetGame(aiGame);
}
void resetGameFlags(SDL_bool *matrixnotfull, SDL_bool *alone, SDL_bool *tutorial, SDL_bool *PlayerVsMachine) {
    *matrixnotfull = SDL_TRUE;
    *alone = SDL_FALSE;
    *tutorial = SDL_FALSE;
    *PlayerVsMachine = SDL_FALSE;
}
void GameOver(SDL_Renderer *renderer, GameState *game, SDL_Texture *backgroundTexture) {
    TTF_Font* font = TTF_OpenFont("DistantG.ttf", 40);
    if (!font) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        return;
    }
    SDL_Color textColor = {217, 71, 9, 255};
    SDL_Surface* gameOverSurface = TTF_RenderText_Blended(font, "GAME OVER", textColor);
    SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
    char finalScore[50];
    sprintf(finalScore, "Final Score: %lu", game->score);

    SDL_Surface* scoreSurface = TTF_RenderText_Blended(font, finalScore, textColor);
    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
    SDL_Rect gameOverRect = {(500 - gameOverSurface->w) / 2, 225, gameOverSurface->w,gameOverSurface->h };
    SDL_Rect scoreRect = {(500 - scoreSurface->w) / 2,gameOverRect.y + gameOverRect.h + 15, scoreSurface->w,scoreSurface->h};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_Rect overlay = {0, 0, 500, 500};
    SDL_RenderFillRect(renderer, &overlay);

    SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);

    HighScore highScores[MAX_SCORES];
    readHighScores(highScores);
    insertHighScore(highScores, game);

    SDL_RenderPresent(renderer);
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    SDL_RenderFillRect(renderer, &overlay);
    renderHighScores(renderer, font, highScores);
    SDL_Delay(1000);

    SDL_FreeSurface(gameOverSurface);
    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(gameOverTexture);
    SDL_DestroyTexture(scoreTexture);
    TTF_CloseFont(font);

}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == 0)
    {
    printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    return -1;
    }
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
    {
    printf("SDL_mixer could not initialize audio! SDL_mixer Error: %s\n", Mix_GetError());
    return -1;
    }
    if (TTF_Init() == -1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        return 1;
    }
    GameState soloGame = {{0}, SDL_TRUE, 0, 0};
    GameState playerGame = {{0}, SDL_TRUE, 0, 0};
    GameState aiGame = {{0}, SDL_TRUE, 0, 0};
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "10");
    SDL_Window *window = SDL_CreateWindow("2048 G3", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(renderer, 500, 500);
    if (TTF_Init() == -1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    return 1;
}
    SDL_Surface* iconesurface = SDL_LoadBMP("photos/window icon.bmp");
    SDL_SetWindowIcon(window,iconesurface);
    SDL_FreeSurface(iconesurface);
    IMAGES images;
    images.image2 = loadTexture(renderer, "photos/2IMAGE.bmp");
    images.image4 = loadTexture(renderer, "photos/4IMAGE.bmp");
    images.image8 = loadTexture(renderer, "photos/8IMAGE.bmp");
    images.image16 = loadTexture(renderer, "photos/16IMAGE.bmp");
    images.image32 = loadTexture(renderer, "photos/32IMAGE.bmp");
    images.image64 = loadTexture(renderer, "photos/64IMAGE.bmp");
    images.image128 = loadTexture(renderer, "photos/128IMAGE.bmp");
    images.image256 = loadTexture(renderer, "photos/256IMAGE.bmp");
    images.image512 = loadTexture(renderer, "photos/512IMAGE.bmp");
    images.image1024 = loadTexture(renderer, "photos/1024IMAGE.bmp");
    images.image2048 = loadTexture(renderer, "photos/2048IMAGE.bmp");
    SDL_Texture* MainMenu = loadTexture(renderer, "photos/Main Menu.bmp");
    SDL_Texture* Solo = loadTexture(renderer, "photos/Solo.bmp");
    SDL_Texture* PLayerVsMachinephoto = loadTexture(renderer, "photos/PLAYER VS MACHINE.BMP");
    SDL_Texture* pauseMenuTexture = loadTexture(renderer, "photos/pausemenu.bmp");
    SDL_Rect backgroundRect = {0, 0, 500, 500}; 
    SDL_RenderCopy(renderer, MainMenu, NULL, &backgroundRect);
    SDL_RenderPresent(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
    SDL_RenderClear(renderer);
    SDL_bool quit = SDL_TRUE;
    SDL_bool matrixnotfull = AddRandomNumber(soloGame.matrix);
    SDL_Event event;
    int sound = 0;
    srand(time(NULL));
    Mix_Music* titlemusic = Mix_LoadMUS("Sounds/Main menu.mp3");
    Mix_Chunk* buttonpress = Mix_LoadWAV("Sounds/ButtonClick.wav");
    SDL_Texture* mouseover = loadTexture(renderer,"photos/mouseover click.bmp");
    SDL_Texture* playVSmachine = loadTexture(renderer, "photos/PlayVSmachine.bmp");
    SDL_Texture* clickPlayVSMachine = loadTexture(renderer, "photos/click play vs machine.bmp");
    SDL_Texture* clickTutorial = loadTexture(renderer, "photos/click tutorial.bmp");
    SDL_Texture* mouseoverPlayerVSMachine = loadTexture(renderer, "photos/Mouseover player vs machine.bmp");
    SDL_Texture* mouseoverTutorial = loadTexture(renderer, "photos/mouseover tutorial.bmp");
    SDL_Texture* aiDifficulty = loadTexture(renderer, "photos/aiDifficulty.bmp");   
    SDL_bool press_start = SDL_TRUE;
    Mix_PlayMusic(titlemusic, -1);
    SDL_Rect pauseButton = {29, 26, 41, 36.5};
    SDL_Rect gameBoard = {50, 97.5, 400, 400}; 
    SDL_bool isHovering1 = SDL_TRUE;
    SDL_bool isHovering2 = SDL_TRUE;
    SDL_bool isHovering3 = SDL_TRUE;
    SDL_Rect region1 = {175, 295, 150, 35};   // play alone
    SDL_Rect region2 = {147.5, 359, 205, 40};   // play vs machine
    SDL_Rect region3 = {175, 427.5, 150, 40};   // tutorial
    SDL_bool alone = SDL_FALSE;
    SDL_bool tutorial = SDL_FALSE;
    SDL_bool game = SDL_TRUE;
    SDL_bool PlayerVsMachine = SDL_FALSE;
    Uint32 lastAIMoveTime = 0; 
    Uint32 aiMoveInterval = diffuclty;
    int a ;
    SDL_bool pauseclicked = SDL_FALSE;
while (game)
{
    // main menu loop
    while (press_start) {
    if (SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quit = SDL_FALSE;
                press_start = SDL_FALSE;
                game = SDL_FALSE;
                break;
            case SDL_MOUSEMOTION:
                if (event.motion.x > region1.x && event.motion.y > region1.y && event.motion.x < region1.x + region1.w && event.motion.y < region1.y + region1.h) {
                if (!isHovering1) {
                    isHovering1 = SDL_TRUE;
                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, mouseover, NULL, &backgroundRect);
                    SDL_RenderPresent(renderer);
                }
                } else if (isHovering1) {
                isHovering1 = SDL_FALSE;
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, MainMenu, NULL, &backgroundRect);
                SDL_RenderPresent(renderer);
            }
                if (event.motion.x > region2.x && event.motion.y > region2.y &&  event.motion.x < region2.x + region2.w && event.motion.y < region2.y + region2.h) {
                if (!isHovering2) {
                    isHovering2 = SDL_TRUE;
                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, mouseoverPlayerVSMachine, NULL, &backgroundRect);
                    SDL_RenderPresent(renderer);
                }
                } else if (isHovering2) {
                isHovering2 = SDL_FALSE;
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, MainMenu, NULL, &backgroundRect);
                SDL_RenderPresent(renderer);
            }
                if (event.motion.x > region3.x && event.motion.y > region3.y && event.motion.x < region3.x + region3.w && event.motion.y < region3.y + region3.h) {
                if (!isHovering3) {
                    isHovering3 = SDL_TRUE;
                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, mouseoverTutorial, NULL, &backgroundRect);
                    SDL_RenderPresent(renderer);
                }
                } else if (isHovering3) {
                isHovering3 = SDL_FALSE;
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, MainMenu, NULL, &backgroundRect);
                SDL_RenderPresent(renderer);
            }
            break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.motion.x > region1.x && event.motion.y > region1.y && event.motion.x < region1.x + region1.w && event.motion.y < region1.y + region1.h) {
                    // Play Alone clicked
                    matrixnotfull = AddRandomNumber(soloGame.matrix);
                    SDL_RenderClear(renderer);
                    SDL_Texture* clickplayalone = loadTexture(renderer, "photos/after playalone click.bmp");
                    SDL_RenderCopy(renderer, clickplayalone, NULL, &backgroundRect);
                    SDL_RenderPresent(renderer);
                    SDL_Delay(100);
                    SDL_DestroyTexture(clickplayalone);
                    resetGame(&soloGame);
                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, Solo, NULL, &backgroundRect);
                    renderGame(soloGame.matrix, renderer, images, &gameBoard, &soloGame, Solo);
                    press_start = SDL_FALSE;
                    alone = SDL_TRUE;
                } else if (event.button.x > 451.5 && event.button.y > 27 && event.button.x < 473.5 && event.button.y < 45) {
                    sound = !sound;
                    Mix_VolumeMusic(sound ? 0 : MIX_MAX_VOLUME);
                } else if (event.motion.x > region3.x && event.motion.y > region3.y &&  event.motion.x < region3.x + region3.w && event.motion.y < region3.y + region3.h) 
                {
                    // Tutorial clicked
                    resetAllGameStates(&soloGame, &playerGame, &aiGame);
                    resetGameFlags(&matrixnotfull, &alone, &tutorial, &PlayerVsMachine);
                    tutorial = SDL_TRUE;

                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, clickTutorial, NULL, &backgroundRect);
                    SDL_RenderPresent(renderer);
                    SDL_Delay(100);

                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, aiDifficulty, NULL, &backgroundRect);
                    SDL_RenderPresent(renderer);

                    SDL_bool chosemode = SDL_FALSE;
                    SDL_bool difficultySelected = SDL_FALSE;

                    SDL_Rect easyRegion = {175, 295, 150, 35};   // Easy difficulty
                    SDL_Rect mediumRegion = {147.5, 359, 205, 40};   // Medium difficulty
                    SDL_Rect hardRegion = {175, 427.5, 150, 40};   // Hard difficulty

                    while (!chosemode) 
                    {
                        while (SDL_PollEvent(&event)) 
                        {
                            if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_KEYDOWN) 
                            {
                                if (event.button.x > easyRegion.x && event.button.y > easyRegion.y && event.button.x < easyRegion.x + easyRegion.w && event.button.y < easyRegion.y + easyRegion.h) 
                                {
                                    aiMoveInterval = 1500;
                                    a = 3; 
                                    chosemode = SDL_TRUE;
                                    difficultySelected = SDL_TRUE;
                                }  else if (event.button.x > mediumRegion.x && event.button.y > mediumRegion.y && event.button.x < mediumRegion.x + mediumRegion.w && event.button.y < mediumRegion.y + mediumRegion.h) {
                                    aiMoveInterval = 1000; 
                                    a=4;
                                    chosemode = SDL_TRUE;
                                    difficultySelected = SDL_TRUE;
                                }  else if (event.button.x > hardRegion.x && event.button.y > hardRegion.y && event.button.x < hardRegion.x + hardRegion.w && event.button.y < hardRegion.y + hardRegion.h) 
                                {
                                    aiMoveInterval = 500;
                                    a = 6;
                                    chosemode = SDL_TRUE;
                                    difficultySelected = SDL_TRUE;
                                } else if (event.button.x > 451.5 && event.button.y > 27 && event.button.x < 473.5 && event.button.y < 45) 
                                {
                                    sound = !sound;
                                    Mix_VolumeMusic(sound ? 0 : MIX_MAX_VOLUME);
                                } else if (event.type == SDL_KEYDOWN) 
                                {
                                    if (event.key.keysym.sym == SDLK_ESCAPE) 
                                    {
                                        chosemode = SDL_FALSE;
                                        difficultySelected = SDL_FALSE;
                                        tutorial = SDL_FALSE;
                                        press_start = SDL_TRUE;
                                        break;
                                    }
                                }
                            }
                        }
                        if (difficultySelected && chosemode && tutorial) 
                        {
                            SDL_Delay(100);
                            SDL_RenderClear(renderer);
                            SDL_RenderCopy(renderer, Solo, NULL, &backgroundRect);
                            renderGame(aiGame.matrix, renderer, images, &gameBoard, &aiGame, Solo);
                            press_start = SDL_FALSE;
                        } else if (!difficultySelected && !chosemode && !tutorial)
                        {
                            SDL_RenderClear(renderer);
                            SDL_RenderCopy(renderer, MainMenu, NULL, &backgroundRect);
                            SDL_RenderPresent(renderer);
                        }
                    }
                } else if (event.motion.x > region2.x && event.motion.y > region2.y && event.motion.x < region2.x + region2.w && event.motion.y < region2.y + region2.h) 
                {
                    // Player vs Machine clicked
                    resetAllGameStates(&soloGame, &playerGame, &aiGame);
                    resetGameFlags(&matrixnotfull, &alone, &tutorial, &PlayerVsMachine);

                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, clickPlayVSMachine, NULL, &backgroundRect);
                    SDL_RenderPresent(renderer);
                    SDL_Delay(100);

                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, aiDifficulty, NULL, &backgroundRect);
                    SDL_RenderPresent(renderer);

                    SDL_bool chosemode = SDL_FALSE;
                    SDL_bool difficultySelected = SDL_FALSE;

                    SDL_Rect easyRegion = {175, 295, 150, 35};   // Easy difficulty
                    SDL_Rect mediumRegion = {147.5, 359, 205, 40};   // Medium difficulty
                    SDL_Rect hardRegion = {175, 427.5, 150, 40};   // Hard difficulty

                    while (!chosemode) 
                    {
                        while (SDL_PollEvent(&event)) 
                        {
                            if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_KEYDOWN) 
                            {
                                if (event.button.x > easyRegion.x && event.button.y > easyRegion.y && event.button.x < easyRegion.x + easyRegion.w && event.button.y < easyRegion.y + easyRegion.h) 
                                {
                                    aiMoveInterval = 1500;
                                    a = 3;
                                    chosemode = SDL_TRUE;
                                    difficultySelected = SDL_TRUE;
                                    PlayerVsMachine = SDL_TRUE;
                                } else if (event.button.x > mediumRegion.x && event.button.y > mediumRegion.y && event.button.x < mediumRegion.x + mediumRegion.w && event.button.y < mediumRegion.y + mediumRegion.h) 
                                {
                                    a = 4;
                                    aiMoveInterval = 1000;
                                    chosemode = SDL_TRUE;
                                    difficultySelected = SDL_TRUE;
                                    PlayerVsMachine = SDL_TRUE;
                                } else if (event.button.x > hardRegion.x && event.button.y > hardRegion.y && event.button.x < hardRegion.x + hardRegion.w && event.button.y < hardRegion.y + hardRegion.h) 
                                {
                                    a = 6;
                                    aiMoveInterval = 500;
                                    chosemode = SDL_TRUE;
                                    difficultySelected = SDL_TRUE;
                                    PlayerVsMachine = SDL_TRUE;
                                } else if (event.button.x > 451.5 && event.button.y > 27 && event.button.x < 473.5 && event.button.y < 45) 
                                {
                                    sound = !sound;
                                    Mix_VolumeMusic(sound ? 0 : MIX_MAX_VOLUME);
                                } else if (event.type == SDL_KEYDOWN) 
                                {
                                    if (event.key.keysym.sym == SDLK_ESCAPE) 
                                        {
                                            chosemode = SDL_TRUE;
                                            difficultySelected = SDL_FALSE;
                                            PlayerVsMachine = SDL_FALSE;
                                            press_start = SDL_TRUE;        
                                        }
                                }
                            }
                        }

                        if (difficultySelected && chosemode && PlayerVsMachine) 
                        {
                            SDL_Delay(100);
                            SDL_RenderClear(renderer);
                            SDL_RenderCopy(renderer, PLayerVsMachinephoto, NULL, &backgroundRect);
                            renderGame(playerGame.matrix, renderer, images, &gameBoard, &playerGame, PLayerVsMachinephoto);
                            press_start = SDL_FALSE;
                            PlayerVsMachine = SDL_TRUE;
                        } else if (!difficultySelected && chosemode && !PlayerVsMachine)
                        {
                            SDL_RenderClear(renderer);
                            SDL_RenderCopy(renderer, MainMenu, NULL, &backgroundRect);
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_DOWN:
                        SDL_RenderClear(renderer);
                        SDL_RenderCopy(renderer, playVSmachine, NULL, &backgroundRect);
                        SDL_RenderPresent(renderer);
                        break;
                    case SDL_SCANCODE_RETURN:
                        matrixnotfull = AddRandomNumber(soloGame.matrix);
                        SDL_RenderClear(renderer);
                        SDL_RenderCopy(renderer, Solo, NULL, &backgroundRect);
                        renderGame(soloGame.matrix, renderer, images, &gameBoard, &soloGame, Solo);
                        press_start = SDL_FALSE;
                        break;
                }
                        if (event.key.keysym.sym == SDLK_ESCAPE) {
                        game = SDL_FALSE;
                        press_start = SDL_FALSE;
                        break;
                    }
                break;
        }
    }
    SDL_Delay(16);
}
    // play alone loop
    while (soloGame.matrixnotfull && alone)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    soloGame.matrixnotfull = SDL_FALSE;
                    game = SDL_FALSE;
                    break;
                case SDL_KEYDOWN:
                    handlePlayerMove(&soloGame, event, renderer, Solo, buttonpress);
                        if (event.key.keysym.sym == SDLK_ESCAPE) 
                        {
                            SDL_bool pause = SDL_TRUE;
                            SDL_bool quitToMainMenu = SDL_FALSE;
                            handlePauseMenu(renderer, pauseMenuTexture, &soloGame, &pause, &quitToMainMenu);
                            if (quitToMainMenu)  
                            {
                                soloGame.matrixnotfull = SDL_FALSE;
                                alone = SDL_FALSE;
                                press_start = SDL_TRUE;
                            }
                        } 
                        else if (!soloGame.paused)
                        {
                            handlePlayerMove(&soloGame, event, renderer, Solo, buttonpress);  
                        }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.motion.x > 451.5 && event.motion.y > 27 && event.motion.x < 473.5 && event.motion.y < 18)
                    {
                        sound = !sound;
                        Mix_VolumeMusic(sound ? 0 : MIX_MAX_VOLUME);
                    }
                    handlePlayerMove(&soloGame, event, renderer, Solo, buttonpress);
                    if (event.button.x >= pauseButton.x && event.button.y >= pauseButton.y && event.button.x <= pauseButton.x + pauseButton.w && event.button.y <= pauseButton.y + pauseButton.h) {

                    SDL_bool pause = SDL_TRUE;
                    SDL_bool quitToMainMenu = SDL_FALSE;
                    handlePauseMenu(renderer, pauseMenuTexture, &soloGame, &pause, &quitToMainMenu);
                    if (quitToMainMenu) {
                        soloGame.matrixnotfull = SDL_FALSE;
                        alone = SDL_FALSE;
                        press_start = SDL_TRUE;
                    }
                }
                break;
                case SDL_MOUSEBUTTONUP:
                    handlePlayerMove(&soloGame, event, renderer, Solo, buttonpress);
                    break;
            }

        }

        if (!soloGame.paused) 
        {
            SDL_Rect gameBoard = {50, 97.5, 400, 400};
            renderGame(soloGame.matrix, renderer, images, &gameBoard, &soloGame, Solo);
            SDL_RenderPresent(renderer);
        }

        SDL_Delay(16);

        if (!soloGame.matrixnotfull)
        {
                            GameOver(renderer, &soloGame, Solo);
                            SDL_RenderPresent(renderer);
                            SDL_Delay(6000); 
                            resetGame(&soloGame);
                            SDL_RenderClear(renderer);
                            SDL_RenderCopy(renderer, MainMenu, NULL, &backgroundRect);
                            SDL_RenderPresent(renderer);
                            alone = SDL_FALSE;
                            press_start = SDL_TRUE;
        }
    }
     // tutorial loop
    while (matrixnotfull && tutorial) 
    {
        while (SDL_PollEvent(&event)) 
        {
            if (event.type == SDL_QUIT) 
            {
                quit = SDL_FALSE;
                tutorial = SDL_FALSE;
                press_start = SDL_TRUE;
                game = SDL_FALSE;
                break;
            }
            if (event.type == SDL_KEYDOWN) 
            {
                if (event.key.keysym.sym == SDLK_ESCAPE) 
                {
                    // Trigger the pause menu
                    SDL_bool pause = SDL_TRUE;
                    SDL_bool quitToMainMenu = SDL_FALSE;
                    handlePauseMenu(renderer, pauseMenuTexture, &aiGame, &pause, &quitToMainMenu);
                    if (quitToMainMenu) 
                    {
                        // Exit to main menu
                        tutorial = SDL_FALSE;
                        press_start = SDL_TRUE;
                        break;
                     }
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) 
            {
                // Check if the pause button is clicked
                if (event.button.x >= pauseButton.x && event.button.y >= pauseButton.y && event.button.x <= pauseButton.x + pauseButton.w && event.button.y <= pauseButton.y + pauseButton.h) 
                {
                    // Trigger the pause menu
                    SDL_bool pause = SDL_TRUE;
                    SDL_bool quitToMainMenu = SDL_FALSE;
                    handlePauseMenu(renderer, pauseMenuTexture, &aiGame, &pause, &quitToMainMenu);
                    if (quitToMainMenu) 
                    {
                        // Exit to main menu
                        tutorial = SDL_FALSE;
                        press_start = SDL_TRUE;
                        break;
                    }
                }
        }
        }
        if (!tutorial) 
        {
            break;  // Exit the tutorial loop if ESCAPE was pressed
        }

    if (!aiGame.paused) 
    {
        int move = decideMove(aiGame.matrix, a);
        if (move == -1) 
        {
            printf("Game Over!\n");
            resetGame(&aiGame);
            tutorial = SDL_FALSE;
            press_start = SDL_TRUE;
            break;
        }
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, Solo, NULL, &backgroundRect);
        switch (move) 
        {
            case 0: up(&aiGame); Mix_PlayChannel(1, buttonpress, 0); SDL_Delay(aiMoveInterval); break;
            case 1: down(&aiGame); Mix_PlayChannel(1, buttonpress, 0); SDL_Delay(aiMoveInterval); break;
            case 2: left(&aiGame); Mix_PlayChannel(1, buttonpress, 0); SDL_Delay(aiMoveInterval); break;
            case 3: right(&aiGame); Mix_PlayChannel(1, buttonpress, 0); SDL_Delay(aiMoveInterval); break;
        }
        Mix_PlayChannel(1, buttonpress, 0);
        matrixnotfull = AddRandomNumber(aiGame.matrix);
        renderGame(aiGame.matrix, renderer, images, &gameBoard, &aiGame, Solo);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}
    // playervsmachine loop
    while (matrixnotfull && PlayerVsMachine) 
{
    resetGame(&playerGame);
    resetGame(&aiGame);
    SDL_Rect leftBoard = {27, 276.5, 198, 198};  // Left side for player
    SDL_Rect rightBoard = {277, 26.5, 198, 198}; // Right side for AI
    SDL_bool gameEnded = SDL_FALSE;
    SDL_bool playerLost = SDL_FALSE;
    SDL_bool aiLost = SDL_FALSE;
    Uint32 lastAIMoveTime = SDL_GetTicks();  // Initialize the last AI move time
    SDL_bool pausequit = SDL_FALSE;

    while (!gameEnded ) 
    {    
        while (SDL_PollEvent(&event)) 
        {
            switch (event.type) 
            {
                case SDL_QUIT:
                    quit = SDL_FALSE;
                    PlayerVsMachine = SDL_FALSE;
                    press_start = SDL_FALSE;
                    game = SDL_FALSE;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) 
                    {
                        SDL_bool pause = SDL_TRUE;
                        SDL_bool quitToMainMenu = SDL_FALSE;
                        handlePauseMenu(renderer, pauseMenuTexture, &playerGame, &pause, &quitToMainMenu);
                        if (quitToMainMenu) 
                        {
                            // Exit to main menu
                            PlayerVsMachine = SDL_FALSE;
                            press_start = SDL_TRUE;
                            gameEnded = SDL_TRUE;
                            pausequit = SDL_TRUE;
                            break;
                        }
                    } 
                    else if (!playerLost && !playerGame.paused) 
                    {
                        handlePlayerMove(&playerGame, event, renderer, PLayerVsMachinephoto, buttonpress);
                        playerGame.matrixnotfull = AddRandomNumber(playerGame.matrix);
                    } 
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.x > 451.5 && event.button.y > 27 && event.button.x < 473.5 && event.button.y < 45) 
                    {
                        sound = !sound;
                        Mix_VolumeMusic(sound ? 0 : MIX_MAX_VOLUME);
                    }
                    if(!playerLost)
                    {
                        handlePlayerMove(&playerGame, event, renderer, PLayerVsMachinephoto, buttonpress);
                    }
                    if (event.button.x >= pauseButton.x && event.button.y >= pauseButton.y && event.button.x <= pauseButton.x + pauseButton.w && event.button.y <= pauseButton.y + pauseButton.h) 
                    {
                        // Trigger the pause menu
                        SDL_bool pause = SDL_TRUE;
                        SDL_bool quitToMainMenu = SDL_FALSE;
                        handlePausePvM(renderer, pauseMenuTexture, &playerGame, &aiGame, &pause, &quitToMainMenu);
                        if (quitToMainMenu) 
                        {
                            // Exit to main menu
                            PlayerVsMachine = SDL_FALSE;
                            press_start = SDL_TRUE;
                            gameEnded = SDL_TRUE;
                            pausequit = SDL_TRUE;
                            break;
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if(!playerLost)
                    {
                        handlePlayerMove(&playerGame, event, renderer, PLayerVsMachinephoto, buttonpress);
                    }
                    break;
            }
        }

        if (!aiLost && aiGame.matrixnotfull && !playerGame.paused) {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - lastAIMoveTime >= aiMoveInterval) {
                int move = decideMove(aiGame.matrix, a);
                if (move != -1) {
                    handleAIMove(&aiGame, move, renderer, PLayerVsMachinephoto);
                    Mix_PlayChannel(1, buttonpress, 0);
                } else {
                    aiLost = SDL_TRUE;
                }
                lastAIMoveTime = currentTime;
            }
        }
        
        if (!playerGame.paused && !aiGame.paused) 
        {
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, PLayerVsMachinephoto, NULL, NULL);
            renderGameInRect(playerGame.matrix, renderer, images, &leftBoard, &playerGame);
            renderGameInRect(aiGame.matrix, renderer, images, &rightBoard, &aiGame);
        }

        if (!playerGame.matrixnotfull)
        {
            playerLost = SDL_TRUE;
        }

        if (!aiGame.matrixnotfull) 
        {
            aiLost = SDL_TRUE;
        }
        
        if (playerLost || aiLost) 
        {
            TTF_Font* font = TTF_OpenFont("DistantG.ttf", 20);
                SDL_Color textColor = {217, 71, 9, 255};
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
                if (playerLost) 
                {
                    SDL_Rect playerOverlay = leftBoard;
                    SDL_RenderFillRect(renderer, &playerOverlay);
                    SDL_Surface* gameOverSurface = TTF_RenderText_Blended(font, "GAME OVER", textColor);
                    if (gameOverSurface)
                    {
                        SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
                        SDL_Rect gameOverRect = {leftBoard.x + (leftBoard.w - gameOverSurface->w) / 2, leftBoard.y + (leftBoard.h - gameOverSurface->h) / 2, gameOverSurface->w, gameOverSurface->h};
                        SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
                        char scoreText[50];
                        sprintf(scoreText, "Score: %lu", playerGame.score);
                        SDL_Surface* scoreSurface = TTF_RenderText_Blended(font, scoreText, textColor);
                        if (scoreSurface) 
                        {
                            SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
                            SDL_Rect scoreRect = {leftBoard.x + (leftBoard.w - scoreSurface->w) / 2, gameOverRect.y + gameOverRect.h + 20, scoreSurface->w, scoreSurface->h};
                            SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
                            SDL_FreeSurface(scoreSurface);
                            SDL_DestroyTexture(scoreTexture);
                        }
                        SDL_FreeSurface(gameOverSurface);
                        SDL_DestroyTexture(gameOverTexture);
                    }
                }
                if (aiLost) 
                {
                    SDL_Rect aiOverlay = rightBoard;
                    SDL_RenderFillRect(renderer, &aiOverlay);
                    SDL_Surface* gameOverSurface = TTF_RenderText_Blended(font, "GAME OVER", textColor);
                    if (gameOverSurface) 
                    {
                        SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
                        SDL_Rect gameOverRect = {rightBoard.x + (rightBoard.w - gameOverSurface->w) / 2, rightBoard.y + (rightBoard.h - gameOverSurface->h) / 2, gameOverSurface->w, gameOverSurface->h};
                        SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
                        char scoreText[50];
                        sprintf(scoreText, "Score: %lu", aiGame.score);
                        SDL_Surface* scoreSurface = TTF_RenderText_Blended(font, scoreText, textColor);
                        if (scoreSurface) 
                        {
                            SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
                            SDL_Rect scoreRect = {rightBoard.x + (rightBoard.w - scoreSurface->w) / 2, gameOverRect.y + gameOverRect.h + 20, scoreSurface->w, scoreSurface->h};
                            SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
                            SDL_FreeSurface(scoreSurface);
                            SDL_DestroyTexture(scoreTexture);
                        }
                        SDL_FreeSurface(gameOverSurface);
                        SDL_DestroyTexture(gameOverTexture);
                    }
                }
                TTF_CloseFont(font);
            static Uint32 firstLossTime = 0;
            if (firstLossTime == 0) {
                firstLossTime = SDL_GetTicks();
            }
            if ( (playerLost && aiLost) || SDL_GetTicks() - firstLossTime > 1000000) {
                gameEnded = SDL_TRUE;
                firstLossTime = 0;
            }
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (gameEnded && !pausequit) 
    {
        SDL_Delay(3000);
        resetAllGameStates(&soloGame, &playerGame, &aiGame);
        resetGameFlags(&matrixnotfull, &alone, &tutorial, &PlayerVsMachine);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, MainMenu, NULL, &backgroundRect);
        SDL_RenderPresent(renderer);
        press_start = SDL_TRUE;
    }
}
    
    if (!game)
    {
        cleanup_resources(window, renderer, titlemusic, buttonpress, Solo, &images);
        return 0;
    }
}
} 