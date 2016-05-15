#include "movement.h"

void moveGame()
{
    if(arduboy.everyXFrames(120) && player.invincible != 0)
    {
        player.invincible = player.invincible >> 1;
    }
    if(arduboy.everyXFrames(2))
        moveStars();
    
    moveSupplies();
    if(arduboy.everyXFrames(3))
        moveBullets();
    if(arduboy.everyXFrames(player.speed) && player.alive)
    {
        movePlayer();
    }
    moveEnemies();
    if(arduboy.everyXFrames(20) && player.alive)
    {
        playerShoots();
    }
    enemiesShoot();
}

void moveSupplies()
{
    for(byte i; i<noOfSupplies; i++)
    {
        supplies[i].x++;
    }
}

void moveBullets()
{
    for(byte i=0; i<numberOfBullets; i++)
    {
        switch(bullets[i].direction)
        {
            case 0:
                bullets[i].x = bullets[i].x - 1 - bullets[i].speed;
                bullets[i].y = bullets[i].y + 1 + bullets[i].speed;
                break;
                
            case 45:
                bullets[i].y = bullets[i].y + 1 + bullets[i].speed;
                break;
                
            case 90:
                 bullets[i].x = bullets[i].x + 1 + bullets[i].speed;
                bullets[i].y = bullets[i].y + 1 + bullets[i].speed;
                break;
                
            case MOVE_LEFT:
                bullets[i].x = bullets[i].x - 1 - bullets[i].speed;
                break;
                
            case 180:
                bullets[i].x = bullets[i].x - 1 - bullets[i].speed;
                bullets[i].y = bullets[i].y - 1 - bullets[i].speed;
                break;
                
            case 225:
                bullets[i].y = bullets[i].y - 1 - bullets[i].speed;
                break;
                
            case 255:
                bullets[i].x = bullets[i].x + 1 + bullets[i].speed;
                bullets[i].y = bullets[i].y - 1 - bullets[i].speed;
                break;
            // MOVE_RIGHT
            default:
                 bullets[i].x = bullets[i].x + 1 + bullets[i].speed;
        }
    }
}

void movePlayer()
{
    if(arduboy.pressed(LEFT_BUTTON) && player.x > 0)
    {
        player.x--;
    }
    if(arduboy.pressed(UP_BUTTON) && player.y > 0)
    {
        player.y--;
    }
    if(arduboy.pressed(DOWN_BUTTON) && player.y+player.height < 63)
    {
        player.y++;
    }
    if(arduboy.pressed(RIGHT_BUTTON) && player.x+player.width < 127)
    {
        player.x++;
    }
}

void moveEnemies()
{
    for(byte i=1; i<=numberOfEnemies; i++)
    {
        if(arduboy.everyXFrames(enemies[i-1].speed))
        {
        
            bool blockedUP = false;
            bool blockedDOWN = false;
            bool blockedLEFT = false;
            bool blockedRIGHT = false;
            
            // Check for collisions.
            for(byte j=1; j<=numberOfEnemies; j++)
            {
                if(j != i)
                {
                    if(enemies[j-1].x+enemies[j-1].width == enemies[i-1].x-1 &&
                       (abs(enemies[j-1].y-enemies[i-1].y) < enemies[i-1].height 
                       || abs(enemies[j-1].y-enemies[i-1].y) < enemies[j-1].height))
                    {
                        blockedLEFT = true;
                    }
                    if(enemies[j-1].x == enemies[i-1].x+1+enemies[i-1].width &&
                       (abs(enemies[j-1].y-enemies[i-1].y) < enemies[i-1].height 
                       || abs(enemies[j-1].y-enemies[i-1].y) < enemies[j-1].height))
                    {
                        blockedRIGHT = true;
                    }
                    if(enemies[j-1].y == enemies[i-1].y+1+enemies[i-1].height  &&
                       (abs(enemies[j-1].x-enemies[i-1].x) < enemies[i-1].width 
                       || abs(enemies[j-1].x-enemies[i-1].x) < enemies[j-1].width))
                    {
                        blockedDOWN = true;
                    }
                    if(enemies[j-1].y + enemies[j-1].height == enemies[i-1].y-1 &&
                       (abs(enemies[j-1].x-enemies[i-1].x) < enemies[i-1].width 
                       || abs(enemies[j-1].x-enemies[i-1].x) < enemies[j-1].width))
                    {
                        blockedUP = true;
                    }
                }
            }
            
            //TODO Alter direction of enemy
            switch(enemies[i-1].direction)
            {
                // UP and RIGHT+UP
                case MOVE_UPRIGHT:
                    if(!blockedRIGHT) enemies[i-1].x = enemies[i-1].x + 1;
                case MOVE_UP:
                    if(!blockedUP) enemies[i-1].y = enemies[i-1].y - 1;
                    break;
                    
                // UP+LEFT
                case MOVE_UPLEFT:
                    if(!blockedLEFT) enemies[i-1].x = enemies[i-1].x - 1;
                    if(!blockedUP) enemies[i-1].y = enemies[i-1].y - 1;
                    break;
                    
                // Left and LEFT+DOWN
                case MOVE_DOWNLEFT:
                    if(!blockedDOWN) enemies[i-1].y = enemies[i-1].y + 1;
                case MOVE_LEFT:
                    if(!blockedLEFT) enemies[i-1].x = enemies[i-1].x - 1;
                    break;
                    
                // DOWN and DOWN+RIGHT
                case MOVE_DOWNRIGHT:
                    if(!blockedRIGHT) enemies[i-1].x = enemies[i-1].x + 1;
                case MOVE_DOWN:
                    if(blockedDOWN) enemies[i-1].y = enemies[i-1].y + 1;
                    break;
                    
                default:
                    if(!blockedLEFT) enemies[i-1].x = enemies[i-1].x - 1;
            }
        }
    }
}

