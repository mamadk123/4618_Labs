#include "stdafx.h"
#include "CPong.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "cvui.h"
#include <thread>

#define JOYSTICK_Y 26
#define JOY_DEADZONE 5.0
#define BUTTON_S1 33
#define BUTTON_S2  32

void CPong::gpio()
{
	_control.get_analog_percent(JOYSTICK_Y, _joy_y_pct);

	if (_control.get_button_debounced(BUTTON_S1))
		_settings_event = true;
	if (_control.get_button_debounced(BUTTON_S2))
		reset_game();
}

void CPong::update()
{
	update_timing();
	handle_settings_event();

		if (!_settings_open && !_game_over)
		{
			update_ball((float)_target_dt);
			update_right_paddle();
			update_left_paddle();
			clamp_ball_inside();
			check_wall_collision();
			check_paddle_collision();
		}
}

void CPong::draw()
{
	_canvas.setTo(cv::Scalar(0, 0, 0));

	draw_game();
	draw_ui();

	if (_settings_open)
		draw_settings_panel();
	if (_game_over)
		draw_game_over();

	cvui::update();
	cv::imshow(_window_name, _canvas);
}

CPong::CPong(cv::Size size, int comport)
{
	_size = size;
	_control.init_com(comport);

	// Rand set up
	srand((unsigned int)time(NULL));

	// joystick
	_joy_y_pct = 50.0;

	// window 
	_window_name = "Lab 5 Pong";
	cv::namedWindow(_window_name);

	//CVUI
	cvui::init(_window_name);

	//canvas
	_canvas = cv::Mat::zeros(_size, CV_8UC3);

	// Ball
	_ball_radius = 20;
	_ball_speed = 1000;
	_ball_vel = cv::Point2f(200.0f, 0.0f);

	//Paddle  
	int paddle_w = 20;
	int paddle_h = 150;
	_paddle_speed = 15;

	// setting windows
	_settings_open = false;

	// cv::Rect(x, y, width, height)
	_left_paddle = cv::Rect( 40, (_size.height - paddle_h) / 2, paddle_w, paddle_h );
	_right_paddle = cv::Rect( _size.width - 40 - paddle_w, (_size.height - paddle_h) / 2, paddle_w, paddle_h );

	//events
	_settings_event = false;
	_game_over = false;

	//timing
	_last_time = cv::getTickCount() / cv::getTickFrequency();
	_fps = 0;
	_fps_sum = 0.0f;
	_max_samples = 100;
	_avg_fps = 0.0;
	_target_dt = 1.0f / 40.0f;
	
	reset_game();
}

CPong::~CPong()
{
	cv::destroyWindow(_window_name);
}

