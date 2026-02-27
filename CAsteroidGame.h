#pragma once

#include "CBase4618.h"
#include "CShip.h"
#include "CAsteroid.h"
#include "CBullet.h"

#include <vector>
#include <string>

/**
 * @class CAsteroidGame
 * @brief Implements the Lab 6 Asteroids game.
 *
 * CAsteroidGame inherits from CBase4618 and manages
 * the complete gameplay loop including:
 *
 * - Microcontroller communication
 * - Player ship control
 * - Bullet firing and lifecycle
 * - Asteroid spawning and movement
 * - Collision detection
 * - Game over logic
 * - Timing and frame updates
 *
 * The class separates responsibilities into small
 * helper functions to maintain clean architecture.
 */
class CAsteroidGame : public CBase4618
{
public:

    /**
     * @brief Construct the Asteroids game object.
     *
     * Initializes:
     * - Serial communication
     * - Window and canvas
     * - Timing variables
     * - Game state
     *
     * @param size    Window resolution.
     * @param comport COM port number.
     */
    CAsteroidGame(cv::Size size, int comport);

    /**
     * @brief Destructor.
     *
     * Closes the OpenCV window.
     */
    ~CAsteroidGame();

    /**
     * @brief Read hardware inputs.
     *
     * Handles joystick and button input
     * from the microcontroller.
     */
    void gpio();

    /**
     * @brief Update game logic.
     *
     * Performs:
     * - Timing update
     * - Ship movement
     * - Bullet updates
     * - Asteroid spawning and movement
     * - Collision detection
     * - Game state transitions
     */
    void update();

    /**
     * @brief Render game objects to screen.
     *
     * Draws:
     * - Ship
     * - Bullets
     * - Asteroids
     * - UI text
     */
    void draw();

private:

    ////////////////////////
    /// Window
    ////////////////////////

    std::string _window_name = "Lab 6 Asteroid"; ///< OpenCV window name


    ////////////////////////
    /// Micro Connection
    ////////////////////////

    void handle_micro_not_connected(); ///< Handle disconnected micro state
    bool _micro_connected = true;      ///< True if microcontroller is connected


    ////////////////////////
    /// Game Over
    ////////////////////////

    bool _game_over = false; ///< True when player has no remaining lives
    void draw_game_over();   ///< Draw game over message


    ////////////////////////
    /// Timing
    ////////////////////////

    void update_timing(); ///< Update frame delta time

    double _last_time; ///< Previous frame time
    double _dt;        ///< Delta time between frames


    ////////////////////////
    /// Reset
    ////////////////////////

    void reset_game();         ///< Reset entire game state
    void handle_game_reset();  ///< Process reset request

    bool _reset_requested = false; ///< Reset button flag


    ////////////////////////
    /// Ship
    ////////////////////////

    CShip _ship; ///< Player ship object

    void process_joystick();              ///< Apply joystick acceleration
    void update_ship();                   ///< Update ship position
    void clamp_object(CGameObject& obj);  ///< Clamp object to screen bounds
    void draw_ship();                     ///< Draw ship

    double _joy_y = 0;      ///< Joystick Y value
    double _joy_x = 0;      ///< Joystick X value
    float _accel_scale = 300.0f; ///< Ship acceleration scaling factor
 
     ////////////////////////
     /// Ship
     ////////////////////////
    void draw_points(); ///< Draws player's point 
    int _score = 0;   ///< ship score

    ////////////////////////
    /// Bullets
    ////////////////////////

    std::vector<CBullet> _bullets; ///< Active bullets

    void handle_fire_request();   ///< Spawn bullet if requested
    void update_bullets();        ///< Update bullet movement
    void remove_dead_bullets();   ///< Remove bullets that are destroyed
    void draw_bullets();          ///< Draw bullets

    bool _fire_requested = false; ///< Fire button flag


    ////////////////////////
    /// Asteroids
    ////////////////////////

    std::vector<CAsteroid> _asteroids; ///< Active asteroids

    void handle_collisions();          ///< Detect and resolve collisions
    void wrap_object(CGameObject& obj);///< Wrap object around screen
    void update_asteroids();           ///< Update asteroid movement and spawning
    void spawn_asteroid();             ///< Create new asteroid
    void remove_dead_asteroids();      ///< Remove destroyed asteroids
    void draw_asteroids();             ///< Draw asteroids

    double _asteroid_spawn_timer = 0.0;    ///< Spawn timer accumulator
    double _asteroid_spawn_interval = 1.5; ///< Time between asteroid spawns
};