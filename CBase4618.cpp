#include "stdafx.h"
#include "CBase4618.h"
#include <opencv2/highgui.hpp>

CBase4618::CBase4618()
{
    _exit = false;
}

CBase4618::~CBase4618()
{
}

void CBase4618::run()
{

    while (!_exit)
    {
        int key = cv::waitKey(1);
        if (key == 'q' || key == 'Q')
            _exit = true;

        gpio();
        update();
        draw();

    }
}