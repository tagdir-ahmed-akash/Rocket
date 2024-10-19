
//MD AKASH HOSSAIN
//2021521460115

#include <stdio.h>
#include <graphics.h>
#include "conio.h"
#include "EasyXPng.h"  // For PNG handling
#include <time.h>      // For timing and randomization
#include <math.h>      // For collision detection (sqrt, pow)
#include <mmsystem.h>  // For playing sound
#include <iostream>
#include <thread>
#include <cmath>

#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "Winmm.lib")

// Define M_PI if it is not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WIDTH 560
#define HEIGHT 800
#define MaxBulletNum 200  // Maximum number of bullets
#define RocketSpeed 5     // Speed of the rocket's movement
#define MaxHealth 7     // Maximum health

void HideCursor(bool Visible);
void playExplosion(); // Declare playExplosion function

class Bullet {
public:
    IMAGE im_bullet;  // Bullet image
    float x, y;       // Position coordinates
    float vx, vy;     // Velocity in x and y directions
    float radius;     // Size of the bullet (half the image width)

    // Constructor
    Bullet() : x(0), y(0), vx(0), vy(0), radius(0) {}

    // Draw the bullet on the screen
    void draw() {
        putimagePng(static_cast<int>(x - radius), static_cast<int>(y - radius), &im_bullet);
    }

    // Update the bullet's position and check for bouncing
    void update() {
        x += vx;
        y += vy;

        // Bounce off the screen edges
        if (x - radius <= 0 || x + radius >= WIDTH) {
            vx = -vx;  // Reverse x direction
        }
        if (y - radius <= 0 || y + radius >= HEIGHT) {
            vy = -vy;  // Reverse y direction
        }
    }
};

class Rocket {
public:
    IMAGE im_rocket;  // Rocket image
    float x, y;       // Position coordinates
    float width;      // Width of the rocket
    float height;     // Height of the rocket
    float radius;     // Effective radius for collision detection

    // Constructor
    Rocket() : x(0), y(0), width(0), height(0), radius(0) {}

    // Draw the rocket on the screen
    void draw() {
        putimagePng(static_cast<int>(x - width / 2), static_cast<int>(y - height / 2), &im_rocket);
    }

    // Move the rocket to the mouse position
    void update(float mouseX, float mouseY) {
        x = mouseX;
        y = mouseY;

        // Ensure the rocket stays within the screen bounds
        if (x - width / 2 < 0) x = width / 2;
        if (x + width / 2 > WIDTH) x = WIDTH - width / 2;
        if (y - height / 2 < 0) y = height / 2;
        if (y + height / 2 > HEIGHT) y = height / 2;
    }
};

class UFO {
public:
    IMAGE im_ufo;  // UFO image
    float x, y;    // Position coordinates
    float speed;   // Speed of the UFO
    float radius;  // Effective radius for collision detection

    // Constructor
    UFO() : x(0), y(0), speed(2.0), radius(0) {}

    // Draw the UFO on the screen
    void draw() {
        putimagePng(static_cast<int>(x - radius), static_cast<int>(y - radius), &im_ufo);
    }

    // Update the UFO's position to chase the rocket
    void update(float rocketX, float rocketY) {
        float dx = rocketX - x;
        float dy = rocketY - y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance > 0) {
            // Move towards the rocket
            x += speed * (dx / distance);
            y += speed * (dy / distance);
        }

        // Ensure the UFO stays within the screen bounds
        if (x < radius) x = radius;
        if (x > WIDTH - radius) x = WIDTH - radius;
        if (y < radius) y = radius;
        if (y > HEIGHT - radius) y = HEIGHT - radius;
    }
};

IMAGE im_bk;      // Background image
IMAGE im_heart;   // Heart image
IMAGE im_explode; // Explosion image
Bullet bullets[MaxBulletNum];  // Array of bullets
Rocket rocket;    // Rocket object
UFO ufo;          // UFO object
bool ufoActive = true; // Indicates if the UFO is active in the game
int bulletNum = 0; // Current number of bullets
int health = MaxHealth; // Player's health
int elapsedTime = 100; // Variable to track elapsed time in seconds
bool gameStarted = false; // Flag to check if the game has started

void startup() {
    srand(static_cast<unsigned int>(time(0))); // Seed for random number generation

    // Load images
    loadimage(&im_bk, _T("background.png"));     // Background image
    loadimage(&bullets[0].im_bullet, _T("bullet.png"));  // Bullet image (same for all bullets)
    loadimage(&rocket.im_rocket, _T("rocket.png"));  // Rocket image
    loadimage(&im_heart, _T("heart.png"));  // Heart image for health display
    loadimage(&im_explode, _T("blowup.png")); // Explosion image

    // Initialize rocket properties
    rocket.x = WIDTH / 2;
    rocket.y = HEIGHT - 100;  // Start near the bottom of the screen
    rocket.width = rocket.im_rocket.getwidth();
    rocket.height = rocket.im_rocket.getheight();
    rocket.radius = static_cast<float>((rocket.width + rocket.height) / 4);  // Average size of the rocket for collision detection

    loadimage(&ufo.im_ufo, _T("ufo.png")); // Load UFO image
    ufo.x = WIDTH / 2; // Starting position
    ufo.y = HEIGHT / 2; // Starting position
    ufo.radius = ufo.im_ufo.getwidth() / 2; // Set radius for collision detection

    // Initialize graphics window and start batch drawing
    initgraph(WIDTH, HEIGHT);
    BeginBatchDraw();
}

