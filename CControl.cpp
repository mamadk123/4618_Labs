#include "stdafx.h"
#include "CControl.h"

#include <string>
#include <sstream>
#include <opencv2/core.hpp>

CControl::CControl() {}
CControl::~CControl() {}

/////////////
// constants
/////////////
#define ADC_MAX 4095.0
#define DEBOUNCE_TIME 0.1

#define ACCEL_X 23
#define ACCEL_Y 24
#define ACCEL_Z 25

static const char ack_char = 'A';
static const char newline_char = '\n';
static const char carriage_return_char = '\r';

static const double init_flush_total_sec = 2.0; // total time allowed to flush startup junk
static const double init_flush_line_sec = 0.05;  // timeout per attempted line when flushing

static const double command_timeout_sec = 1.0;   // max time to wait for ACK reply 


static bool read_line(Serial& serial_port, std::string& out_line, double timeout_seconds)
{
    out_line.clear(); //Clear the old data

    char received_char = 0;
    double start_tick_count = cv::getTickCount();

    while (true)
    {
        double elapsed_seconds = (cv::getTickCount() - start_tick_count) / cv::getTickFrequency();

        if (elapsed_seconds > timeout_seconds) 
            return false;

        if (serial_port.read(&received_char, 1) > 0) // Read one byte if available
        {
            if (received_char == newline_char)  // Newline means end of line
                return true;

            if (received_char != carriage_return_char)  // Ignore carriage return
                out_line += received_char;
        }
    }
}

void CControl::init_com(int comport)
{
    std::string port_name = "COM" + std::to_string(comport);

    _com.open(port_name.c_str()); // open expects const char*

    std::string junk_line;
    double flush_start_tick = cv::getTickCount();

    while ((cv::getTickCount() - flush_start_tick) / cv::getTickFrequency() < init_flush_total_sec)
    {
        if (!read_line(_com, junk_line, init_flush_line_sec)) // If nothing arrives stop flushing
            break;
    }
}

bool CControl::get_data(int type, int channel, int& result)
{
    // Build "G type channel\n"
    std::stringstream tx_builder;
    tx_builder << "G " << type << " " << channel << newline_char;
    std::string tx_string = tx_builder.str();

    _com.write(tx_string.c_str(), (int)tx_string.length()); // Send to microcontroller

    double command_start_tick = cv::getTickCount();

    while (true)
    {
        double elapsed_seconds = (cv::getTickCount() - command_start_tick) / cv::getTickFrequency();

        if (elapsed_seconds > command_timeout_sec) 
            return false;

        // Read one full line
        std::string reply_line;
        double remaining_seconds = command_timeout_sec - elapsed_seconds;

        if (!read_line(_com, reply_line, remaining_seconds)) 
            return false;

        if (reply_line.empty() || reply_line[0] != ack_char) // Ignore garbage lines
            continue;

        // Parse reply: "A type channel value"
        std::stringstream rx_parser(reply_line);

        char ack = 0;
        int reply_type = 0;
        int reply_channel = 0;
        int reply_value = 0;

        rx_parser >> ack >> reply_type >> reply_channel >> reply_value;

        // Validate reply matches what we asked for
        if (ack != ack_char) 
            continue;
        if (reply_type != type) 
            continue;
        if (reply_channel != channel) 
            continue;

        result = reply_value;
        return true;
    }
}

bool CControl::set_data(int type, int channel, int val)
{
    // Build "S type channel value\n"
    std::stringstream tx_builder;
    tx_builder << "S " << type << " " << channel << " " << val << newline_char;
    std::string tx_string = tx_builder.str();

    _com.write(tx_string.c_str(), (int)tx_string.length()); // Send to microcontroller

    double command_start_tick = cv::getTickCount();

    while (true)
    {
        double elapsed_seconds = (cv::getTickCount() - command_start_tick) / cv::getTickFrequency();

        if (elapsed_seconds > command_timeout_sec) 
            return false;

        // Read one full line
        std::string reply_line;
        double remaining_seconds = command_timeout_sec - elapsed_seconds;

        if (!read_line(_com, reply_line, remaining_seconds)) 
            return false;

        if (reply_line.empty() || reply_line[0] != ack_char) // Ignore garbage lines
            continue;

        // Parse reply: "A type channel value"
        std::stringstream rx_parser(reply_line);

        char ack = 0;
        int reply_type = 0;
        int reply_channel = 0;
        int reply_value = 0;

        rx_parser >> ack >> reply_type >> reply_channel >> reply_value;

        // Validate reply matches what we set
        if (ack != ack_char) 
            continue;
        if (reply_type != type) 
            continue;
        if (reply_channel != channel) 
            continue;

        return true;
    }
}

bool CControl::get_analog_percent(int channel, double& percent)
{
    int raw = 0;
    if (!get_data(ANALOG, channel, raw))
        return false;

    percent = (raw / ADC_MAX) * 100.0;
    return true;
}

bool CControl::get_button_debounced(int channel)
{
    int button_val = 1;
    if (!get_data(DIGITAL, channel, button_val))
        return false;

    double now = cv::getTickCount() / cv::getTickFrequency();

    // Button pressed (active low)
    if (button_val == 0)
    {
        // First time seeing this press
        if (_press_start[channel] <= 0.0)
        {
            _press_start[channel] = now;
        }
        else
        {
            // Debounce time passed and not yet counted
            if ((now - _press_start[channel] >= DEBOUNCE_TIME) &&
                (_counted_time[channel] < _press_start[channel]))
            {
                _counted_time[channel] = now;
                return true;   // ONE clean press event
            }
        }
    }
    else
    {
        // Button released → reset press timer
        _press_start[channel] = -1.0;
    }

    return false;
}

bool CControl::get_accel(double& ax, double& ay, double& az)
{

    double x_pct, y_pct, z_pct; // percentage values

    if (!get_analog_percent(ACCEL_X, x_pct))
        return false;
    if (!get_analog_percent(ACCEL_Y, y_pct))
        return false;
    if (!get_analog_percent(ACCEL_Z, z_pct))
        return false;

    ax = (x_pct - 50.0) / 50.0; //converts percentage to -1 to 1
    ay = (y_pct - 50.0) / 50.0;
    az = (z_pct - 50.0) / 50.0;
    return true;
}