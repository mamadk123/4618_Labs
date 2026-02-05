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

    if (button_val == 0)
    {
        if (_press_start < 0.0)
        {
            _press_start = now;
        }
        else
        {
            if ((now - _press_start >= DEBOUNCE_TIME) &&
                (_counted_time < _press_start))
            {
                _counted_time = now;
                return true;
            }
        }
    }
    else
    {
        _press_start = -1.0;
    }

    return false;
}