// Display function
void show() {
    // Draw background
    putimage(0, 0, &im_bk);

    // Draw all bullets
    for (int i = 0; i < bulletNum; i++) {
        bullets[i].draw();
    }

    // Draw the rocket
    rocket.draw();

    // Draw the UFO if active
    if (ufoActive) {
        ufo.draw();
    }

    // Display health (hearts) in the top-left corner
    for (int i = 0; i < health; i++) {
        putimagePng(10 + i * 40, 10, &im_heart);  // Display heart icons for health
    }

    // Display the timer at the top right corner
    TCHAR timerText[50]; // Buffer for the timer text
    swprintf_s(timerText, sizeof(timerText) / sizeof(TCHAR), _T("Time: %d seconds"), elapsedTime); // Format the timer text
    outtextxy(WIDTH - 150, 10, timerText); // Display timer text

    FlushBatchDraw();
    Sleep(10);
}

// Function to display the start screen
void showStartScreen() {
    setbkmode(TRANSPARENT); // Set transparent background for text
    outtextxy(WIDTH / 2 - 100, HEIGHT / 2 - 20, _T("Press ESC to Start Game!"));
    FlushBatchDraw();
}

// Update function without user input
void updateWithoutInput() {
    if (!gameStarted) return; // Skip updates if the game hasn't started

    static clock_t start = clock();     // Start time
    clock_t now = clock();              // Current time

    // Calculate elapsed seconds
    int nowSecond = 100 - static_cast<int>((now - start) / CLOCKS_PER_SEC); // Time countdown logic

    // Update elapsed time
    static clock_t lastTimerUpdate = clock(); // Last time the timer was updated
    if ((now - lastTimerUpdate) / CLOCKS_PER_SEC >= 1) {
        elapsedTime--; // Decrement elapsed time
        lastTimerUpdate = now; // Update last timer update
    }

    // Check if the timer has reached zero
    if (elapsedTime <= 0) {
        playExplosion();  // Play explosion sound

        // Show the explosion image while the sound plays
        putimage(static_cast<int>(rocket.x - rocket.width / 2), static_cast<int>(rocket.y - rocket.height / 2), &im_explode);
        outtextxy(WIDTH / 2 - 100, HEIGHT / 2, _T("Game Over! Time's up."));

        // Flush the graphics and give the explosion some time to appear on screen
        FlushBatchDraw();
        Sleep(4000);  // Keep the game window open for 4 seconds

        // Close the game window and exit
        closegraph();
        exit(0);
    }

    // Every 2 seconds, create a new bullet
    static int lastSecond = 0;
    if (nowSecond != lastSecond) {
        lastSecond = nowSecond;

        if (bulletNum < MaxBulletNum) {
            // Initialize new bullet properties
            bullets[bulletNum].x = WIDTH / 2;
            bullets[bulletNum].y = 10;

            // Generate a random angle and scalar for bullet velocity
            double angle = (rand() / static_cast<double>(RAND_MAX) - 0.5) * 0.9 * M_PI;
            double scalar = 2.0 * rand() / static_cast<double>(RAND_MAX) + 2.0;

            // Set bullet velocity
            bullets[bulletNum].vx = static_cast<float>(scalar * sin(angle));
            bullets[bulletNum].vy = static_cast<float>(scalar * cos(angle));
            bullets[bulletNum].radius = bullets[0].im_bullet.getwidth() / 2;

            // Use the same bullet image
            bullets[bulletNum].im_bullet = bullets[0].im_bullet;

            // Increase bullet count
            bulletNum++;
        }
    }

    // Update all bullets' positions
    for (int i = 0; i < bulletNum; i++) {
        bullets[i].update();
    }

    // Update UFO position to chase the rocket
    if (ufoActive) {
        ufo.update(rocket.x, rocket.y);
    }
}

// Update function with user input
void updateWithInput() {
    MOUSEMSG m;  // Mouse event message

    // Process all mouse events
    while (MouseHit()) {
        m = GetMouseMsg();  // Get mouse message

        // If the mouse moves, update the rocket position
        if (m.uMsg == WM_MOUSEMOVE) {
            rocket.update(static_cast<float>(m.x), static_cast<float>(m.y));  // Move the rocket to the mouse position
        }
    }

    // Check if ESC key is pressed to start the game
    if (GetAsyncKeyState(VK_ESCAPE)) {
        gameStarted = true; // Set the game state to started
    }
}

// Collision detection functions
bool checkCollision(Bullet& bullet, Rocket& rocket) {
    // Calculate the distance between the bullet and the rocket
    float dx = bullet.x - rocket.x;
    float dy = bullet.y - rocket.y;
    float distance = sqrt(dx * dx + dy * dy);

    // Check if the distance is less than the sum of the radii (collision)
    return distance < (bullet.radius + rocket.radius);
}

