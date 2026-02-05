#include "stdafx.h"
#include "CSketch.h"
#include <opencv2/highgui.hpp>

#define WINDOW_NAME "Etch-A-Sketch"

CSketch::CSketch(const cv::Size& canvas_size, int comport)
{
    _control.init_com(comport);

    _canvas = cv::Mat::zeros(canvas_size, CV_8UC3);     // Initialize canvas (color image)

    _current_pos = cv::Point(canvas_size.width / 2, canvas_size.height / 2);
    _prev_pos = _current_pos;

    _color_index = 0;
    _reset = false;
    _exit = false;

    cv::namedWindow(WINDOW_NAME);     // Create display window
}


void CSketch::update()
{
    
}

void CSketch::draw()
{
    cv::imshow(WINDOW_NAME, _canvas);
}