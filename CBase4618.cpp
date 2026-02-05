#include "stdafx.h"
#include "CBase4618.h"
#include <opencv2/highgui.hpp>

CBase4618::CBase4618(){
}

CBase4618::~CBase4618() {
}

void CBase4618::run()
{
    char key = 0;

    while (key != 'q')
    {
        update();
        draw();

        key = cv::waitKey(1); // only time wait key is allowed ;)
    }
    cv::destroyAllWindows();
}