bool checkCollisionUFO(Rocket& rocket, UFO& ufo) {
    // Calculate the distance between the UFO and the rocket
    float dx = ufo.x - rocket.x;
    float dy = ufo.y - rocket.y;
    float distance = sqrt(dx * dx + dy * dy);

    // Check if the distance is less than the sum of the radii (collision)
    return distance < (ufo.radius + rocket.radius);
}

// Function to play explosion sound
void playExplosion() {
    // Ensure the previous sound is stopped before playing a new one
    mciSendString(TEXT("stop explode"), NULL, 0, NULL);

    // Open the explosion sound file
    if (mciSendString(TEXT("open \"explode.mp3\" type mpegvideo alias explode"), NULL, 0, NULL) != 0) {
        std::cout << "Error: Could not open 'explode.mp3'. Check if the file exists and the path is correct." << std::endl;
        return;
    }

    // Set volume (adjustable range: 0-1000)
    if (mciSendString(TEXT("setaudio explode volume to 500"), NULL, 0, NULL) != 0) {
        std::cout << "Error: Could not set volume for the explosion sound." << std::endl;
        return;
    }

    // Play the explosion sound asynchronously
    if (mciSendString(TEXT("play explode"), NULL, 0, NULL) != 0) {
        std::cout << "Error: Could not play 'explode.mp3'." << std::endl;
    }
}

// Close sound when exiting
void closeSound() {
    mciSendString(TEXT("close explode"), NULL, 0, NULL);
}

// Function to check collisions
void checkCollisions() {
    // Check for collisions between bullets and the rocket
    for (int i = 0; i < bulletNum; i++) {
        if (checkCollision(bullets[i], rocket)) {
            // Collision detected, reduce health
            health--;

            // If health reaches 0, trigger explosion and game over
            if (health <= 0) {
                playExplosion();  // Play explosion sound

                // Show the explosion image while the sound plays
                putimage(static_cast<int>(rocket.x - rocket.width / 2), static_cast<int>(rocket.y - rocket.height / 2), &im_explode);
                outtextxy(WIDTH / 2 - 100, HEIGHT / 2, _T("Game Over! Health depleted."));

                // Flush the graphics and give the explosion some time to appear on screen
                FlushBatchDraw();
                Sleep(4000);  // Keep the game window open for 4 seconds

                // Close the game window and exit
                closegraph();
                exit(0);
            }

            // Reset the bullet outside the screen after collision
            bullets[i].x = -100;
            bullets[i].y = -100;
        }

        // Check if bullet bypasses the rocket (out of screen)
        if (bullets[i].y > HEIGHT) {
            health--;  // Deduct health when bypassed

            if (health <= 0) {
                playExplosion();  // Play explosion sound

                // Show the explosion image while the sound plays
                putimage(static_cast<int>(rocket.x - rocket.width / 2), static_cast<int>(rocket.y - rocket.height / 2), &im_explode);
                outtextxy(WIDTH / 2 - 100, HEIGHT / 2, _T("Game Over! Health depleted."));

                // Flush the graphics and give the explosion some time to appear on screen
                FlushBatchDraw();
                Sleep(4000);  // Keep the game window open for 4 seconds

                // Close the game window and exit
                closegraph();
                exit(0);
            }

            // Reset the bullet outside the screen
            bullets[i].x = -100;
            bullets[i].y = -100;
        }
    }

    // Check for collision between the UFO and the rocket
    if (ufoActive && checkCollisionUFO(rocket, ufo)) {
        // UFO touches the rocket, deplete all health instantly
        health = 0;

        // Trigger game over sequence
        playExplosion();  // Play explosion sound

        // Show the explosion image while the sound plays
        putimage(static_cast<int>(rocket.x - rocket.width / 2), static_cast<int>(rocket.y - rocket.height / 2), &im_explode);
        outtextxy(WIDTH / 2 - 100, HEIGHT / 2, _T("Game Over! Health depleted."));

        // Flush the graphics and give the explosion some time to appear on screen
        FlushBatchDraw();
        Sleep(4000);  // Keep the game window open for 4 seconds

        // Close the game window and exit
        closegraph();
        exit(0);
    }
}

int main() {
    HideCursor(true);  // Hide console cursor
    startup();         // Initialize game resources

    // Main game loop
    while (true) {
        updateWithInput();   // Update rocket position with input

        if (!gameStarted) {
            showStartScreen(); // Show the start screen
        }
        else {
            updateWithoutInput(); // Update bullets and create new ones
            show();              // Render everything to the screen
            checkCollisions();   // Check for collisions between bullets and the rocket
        }
    }

    closeSound();  // Close sound resources
    closegraph();  // Close graphics
    return 0;      // Exit program
}

// Function to hide console cursor
void HideCursor(bool Visible) {
    CONSOLE_CURSOR_INFO Cursor;
    Cursor.bVisible = !Visible;
    Cursor.dwSize = sizeof(Cursor);
    HANDLE Hand = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorInfo(Hand, &Cursor);
}