#pragma once

#include "CBase4618.h"
#include <opencv2/core.hpp>

/**
 * @file CSketch.h
 * @brief Etch-A-Sketch application class for ELEX 4618 Lab 4.
 *
 * CSketch implements the application-specific behavior for the Etch-A-Sketch
 * lab by overriding the update and draw methods defined in CBase4618.
 */

 /**
  * @class CSketch
  * @brief Etch-A-Sketch application derived from CBase4618.
  *
  * This class handles user interaction, drawing logic, and application state
  * for the Etch-A-Sketch program. Hardware access is performed through the
  * inherited CControl object.
  */
class CSketch : public CBase4618
{
private:
    cv::Point _current_pos;  ///< Current cursor position on the canvas
    cv::Point _prev_pos;     ///< Previous cursor position (for line drawing)

    int  _color_index;       ///< Index of the current drawing color
    double _joy_x_pct;    ///< Joystick X position in percent (0–100)
    double _joy_y_pct;    ///< Joystick Y position in percent (0–100)
    bool   _color_change_event; ///< True when a debounced button press is detected
    bool _reset_event;    ///< Clear canvas request
    double _last_shake_time;   ///< Last shake timestamp

public:
    /**
     * @brief Constructs a CSketch object.
     *
     * Initializes the drawing canvas and internal state.
     *
     * @param canvas_size Size of the drawing canvas in pixels
     * @param comport COM port number used for hardware communication
     */
    CSketch(const cv::Size& canvas_size, int comport);

    /**
     * @brief Destroys the CSketch object and performs hardware cleanup.
     *
     * This destructor ensures the embedded system is left in a known safe state
     * when the application exits. All RGB LEDs controlled by the sketch are turned
     * off before the object is destroyed.
     *
     * The destructor is called automatically when the application terminates
     * or when the CSketch object goes out of scope.
     */
    ~CSketch();
    /**
     * @brief Performs hardware input and output operations for the Etch-A-Sketch application.
     *
     * This method reads joystick and button inputs using the CControl interface
     * and performs any required hardware output actions such as LED updates.
     * The results of these operations are stored in member variables for use
     * by the update method.
     */
    void gpio(); //Override

    /**
     * @brief Updates application state and reads inputs.
     *
     * Reads joystick and button inputs and updates internal state.
     */
    void update(); //OVERRIDE

    /**
     * @brief Draws the current application state to the canvas.
     *
     * Renders the canvas contents and any GUI elements.
     */
    void draw();  //OVERRIDE

    /**
     * @brief Updates the BoosterPack RGB LEDs to match the current drawing color.
     *
     * This function sets the state of the on-board RGB LEDs based on the current
     * value of `_color_index`. Only the LEDs corresponding to the active drawing
     * color are turned on; all others are turned off.
     *
     * Color mapping:
     * - Index 0: Green
     * - Index 1: Red
     * - Index 2: Blue
     * - Index 3: Yellow (Red + Green)
     * - Index 4: Magenta (Red + Blue)
     *
     * This function performs hardware output only and does not modify application
     * state. It is typically called after a color change event.
     */
    void set_led_for_color();
};