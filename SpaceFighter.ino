/* 
 * SpaceFighter
 * Copyright (C) 2015 
 * Maicon Hieronymus <mhierony@students.uni-mainz.de>
 * All rights reserved.
 * 
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 * -----------------------------------------------------------------------------
 *  
 * @file        SpaceFighter.ino
 * @date        14.10.2015
 * @version     0.1
 * @author      Maicon Hieronymus <mhierony@students.uni-mainz.de>
 *
 * @brief This program is a game similar to Snake on old mobile phones. 
 * It is coded with Arduboy in mind. Since I do not have an Arduboy yet,
 * I have not tested anything yet. 
 * Enter name and display highscore are taken from Sebastian Goscik as seen in
 * https://github.com/Arduboy/Arduboy/blob/master/examples/ArduBreakout/ArduBreakout.ino
 * and slightly modified.
 */

#include "Arduboy.h"

#define FIRE_BUTTON 1
#define PAUSE_BUTTON 2
#define DOWN 4
#define RIGHT 8
#define UP 16
#define LEFT 32

typedef struct
{
    // All coordinates are the upper right corner of an image.
    byte x;
    byte y;
    byte height;
    byte width;
    unsigned int score;
    byte destroyed_ships;
    byte lives;
    boolean alive;
    byte numberOfSuperbombs;
    // set invincible to 1 and then every two second a left-shift until invincible
    // is zero. This way you are 16 seconds invincible. Or less, if you set
    // invincible to 2, 4 or 8.
    byte invincible;
    byte bulletType;
} Player;

typedef struct
{
    byte x;
    byte y;
} Star;

typedef struct
{
    // All coordinates are the upper right corner of an image.
    byte x;
    byte y;
    byte height;
    byte width;
    // Each number is a placeholder for different ships. These types define
    // the image, bullettype and speed. The highest 1 indicates the type
    // e.g. 0100 is the same type as 0111 or 0101 or 0110.
    byte shipType;
    byte lifepoints;
    boolean alive;
    // Each number is a placeholder for another movement, e.g. straight-forward
    // or sine.
    byte movement;
    // If true, create a random supply after destruction.
    boolean supply;
} Enemy;

typedef struct
{
    // All coordinates are the upper right corner of an image.
    byte x;
    byte y;
    byte height;
    byte width;
    byte appearance;
    byte damage;
    byte speed;
    boolean alive;
    boolean playersBullet;
} Bullet;

typedef struct
{
    byte x;
    byte y;
    boolean alive;
    // Each number is a placeholder for different supplies, e.g. extra live
    // superbomb, weapon upgrade or invincibility (or even ship upgrade?).
    byte type;
} Supply;

// Counter the frames. Every 60 frames is one second. This can be used for 
// measuring time.
byte frameCounter;
// The stars in the background are saved here.
Star stars[20];
byte numberOfStars = 0;
byte SCREEN_HEIGHT = 64;
byte SCREEN_WIDTH = 128;
Player player;
Arduboy arduboy;
Enemy enemies[10];
byte numberOfEnemies;
Bullet bullets[100];
byte numberOfBullets;

/**
 * @brief Intro with wooosh - Arduboy.
 */
void showIntro()
{
    for(i = 0; i < 16; i++)
    {
        arduboy.clearDisplay();
        // X, Y, name, width, height, color
        arduboy.drawBitmap(0, 0, arduboyLogo, 128, 64, 1); 
        arduboy.display();
    }

    arduboy.tunes.tone(987, 160);
    delay(160);
    arduboy.tunes.tone(1318, 400);
    delay(2000);
}

/**
 * @brief Titlescreen with sound - SpaceFighter by me.
 * TODO: create nice music.
 */
void showTitle()
{
    for(i = 0; i < 16; i++)
    {
        arduboy.clearDisplay();
        // X, Y, name, width, height, color
        arduboy.drawBitmap(0, 0, title, 128, 64, 1); 
        arduboy.display();
    }

    arduboy.tunes.tone(987, 160);
    delay(160);
    arduboy.tunes.tone(1318, 400);
    delay(2000);
    
    for(i = 0; i < 16; i++)
    {
        arduboy.clearDisplay();
        // X, Y, name, width, height, color
        arduboy.drawBitmap(0, 0, author, 128, 64, 1); 
        arduboy.display();
    }
    
    arduboy.tunes.tone(987, 160);
    delay(160);
    arduboy.tunes.tone(1318, 400);
    delay(2000);
}

/**
 * @brief Display Highscore with the scores. 
 * This method is from Sebastian Goscik as seen in
 * https://github.com/Arduboy/Arduboy/blob/master/examples/ArduBreakout/ArduBreakout.ino
 * and slightly modified.
 * Function by nootropic design to display highscores.
 * TODO: Alter for bigger scores.
 */
