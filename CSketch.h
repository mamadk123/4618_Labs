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
    bool _reset;             ///< Flag indicating the canvas should be cleared
    bool _exit;              ///< Flag indicating the application should exit

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
};