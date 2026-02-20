#pragma once

#include "CBase4618.h"
#include <opencv2/opencv.hpp>
#include <string>

/**
 * @class CPong
 * @brief Implements the Pong game for Lab 5 using OpenCV graphics.
 *
 * CPong inherits from CBase4618 and implements the required
 * gpio(), update(), and draw() methods.
 *
 * The class manages:
 *  - Ball position and velocity
 *  - Player and AI paddles
 *  - Score tracking
 *  - Frame rate measurement
 *  - Timing using OpenCV tick counters
 */

class CPong : public CBase4618
{
public:
        /**
     * @brief Constructs the CPong game object.
     *
     * Initializes the canvas, ball, paddles, scores,
     * timing variables, and trackbar values.
     *
     * @param size Size of the OpenCV drawing canvas
     * @param scale Display scaling factor
     */
    CPong(cv::Size size, int scale);

    /**
     * @brief Destroys the CPong object.
     */
    ~CPong();

    /**
     * @brief Reads user input from hardware or GUI.
     *
     */
    void gpio();

    /**
     * @brief Updates game state.
     *
     * Computes delta time, updates ball position,
     * handles animation timing and frame rate measurement.
     */
    void update();

    /**
     * @brief Renders the current game frame.
     *
     * Draws paddles, ball, score, and FPS to the canvas.
     */
    void draw();

    void CPong::reset_game();

    void CPong::reset_ball();

private:

    void CPong::check_wall_collision();
    void CPong::check_paddle_collision();

    cv::Size m_size;            ///< Size of the game canvas
    int m_scale;                ///< Display scaling factor

    cv::Mat m_canvas;           ///< Image buffer used for drawing
    std::string m_window_name;          ///< Window title string

    cv::Point2f m_ball_pos;     ///< Current ball position (subpixel precision)
    cv::Point2f m_ball_vel;     ///< Current ball velocity (pixels per second)

    cv::Rect m_left_paddle;     ///< Rectangle defining left paddle
    cv::Rect m_right_paddle;    ///< Rectangle defining right paddle

    int m_score_left;           ///< Left player score
    int m_score_right;          ///< Right player score

    int m_ball_radius;          ///< Radius of the ball (pixels)
    int m_ball_speed;           ///< Ball speed magnitude (pixels per second)

    float m_fps;                ///< Measured frames per second

    int64 m_last_tick;          ///< Previous OpenCV tick count used for delta time calculation

};