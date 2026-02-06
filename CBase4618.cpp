#include "stdafx.h"
#include "CBase4618.h"
#include <opencv2/highgui.hpp>

CBase4618::CBase4618(){
    _exit = false;
}

CBase4618::~CBase4618() {
}

void CBase4618::run()
{

    while (!_exit)
    {
        gpio();
        update();
        draw();

        char key = cv::waitKey(1); // only time wait key is allowed ;)
        if (key == 'q' || key == 'Q')
            _exit = true;
    }
    cv::destroyAllWindows();
}