void CPong::update_timing()
{
	double now = (double)cv::getTickCount() / cv::getTickFrequency();
	double dt = now - _last_time;

	if (dt < _target_dt)
	{
		std::this_thread::sleep_for(
			std::chrono::duration<double>(_target_dt - dt)
		);

		now = (double)cv::getTickCount() / cv::getTickFrequency();
		dt = now - _last_time;
	}

	_last_time = now;

	_fps = 1.0 / dt;

	// Add new fps
	_fps_history.push_back(_fps);
	_fps_sum += _fps;

	// Keep only last 100
	if (_fps_history.size() > _max_samples)
	{
		_fps_sum -= _fps_history.front();
		_fps_history.erase(_fps_history.begin());
	}

	_avg_fps = _fps_sum / _fps_history.size();
}
void CPong::reset_ball()
{
	// Center ball
	_ball_pos = cv::Point2f(_size.width / 2.0f, _size.height / 2.0f);

	float component = _ball_speed / std::sqrt(2.0f);

	// Random signs
	int x_dir = (rand() % 2 == 0) ? 1 : -1;
	int y_dir = (rand() % 2 == 0) ? 1 : -1;

	_ball_vel.x = x_dir * component;
	_ball_vel.y = y_dir * component;
}
void CPong::reset_game()
{
	// Scores
	_score_left = 0;
	_score_right = 0;

	_game_over = false;

	reset_ball();
}
void CPong::check_wall_collision()
{
	// Top wall
	if (_ball_pos.y - _ball_radius <= 0)
	{
		_ball_pos.y = (float)_ball_radius;
		_ball_vel.y *= -1.0f;
	}

	// Bottom wall
	if (_ball_pos.y + _ball_radius >= _size.height)
	{
		_ball_pos.y = (float)(_size.height - _ball_radius);
		_ball_vel.y *= -1.0f;
	}

	// Left wall → right player scores
	if (_ball_pos.x - _ball_radius <= 0)
	{
		_score_right++;

		if (_score_right >= 5)
		{
			_game_over = true;
		}
		else
		{
			reset_ball();
		}
	}

	// Right wall → left player scores
	if (_ball_pos.x + _ball_radius >= _size.width)
	{
		_score_left++;

		if (_score_left >= 5)
		{
			_game_over = true;
		}
		else
		{
			reset_ball();
		}
	}
}
void CPong::check_paddle_collision() {
	// Left paddle
	if (_ball_pos.x - _ball_radius <= _left_paddle.x + _left_paddle.width &&
		_ball_pos.y >= _left_paddle.y &&
		_ball_pos.y <= _left_paddle.y + _left_paddle.height &&
		_ball_vel.x < 0)
	{
		_ball_pos.x = _left_paddle.x + _left_paddle.width + _ball_radius;
		_ball_vel.x *= -1.0f;
	}

	// Right paddle
	if (_ball_pos.x + _ball_radius >= _right_paddle.x &&
		_ball_pos.y >= _right_paddle.y &&
		_ball_pos.y <= _right_paddle.y + _right_paddle.height &&
		_ball_vel.x > 0)
	{
		_ball_pos.x = _right_paddle.x - _ball_radius;
		_ball_vel.x *= -1.0f;
	}
}
void CPong::handle_settings_event()
{
	if (_settings_event)
	{
		_settings_open = !_settings_open;
		_settings_event = false;
	}
}
void CPong::update_ball(float dt)
{
	float mag = std::sqrt(_ball_vel.x * _ball_vel.x + _ball_vel.y * _ball_vel.y);

	if (mag > 0.0f)
	{
		_ball_vel.x = (_ball_vel.x / mag) * _ball_speed;
		_ball_vel.y = (_ball_vel.y / mag) * _ball_speed;
	}

	_ball_pos.x += _ball_vel.x * dt;
	_ball_pos.y += _ball_vel.y * dt;
}
void CPong::update_right_paddle()
{
	double dy = _joy_y_pct - 50.0;

	if (std::abs(dy) < JOY_DEADZONE)
		dy = 0;

	dy = (dy / 50.0) * _paddle_speed;

	_right_paddle.y -= static_cast<int>(dy);

	if (_right_paddle.y < 0)
		_right_paddle.y = 0;
	else if (_right_paddle.y + _right_paddle.height > _size.height)
		_right_paddle.y = _size.height - _right_paddle.height;
}
void CPong::update_left_paddle() {

	_left_paddle.y = (int)(_ball_pos.y - _left_paddle.height / 2);

	if (_left_paddle.y < 0)
		_left_paddle.y = 0;
	else if (_left_paddle.y + _left_paddle.height > _size.height)
		_left_paddle.y = _size.height - _left_paddle.height;
}
void CPong::clamp_ball_inside()
{
	if (_ball_pos.x - _ball_radius < 0)
		_ball_pos.x = (float)_ball_radius;

	if (_ball_pos.x + _ball_radius > _size.width)
		_ball_pos.x = (float)(_size.width - _ball_radius);

	if (_ball_pos.y - _ball_radius < 0)
		_ball_pos.y = (float)_ball_radius;

	if (_ball_pos.y + _ball_radius > _size.height)
		_ball_pos.y = (float)(_size.height - _ball_radius);
}
void CPong::draw_game()
{
	cv::rectangle(_canvas, _left_paddle, cv::Scalar(255, 255, 255), -1);
	cv::rectangle(_canvas, _right_paddle, cv::Scalar(255, 255, 255), -1);

	cv::circle(_canvas,
		cv::Point((int)_ball_pos.x, (int)_ball_pos.y),
		_ball_radius,
		cv::Scalar(255, 255, 255),
		-1);
}
void CPong::draw_ui()
{
	std::string score = std::to_string(_score_left) + " : " + std::to_string(_score_right);
	cv::putText(_canvas, score,
		cv::Point(_size.width / 2 - 40, 50),
		cv::FONT_HERSHEY_SIMPLEX,
		1.0,
		cv::Scalar(255, 255, 255),
		2);

	std::string fps_text = "FPS: " + std::to_string((int)_avg_fps);
	cv::putText(_canvas, fps_text,
		cv::Point(20, 40),
		cv::FONT_HERSHEY_SIMPLEX,
		1.0,
		cv::Scalar(255, 255, 255),
		2);

	if (cvui::button(_canvas, _size.width - 120, 10, 110, 35, "SETTINGS"))
		_settings_event = true;
}
void CPong::draw_settings_panel()
{
	int panel_w = 450;
	int panel_h = 330;

	int px = (_size.width - panel_w) / 2;
	int py = (_size.height - panel_h) / 2;

	int margin_top = 30;
	int spacing = 80;

	cvui::window(_canvas, px, py, panel_w, panel_h, "Settings");

	int y = py + margin_top;

	// Ball Radius
	cvui::text(_canvas, px + 30, y, "Ball Radius");
	cvui::trackbar(_canvas, px + 30, y + 20, 380, &_ball_radius, 5, 100);

	y += spacing;

	// Ball Speed
	cvui::text(_canvas, px + 30, y, "Ball Speed");
	cvui::trackbar(_canvas, px + 30, y + 20, 380, &_ball_speed, 500, 1500);

	y += spacing;

	// Paddle Speed
	cvui::text(_canvas, px + 30, y, "Paddle Speed");
	cvui::trackbar(_canvas, px + 30, y + 20, 380, &_paddle_speed, 10, 30);

	if (cvui::button(_canvas, px + 110, py + 270, 100, 30, "CLOSE"))
		_settings_open = false;

	if (cvui::button(_canvas, px + 230, py + 270, 100, 30, "EXIT"))
		_exit = true;
}
void CPong::draw_game_over()
{
	std::string msg1 = "GAME OVER";
	std::string msg2 = "Press RESET";

	int x1 = _size.width / 2 - 170;
	int y1 = _size.height / 2;

	int x2 = _size.width / 2 - 140;
	int y2 = _size.height / 2 + 60;

	cv::putText(_canvas,
		msg1,
		cv::Point(x1, y1),
		cv::FONT_HERSHEY_SIMPLEX,
		2.0,
		cv::Scalar(0, 0, 255),
		4);

	cv::putText(_canvas,
		msg2,
		cv::Point(x2, y2),
		cv::FONT_HERSHEY_SIMPLEX,
		1.0,
		cv::Scalar(255, 255, 255),
		2);
}