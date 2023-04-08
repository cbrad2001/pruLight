/*  game.h
*   
*   This file defines the functions that contain the game logic.
*/

#ifndef _GAME_H_
#define _GAME_H_

// Starts and stops the thread that drives and contains the game logic.
void Game_start(void);
void Game_end(void);

// Function to pause execution of a thread until Game_end() is called and the game ends.
void Game_wait(void);

// Gets the current score
int Game_getCurrentScore(void);

#endif