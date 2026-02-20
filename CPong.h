#pragma once

#include "CBase4618.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <deque>

/**
 * @class CPong
 * @brief Implements the Pong game for Lab 5 using OpenCV graphics and embedded I/O.
 *
 * CPong inherits from CBase4618 and implements the required:
 *  - gpio()
 *  - update()
 *  - draw()
 *
 * The class manages:
 *  - Ball physics using time-based motion
 *  - Paddle movement using joystick input
 *  - Collision detection
 *  - Score tracking
 *  - Game over logic
 *  - Settings GUI
 *  - Frame rate measurement and 30 FPS timing
 */
class CPong : public CBase4618
{
public:

    /**
     * @brief Constructs the Pong game object.
     *
     * Initializes:
     *  - Canvas and window
     *  - Ball and paddle geometry
     *  - Timing variables
     *  - Scores and game state
     *
     * @param size Size of the OpenCV drawing canvas
     * @param comport Serial port used for embedded controller communication
     */
    CPong(cv::Size size, int comport);

    /**
     * @brief Destructor.
     *
     * Releases OpenCV window resources.
     */
    ~CPong();

    /**
     * @brief Reads user input from hardware.
     *
     * - Reads joystick analog value for paddle movement.
     * - Detects pushbutton events (settings toggle, reset).
     */
    void gpio();

    /**
     * @brief Updates the game state.
     *
     * - Computes delta time
     * - Updates ball motion
     * - Updates paddle position
     * - Checks for collisions
     * - Handles game over state
     */
    void update();

    /**
     * @brief Renders the current frame.
     *
     * Draws:
     *  - Ball
     *  - Paddles
     *  - Score
     *  - FPS
     *  - Settings panel
     *  - Game over message (if active)
     */
    void draw();

    /**
     * @brief Resets the entire game state.
     *
     * - Clears scores
     *  - Resets ball position
     *  - Clears game over flag
     */
    void reset_game();

    /**
     * @brief Resets the ball to the center of the screen.
     *
     * The ball is launched in a random direction
     * with constant speed magnitude.
     */
    void reset_ball();

private:

    /** @brief Checks collision between ball and walls. */
    void check_wall_collision();

    /** @brief Checks collision between ball and paddles. */
    void check_paddle_collision();

    /** @brief Handles toggle event for settings window. */
    void handle_settings_event();

    /**
     * @brief Updates ball position based on velocity and delta time.
     *
     * @param dt Delta time (seconds)
     */
    void update_ball(float dt);

    /** @brief Updates right paddle position using joystick input. */
    void update_right_paddle();

    /** @brief Ensures ball remains inside display boundaries. */
    void clamp_ball_inside();

    /** @brief Draws the game over message overlay. */
    void draw_game_over();

    /** @brief Draws ball, paddles, score, and FPS. */
    void draw_game();

    /** @brief Draws static UI elements such as buttons. */
    void draw_ui();

    /** @brief Draws the settings control panel. */
    void draw_settings_panel();

    // ------------------------------------------------------------------
    // Hardware and Game State
    // ------------------------------------------------------------------

    double m_joy_y_pct;        ///< Joystick vertical position (percentage 0–100)
    int m_paddle_speed;        ///< Paddle movement speed
    bool m_settings_open;      ///< True if settings panel is visible
    bool m_settings_event;     ///< Trigger flag for settings toggle
    bool m_game_over;          ///< True when a player reaches 5 points

    // ------------------------------------------------------------------
    // Rendering
    // ------------------------------------------------------------------

    cv::Size m_size;           ///< Canvas dimensions
    cv::Mat m_canvas;          ///< Frame buffer for rendering
    std::string m_window_name; ///< OpenCV window title

    // ------------------------------------------------------------------
    // Ball State
    // ------------------------------------------------------------------

    cv::Point2f m_ball_pos;    ///< Ball position (floating point precision)
    cv::Point2f m_ball_vel;    ///< Ball velocity (pixels per second)
    int m_ball_radius;         ///< Ball radius (pixels)
    int m_ball_speed;          ///< Ball speed magnitude (pixels per second)

    // ------------------------------------------------------------------
    // Paddle State
    // ------------------------------------------------------------------

    cv::Rect m_left_paddle;    ///< Left paddle rectangle
    cv::Rect m_right_paddle;   ///< Right paddle rectangle

    // ------------------------------------------------------------------
    // Score
    // ------------------------------------------------------------------

    int m_score_left;          ///< Left player score
    int m_score_right;         ///< Right player score

    // ------------------------------------------------------------------
    // Timing
    // ------------------------------------------------------------------

    double m_last_time;   ///< Last frame timestamp (seconds)
    double m_fps;         ///< Current measured FPS
    std::deque<double> m_fps_samples;
    double m_fps_sum;
};