void showHighscore()
{
    byte y = 10;
    byte x = 24;
    // Each block of EEPROM has 10 high scores, and each high score entry
    // is 5 bytes long:  3 bytes for initials and two bytes for score.
    int address = 2*10*5;
    byte hi, lo;
    arduboy.clearDisplay();
    arduboy.drawBitmap(0, 0, highscore, 128, 24, 1);
    arduboy.display();

    for(i=0; i<10; i++)
    {
        sprintf(text, "%2d", i+1);
        arduboy.setCursor(x,y+(i*8));
        arduboy.print(text);
        arduboy.display();
        hi = EEPROM.read(address + (5*i));
        lo = EEPROM.read(address + (5*i) + 1);

        if ((hi == 0xFF) && (lo == 0xFF))
        {
            score = 0;
        }
        else
        {
            score = (hi << 8) | lo;
        }

        initials[0] = (char)EEPROM.read(address + (5*i) + 2);
        initials[1] = (char)EEPROM.read(address + (5*i) + 3);
        initials[2] = (char)EEPROM.read(address + (5*i) + 4);

        if (score > 0)
        {
            sprintf(text, "%c%c%c %u", initials[0], initials[1], initials[2], score);
            arduboy.setCursor(x + 24, y + (i*8));
            arduboy.print(text);
            arduboy.display();
        }
    }
    arduboy.display();
}

/**
 * @brief Generate a random star for the background.
 */
void generateStar()
{
    int rand = random(1, 100);
    
    // Make sure there are not too many stars.
    if(rand<10 && numberOfStars < 20)
    {
        star s;
        s.x = SCREEN_WIDTH-1;
        // Create a y-coordinate between bottom and top.
        s.y = random(0, SCREEN_HEIGHT);;
        stars[numberOfStars] = s;
        numberOfStars++;
    }
}

/**
 * @brief Draw all stars in the background.
 */
void drawStars()
{
    for(int i=0; i<numberOfStars; i++)
    {
        arduboy.drawPixel(stars[i].x, stars[i].y, 1);
    }
}

/**
 * @brief Generate random enemies with their strength based on the player's
 * score and the number of enemies on the screen. May generate nothing.
 * You may use this every second.
 * TODO: Get the size of the enemy.
 */
void generateEnemy()
{
    if(random(1, 9) > numberOfEnemies)
    {
        Enemy e;
        e.x = SCREEN_WIDTH-1;
        e.y = random(0, SCREEN_HEIGHT-1);
        e.type = random(0, 8);
        e.lifepoints = random(1+player.destroyed_ships, 10+player.destroyed_ships);
        e.alive = true;
        e.movement = random(0, 8);
        if(random(0, 100) > 95)
        {
            e.supply = true;
        }
        enemies[numberOfEnemies] = e;
        numberOfEnemies++;
    }
}

/**
 * @brief draw all the enemies.
 * TODO: Get the size of each image right.
 */
void drawEnemies()
{
    for(int i=0; i<numberOfEnemies; i++)
    {
        if(enemies[i].type < 2)
        {
            arduboy.drawBitmap(0, 0, enemy_00, 128, 64, 1);
        } else if(enemies[i].type < 4)
        {
            arduboy.drawBitmap(0, 0, enemy_01, 128, 64, 1);
        } else if(enemies[i].type < 8)
        {
            arduboy.drawBitmap(0, 0, enemy_02, 128, 64, 1);
        } else if(enemies[i].type < 16)
        {
            arduboy.drawBitmap(0, 0, enemy_03, 128, 64, 1);
        } else if(enemies[i].type < 32)
        {
            arduboy.drawBitmap(0, 0, enemy_04, 128, 64, 1);
        } else if(enemies[i].type < 64)
        {
            arduboy.drawBitmap(0, 0, enemy_05, 128, 64, 1);
        } else if(enemies[i].type < 128)
        {
            arduboy.drawBitmap(0, 0, enemy_06, 128, 64, 1);
        } else if(enemies[i].type < 256)
        {
            arduboy.drawBitmap(0, 0, enemy_07, 128, 64, 1);
        }
    }
    arduboy.display();
}

/**
 * @brief Create supply
 */

/**
 * @brief Draw supply
 */

/**
 * @brief Supplies move
 */

/**
 * @brief Bullets move
 */

/**
 * @brief Player moves
 */

/**
 * @brief Enemies move
 */

/**
 * @brief Stars move
 */

/**
 * @brief Enemies shoot
 */

/**
 * @brief Player shoots
 */

/**
 * @brief Player: Check collision.
 */

/**
 * @brief Enemy: Check shot.
 */

/**
 * @brief Draw score
 */

/**
 * @brief Pause
 */

/**
 * @brief draw Pause
 */

/**
 * @brief Prints 'Hello World!', shows the intro and inits the seed for random
 * numbers.
 */
void setup()
{
    arduboy.start();
    arduboy.setFrameRate(60);
    arduboy.print("Hello World!");
    arduboy.display();
    showIntro();
    arduboy.initRandomSeed();
}

void loop
{

}