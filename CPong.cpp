#include "stdafx.h"
#include "CPong.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "cvui.h"

#define JOYSTICK_Y 26
#define JOY_DEADZONE 5.0
#define BUTTON_S1 33
#define BUTTON_S2  32

void CPong::gpio()
{
	_control.get_analog_percent(JOYSTICK_Y, m_joy_y_pct);

	if (_control.get_button_debounced(BUTTON_S1))
		m_settings_event = true;
	if (_control.get_button_debounced(BUTTON_S2))
		reset_game();
}

void CPong::update()
{
	float dt = 1.0f / 33.0f;

	handle_settings_event();

		if (!m_settings_open && !m_game_over)
		{
			update_ball((float)dt);
			update_right_paddle();
			clamp_ball_inside();
			check_wall_collision();
			check_paddle_collision();
		}
}

void CPong::draw()
{
	////////////////////////////////////////////////////////
	double now = cv::getTickCount() / cv::getTickFrequency();
	double frame_time = now - m_last_time;

	if (frame_time > 0)
	{
		double current_fps = 1.0 / frame_time;

		// Add new sample
		m_fps_samples.push_back(current_fps);
		m_fps_sum += current_fps;

		// If more than 100 samples, remove oldest
		if (m_fps_samples.size() > 100)
		{
			m_fps_sum -= m_fps_samples.front();
			m_fps_samples.pop_front();
		}

		// Average
		m_fps = m_fps_sum / m_fps_samples.size();
	}

	m_last_time = now;
	////////////////////////////////////////////
	m_canvas.setTo(cv::Scalar(0, 0, 0));

	draw_game();
	draw_ui();

	if (m_settings_open)
		draw_settings_panel();
	if (m_game_over)
		draw_game_over();

	cvui::update();
	cv::imshow(m_window_name, m_canvas);

	// ----- Cap to 30 FPS -----
	double frame_end = cv::getTickCount() / cv::getTickFrequency();
	double elapsed = frame_end - now;

	double target = 1.0 / 150.0;

	if (elapsed < target)
	{
		int delay = (int)((target - elapsed) * 1000.0);
		cv::waitKey(delay);
	}
}

CPong::CPong(cv::Size size, int comport)
{
	m_size = size;
	_control.init_com(comport);

	// Rand set up
	srand((unsigned int)time(NULL));

	// joystick
	m_joy_y_pct = 50.0;

	// window 
	m_window_name = "Lab 5 Pong";
	cv::namedWindow(m_window_name);

	//CVUI
	cvui::init(m_window_name);

	//canvas
	m_canvas = cv::Mat::zeros(m_size, CV_8UC3);

	// Ball
	m_ball_radius = 20;
	m_ball_speed = 200;
	m_ball_vel = cv::Point2f(200.0f, 0.0f);

	//Paddle  
	int paddle_w = 20;
	int paddle_h = 150;
	m_paddle_speed = 8;

	// setting windows
	m_settings_open = false;

	// cv::Rect(x, y, width, height)
	m_left_paddle = cv::Rect( 40, (m_size.height - paddle_h) / 2, paddle_w, paddle_h );
	m_right_paddle = cv::Rect( m_size.width - 40 - paddle_w, (m_size.height - paddle_h) / 2, paddle_w, paddle_h );

	//events
	m_settings_event = false;
	m_game_over = false;

	//timing
	m_last_time = cv::getTickCount() / cv::getTickFrequency();
	m_fps = 0;
	m_fps_sum = 0.0;
	
	reset_game();
}

CPong::~CPong()
{
	cv::destroyWindow(m_window_name);
}

