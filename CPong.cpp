#include "stdafx.h"
#include "CPong.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

void CPong::gpio(){}

void CPong::update()
{
	int64 now = cv::getTickCount();
	float dt = (float)(now - m_last_tick) / (float)cv::getTickFrequency();
	m_last_tick = now;

	// FPS
	if (dt > 0.0f)
		m_fps = 1.0f / dt;

	// ensuring the velocity matches trackbar
	float mag = std::sqrt(m_ball_vel.x * m_ball_vel.x + m_ball_vel.y * m_ball_vel.y);
	if (mag > 0.0f)
	{
		m_ball_vel.x = (m_ball_vel.x / mag) * m_ball_speed;
		m_ball_vel.y = (m_ball_vel.y / mag) * m_ball_speed;
	}

	//ball
	m_ball_pos.x += m_ball_vel.x * dt;
	m_ball_pos.y += m_ball_vel.y * dt;

	check_wall_collision();
	check_paddle_collision();

}

void CPong::draw()
{
	// clear canvas
	m_canvas.setTo(cv::Scalar(0, 0, 0));

	// draw paddles
	cv::rectangle(m_canvas, m_left_paddle, cv::Scalar(255, 255, 255), -1); //-1 means filled
	cv::rectangle(m_canvas, m_right_paddle, cv::Scalar(255, 255, 255), -1);

	// draw ball
	cv::circle(m_canvas, cv::Point((int)m_ball_pos.x, (int)m_ball_pos.y), m_ball_radius, cv::Scalar(255, 255, 255), -1);

	// Scores
	std::string score = std::to_string(m_score_left) + " : " + std::to_string(m_score_right);
	cv::putText(m_canvas, score, cv::Point(m_size.width / 2 - 40, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

	// FPS
	std::string fps_text = "FPS: " + std::to_string((int)m_fps);
	cv::putText(m_canvas, fps_text, cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

	cv::imshow(m_window_name, m_canvas);
}

CPong::CPong(cv::Size size, int scale)
{
	m_size = size;
	m_scale = scale;

	// Rand set up
	srand((unsigned int)time(NULL));

	// window 
	m_window_name = "Lab 5 Pong";
	cv::namedWindow(m_window_name);

	//canvas
	m_canvas = cv::Mat::zeros(m_size, CV_8UC3);

	// Ball
	m_ball_radius = 20;
	m_ball_speed = 200;
	m_ball_vel = cv::Point2f(200.0f, 0.0f);

	//Paddle  
	int paddle_w = 20;
	int paddle_h = 450;
	// cv::Rect(x, y, width, height)
	m_left_paddle = cv::Rect( 40, (m_size.height - paddle_h) / 2, paddle_w, paddle_h );
	m_right_paddle = cv::Rect( m_size.width - 40 - paddle_w, (m_size.height - paddle_h) / 2, paddle_w, paddle_h );

	// timing
	m_fps = 0.0f;
	m_last_tick = cv::getTickCount();

	//Trackbars
	cv::createTrackbar("Ball Radius", m_window_name, &m_ball_radius, 100);
	cv::createTrackbar("Ball Speed", m_window_name, &m_ball_speed, 800);
	cv::setTrackbarMin("Ball Speed", m_window_name, 1);
	cv::setTrackbarMin("Ball Radius", m_window_name, 1);

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
		reset_ball();
	}

	// Right wall → left player scores
	if (m_ball_pos.x + m_ball_radius >= m_size.width)
	{
		m_score_left++;
		reset_ball();
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