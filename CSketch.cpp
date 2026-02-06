#include "stdafx.h"
#include "CSketch.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "cvui.h"

#define JOYSTICK_X 2
#define JOYSTICK_Y 26

#define BUTTON_S2  32
#define BUTTON_S1 33

#define LED_RED   39
#define LED_GREEN 38
#define LED_BLUE  37

#define JOY_DEADZONE 5.0      // percent
#define JOY_SPEED   5.0      // pixels per frame

#define SHAKE_THRESHOLD 1.25     // g's 
#define SHAKE_COOLDOWN  0.30

#define WINDOW_NAME "Etch-A-Sketch"

//////////////////////
/// Color constants
//////////////////////
#define NUM_COLORS 5
static const cv::Scalar DRAW_COLORS[] =  {
    cv::Scalar(0, 255, 0),     // Green
    cv::Scalar(0, 0, 255),     // Red
    cv::Scalar(255, 0, 0),     // Blue
    cv::Scalar(0, 255, 255),   // Yellow
    cv::Scalar(255, 0, 255)    // Magenta
};

CSketch::CSketch(const cv::Size& canvas_size, int comport)
{
    _control.init_com(comport);

    _canvas = cv::Mat::zeros(canvas_size, CV_8UC3);     // Initialize canvas (color image)

    cv::namedWindow(WINDOW_NAME);     // Create display window

    //GUI buttons set up
    cvui::init(WINDOW_NAME);

    _current_pos = cv::Point(canvas_size.width / 2, canvas_size.height / 2);
    _prev_pos = _current_pos;

    _color_index = 0;
    _joy_y_pct = 50;
    _joy_x_pct = 50;
    _last_shake_time = 0.0;

    _color_change_event = false;
    _reset_event = false;

    set_led_for_color();
}

void CSketch::update(){
    
    // Joystick centered at 50%
    double dx = _joy_x_pct - 50.0;
    double dy = _joy_y_pct - 50.0;

    // Deadzone
    if (std::abs(dx) < JOY_DEADZONE) 
        dx = 0;
    if (std::abs(dy) < JOY_DEADZONE) 
        dy = 0;

    // Convert to velocity
    dx = (dx / 50.0) * JOY_SPEED;
    dy = (dy / 50.0) * JOY_SPEED;

    // Store previous position
    _prev_pos = _current_pos;

    // Update position (Y IS INVERTED)
    _current_pos.x += static_cast<int>(dx);
    _current_pos.y -= static_cast<int>(dy);

    // Clamp
    if (_current_pos.x < 0)
        _current_pos.x = 0;
    else if (_current_pos.x >= _canvas.cols)
        _current_pos.x = _canvas.cols - 1;

    if (_current_pos.y < 0)
        _current_pos.y = 0;
    else if (_current_pos.y >= _canvas.rows)
    _current_pos.y = _canvas.rows - 1;

    // Draw line
    cv::line(_canvas, _prev_pos, _current_pos, DRAW_COLORS[_color_index], 2);

    // resseting the canvas
    if (_reset_event)
    {
        _canvas = cv::Mat::zeros(_canvas.size(), CV_8UC3);

        _prev_pos = _current_pos;
        _reset_event = false;
    }

    // changing color
    if (_color_change_event)
    {
        _color_index = (_color_index + 1) % NUM_COLORS;

        _color_change_event = false;
        set_led_for_color();
    }    
}

void CSketch::draw(){
    cv::Mat display = _canvas.clone();

    // Cursor
    cv::circle(display,_current_pos, 3, DRAW_COLORS[_color_index], -1);

    // GUI buttons
    if (cvui::button(display, 10, 10, 100, 30, "CLEAR"))
        _reset_event = true;

    if (cvui::button(display, 120, 10, 100, 30, "EXIT"))
        _exit = true;
    

    cvui::update(WINDOW_NAME);
    cv::imshow(WINDOW_NAME, display);
}

void CSketch::gpio() {

    _control.get_analog_percent(JOYSTICK_X, _joy_x_pct);
    _control.get_analog_percent(JOYSTICK_Y, _joy_y_pct);

    if (_control.get_button_debounced(BUTTON_S2))
        _color_change_event = true;
    
    if (_control.get_button_debounced(BUTTON_S1))
        _reset_event = true;

    double ax, ay, az;
    if (_control.get_accel(ax, ay, az))
    {
        double mag = std::sqrt(ax * ax + ay * ay + az * az);

        double now = cv::getTickCount() / cv::getTickFrequency();

        if (mag > SHAKE_THRESHOLD &&
            (now - _last_shake_time) > SHAKE_COOLDOWN)
        {
            _reset_event = true;
            _last_shake_time = now;
        }
    }

}

CSketch::~CSketch()
{
    cv::destroyAllWindows();
    // Turn off all RGB LEDs
    _control.set_data(DIGITAL, LED_RED, 0);
    _control.set_data(DIGITAL, LED_GREEN, 0);
    _control.set_data(DIGITAL, LED_BLUE, 0);
}

void CSketch::set_led_for_color()
{
    _control.set_data(DIGITAL, LED_RED, 0);
    _control.set_data(DIGITAL, LED_GREEN, 0);
    _control.set_data(DIGITAL, LED_BLUE, 0);

    if (_color_index == 0)       _control.set_data(DIGITAL, LED_GREEN, 1);
    else if (_color_index == 1)  _control.set_data(DIGITAL, LED_RED, 1);
    else if (_color_index == 2)  _control.set_data(DIGITAL, LED_BLUE, 1);
    else if (_color_index == 3)
    {
        _control.set_data(DIGITAL, LED_RED, 1);
        _control.set_data(DIGITAL, LED_GREEN, 1);
    }
    else if (_color_index == 4)
    {
        _control.set_data(DIGITAL, LED_RED, 1);
        _control.set_data(DIGITAL, LED_BLUE, 1);
    }
}