void moveStars()
{
    for(byte i=0; i<numberOfStars; i++)
    {
        stars[i].x--;
    }
}

void enemiesShoot()
{
    byte i = 1;

    while(numberOfBullets<MAXBULLETS && i<numberOfEnemies)
    {
        if(enemies[i-1].shipType < 2 && arduboy.everyXFrames(120) && random(0,100) > 60)
        {
            Bullet b;
            // Shoot the bullet up left from the enemy.
            b.x = enemies[i-1].x + 1;
            b.y = enemies[i-1].y + (enemies[i-1].height >> 1);
            // Height and width depend on appearance which correlates to the
            // bitmap.
            b.appearance = 1;
            b.height = 2;
            b.width = 2;
            // There are no lifepoints for the player. Every hit is a kill.
            b.damage = 0;
            b.speed = 0;
            b.alive = true;
            b.playersBullet = false;
            b.direction = MOVE_LEFT;
            bullets[numberOfBullets] = b;
            numberOfBullets++;
        } else if(enemies[i-1].shipType < 4 && arduboy.everyXFrames(60) && random(0,100) > 70)
        {
            Bullet b;
            // Shoot the bullet up left from the enemy.
            b.x = enemies[i-1].x + 1;
            b.y = enemies[i-1].y + 1;
            // Height and width depend on appearance which correlates to the
            // bitmap.
            b.appearance = 1;
            b.height = 2;
            b.width = 2;
            // There are no lifepoints for the player. Every hit is a kill.
            b.damage = 0;
            b.speed = 0;
            b.alive = true;
            b.playersBullet = false;
            b.direction = MOVE_LEFT;
            bullets[numberOfBullets] = b;
            numberOfBullets++;
        } else if(enemies[i-1].shipType < 8 && arduboy.everyXFrames(240) && random(0,100) > 50)
        {
            Bullet b;
            // Shoot the bullet up left from the enemy.
            b.x = enemies[i-1].x + 1;
            b.y = enemies[i-1].y + 1;
            // Height and width depend on appearance which correlates to the
            // bitmap.
            b.appearance = 1;
            b.height = 2;
            b.width = 2;
            // There are no lifepoints for the player. Every hit is a kill.
            b.damage = 0;
            b.speed = 0;
            b.alive = true;
            b.playersBullet = false;
            b.direction = MOVE_LEFT;
            bullets[numberOfBullets] = b;
            numberOfBullets++;
        } else if(enemies[i-1].shipType < 16)
        {

        } else if(enemies[i-1].shipType < 32)
        {

        } else if(enemies[i-1].shipType < 64)
        {

        } else if(enemies[i-1].shipType < 128)
        {

        } else if(enemies[i-1].shipType < 256)
        {
            
        }
        i++;
    }
}

void playerShoots()
{
    if(arduboy.pressed(B_BUTTON))
    {
        if(MAXBULLETS > numberOfBullets 
            && player.bullets < player.maxBullets)
        {
            Bullet b;
            // A bullet should appear at front of the ship in the middle.
            b.x = player.x + 1;
            b.y = player.y + (player.height >> 1);
            b.appearance = player.bulletType;
            b.damage = player.bulletType;
            b.speed = player.bulletSpeed;
            b.alive = true;
            b.playersBullet = true;
            b.direction = MOVE_RIGHT;
            player.bullets++;
            bullets[numberOfBullets] = b;
            numberOfBullets++;
        }
    }
}