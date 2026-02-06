#pragma once

#include "CControl.h"
#include <opencv2/core.hpp>

/**
 * @file CBase4618.h
 * @brief Abstract base class for ELEX 4618 embedded GUI applications.
 *
 * This class provides a common execution framework for all labs in the course.
 * It owns the hardware control interface and the OpenCV drawing canvas.
 * Derived classes must implement the update and draw methods.
 */

 /**
  * @class CBase4618
  * @brief Base class providing the main application loop and shared resources.
  *
  * CBase4618 defines a standard run loop that repeatedly calls update and draw.
  * The loop exits when the user presses the 'q' key.
  * Derived classes implement application-specific behavior.
  */
class CBase4618
{
protected:
    CControl _control;   ///< Hardware control interface
    cv::Mat  _canvas;    ///< OpenCV canvas used for drawing
    bool _exit;

public:
    /**
     * @brief Constructs the base class.
     */
    CBase4618();

    /**
     * @brief Virtual destructor.
     */
    ~CBase4618();


    /**
     * @brief Handles hardware input/output and communication with the embedded system.
     *
     * This method is called once per iteration of the main run loop and is intended
     * to perform all low-level GPIO, ADC, and communication-related operations.
     * The base class does not implement this method; derived classes must define
     * the specific hardware interactions required by the application.
     *
     * Typical responsibilities include:
     * - Reading analog inputs (e.g. joystick positions)
     * - Reading debounced digital inputs (e.g. buttons)
     * - Sending output commands (e.g. LEDs)
     *
     * Application-level logic and rendering must not be performed in this method.
     */
    virtual void gpio() = 0;
    
    /**
     * @brief Updates internal state and reads inputs.
     *
     * This method is implemented by the derived class.
     */
    virtual void update() = 0;

    /**
     * @brief Draws the current application state to the canvas.
     *
     * This method is implemented by the derived class.
     */
    virtual void draw() = 0;

    /**
     * @brief Runs the main application loop.
     *
     * Calls update and draw repeatedly until the user presses 'q'.
     * This is the only location where cv::waitKey is used.
     */
    void run();
};