void CPong::reset_ball()
{
	// Center ball
	m_ball_pos = cv::Point2f(m_size.width / 2.0f, m_size.height / 2.0f);

	float component = m_ball_speed / std::sqrt(2.0f);

	// Random signs
	int x_dir = (rand() % 2 == 0) ? 1 : -1;
	int y_dir = (rand() % 2 == 0) ? 1 : -1;

	m_ball_vel.x = x_dir * component;
	m_ball_vel.y = y_dir * component;
}
void CPong::reset_game()
{
	// Scores
	m_score_left = 0;
	m_score_right = 0;

	m_game_over = false;

	reset_ball();
}
void CPong::check_wall_collision()
{
	// Top wall
	if (m_ball_pos.y - m_ball_radius <= 0)
	{
		m_ball_pos.y = (float)m_ball_radius;
		m_ball_vel.y *= -1.0f;
	}

	// Bottom wall
	if (m_ball_pos.y + m_ball_radius >= m_size.height)
	{
		m_ball_pos.y = (float)(m_size.height - m_ball_radius);
		m_ball_vel.y *= -1.0f;
	}

	// Left wall → right player scores
	if (m_ball_pos.x - m_ball_radius <= 0)
	{
		m_score_right++;

		if (m_score_right >= 5)
		{
			m_game_over = true;
		}
		else
		{
			reset_ball();
		}
	}

	// Right wall → left player scores
	if (m_ball_pos.x + m_ball_radius >= m_size.width)
	{
		m_score_left++;

		if (m_score_left >= 5)
		{
			m_game_over = true;
		}
		else
		{
			reset_ball();
		}
	}
}
void CPong::check_paddle_collision() {
	// Left paddle
	if (m_ball_pos.x - m_ball_radius <= m_left_paddle.x + m_left_paddle.width &&
		m_ball_pos.y >= m_left_paddle.y &&
		m_ball_pos.y <= m_left_paddle.y + m_left_paddle.height &&
		m_ball_vel.x < 0)
	{
		m_ball_pos.x = m_left_paddle.x + m_left_paddle.width + m_ball_radius;
		m_ball_vel.x *= -1.0f;
	}

	// Right paddle
	if (m_ball_pos.x + m_ball_radius >= m_right_paddle.x &&
		m_ball_pos.y >= m_right_paddle.y &&
		m_ball_pos.y <= m_right_paddle.y + m_right_paddle.height &&
		m_ball_vel.x > 0)
	{
		m_ball_pos.x = m_right_paddle.x - m_ball_radius;
		m_ball_vel.x *= -1.0f;
	}
}
void CPong::handle_settings_event()
{
	if (m_settings_event)
	{
		m_settings_open = !m_settings_open;
		m_settings_event = false;
	}
}
void CPong::update_ball(float dt)
{
	float mag = std::sqrt(m_ball_vel.x * m_ball_vel.x + m_ball_vel.y * m_ball_vel.y);

	if (mag > 0.0f)
	{
		m_ball_vel.x = (m_ball_vel.x / mag) * m_ball_speed;
		m_ball_vel.y = (m_ball_vel.y / mag) * m_ball_speed;
	}

	m_ball_pos.x += m_ball_vel.x * dt;
	m_ball_pos.y += m_ball_vel.y * dt;
}
void CPong::update_right_paddle()
{
	double dy = m_joy_y_pct - 50.0;

	if (std::abs(dy) < JOY_DEADZONE)
		dy = 0;

	dy = (dy / 50.0) * m_paddle_speed;

	m_right_paddle.y -= static_cast<int>(dy);

	if (m_right_paddle.y < 0)
		m_right_paddle.y = 0;
	else if (m_right_paddle.y + m_right_paddle.height > m_size.height)
		m_right_paddle.y = m_size.height - m_right_paddle.height;
}
void CPong::clamp_ball_inside()
{
	if (m_ball_pos.x - m_ball_radius < 0)
		m_ball_pos.x = (float)m_ball_radius;

	if (m_ball_pos.x + m_ball_radius > m_size.width)
		m_ball_pos.x = (float)(m_size.width - m_ball_radius);

	if (m_ball_pos.y - m_ball_radius < 0)
		m_ball_pos.y = (float)m_ball_radius;

	if (m_ball_pos.y + m_ball_radius > m_size.height)
		m_ball_pos.y = (float)(m_size.height - m_ball_radius);
}
void CPong::draw_game()
{
	cv::rectangle(m_canvas, m_left_paddle, cv::Scalar(255, 255, 255), -1);
	cv::rectangle(m_canvas, m_right_paddle, cv::Scalar(255, 255, 255), -1);

	cv::circle(m_canvas,
		cv::Point((int)m_ball_pos.x, (int)m_ball_pos.y),
		m_ball_radius,
		cv::Scalar(255, 255, 255),
		-1);
}
void CPong::draw_ui()
{
	std::string score = std::to_string(m_score_left) + " : " + std::to_string(m_score_right);
	cv::putText(m_canvas, score,
		cv::Point(m_size.width / 2 - 40, 50),
		cv::FONT_HERSHEY_SIMPLEX,
		1.0,
		cv::Scalar(255, 255, 255),
		2);

	std::string fps_text = "FPS: " + std::to_string((int)m_fps);
	cv::putText(m_canvas, fps_text,
		cv::Point(20, 40),
		cv::FONT_HERSHEY_SIMPLEX,
		1.0,
		cv::Scalar(255, 255, 255),
		2);

	if (cvui::button(m_canvas, m_size.width - 120, 10, 110, 35, "SETTINGS"))
		m_settings_event = true;
}
void CPong::draw_settings_panel()
{
	int panel_w = 450;
	int panel_h = 330;

	int px = (m_size.width - panel_w) / 2;
	int py = (m_size.height - panel_h) / 2;

	int margin_top = 30;
	int spacing = 80;

	cvui::window(m_canvas, px, py, panel_w, panel_h, "Settings");

	int y = py + margin_top;

	// Ball Radius
	cvui::text(m_canvas, px + 30, y, "Ball Radius");
	cvui::trackbar(m_canvas, px + 30, y + 20, 380, &m_ball_radius, 5, 100);

	y += spacing;

	// Ball Speed
	cvui::text(m_canvas, px + 30, y, "Ball Speed");
	cvui::trackbar(m_canvas, px + 30, y + 20, 380, &m_ball_speed, 1, 400);

	y += spacing;

	// Paddle Speed
	cvui::text(m_canvas, px + 30, y, "Paddle Speed");
	cvui::trackbar(m_canvas, px + 30, y + 20, 380, &m_paddle_speed, 1, 20);

	if (cvui::button(m_canvas, px + 110, py + 270, 100, 30, "CLOSE"))
		m_settings_open = false;

	if (cvui::button(m_canvas, px + 230, py + 270, 100, 30, "EXIT"))
		_exit = true;
}
void CPong::draw_game_over()
{
	std::string msg1 = "GAME OVER";
	std::string msg2 = "Press RESET";

	int x1 = m_size.width / 2 - 170;
	int y1 = m_size.height / 2;

	int x2 = m_size.width / 2 - 140;
	int y2 = m_size.height / 2 + 60;

	cv::putText(m_canvas,
		msg1,
		cv::Point(x1, y1),
		cv::FONT_HERSHEY_SIMPLEX,
		2.0,
		cv::Scalar(0, 0, 255),
		4);

	cv::putText(m_canvas,
		msg2,
		cv::Point(x2, y2),
		cv::FONT_HERSHEY_SIMPLEX,
		1.0,
		cv::Scalar(255, 255, 255),
		2);
}