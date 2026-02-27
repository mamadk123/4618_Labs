#include "stdafx.h"
#include "CAsteroidGame.h"
#include <cmath>
#include "cvui.h"

#define JOYSTICK_Y 26
#define JOYSTICK_X 2
#define JOY_DEADZONE 5.0
#define BUTTON_S1 33
#define BUTTON_S2  32
#define BULLET_COOLDOWN 0.025
CAsteroidGame::CAsteroidGame(cv::Size size, int comport)
{
    _control.init_com(comport);

    cv::namedWindow(_window_name);
    //CVUI
    cvui::init(_window_name);
    //canvas
    _canvas = cv::Mat::zeros(size, CV_8UC3);

    //timing
    _last_time = (double)cv::getTickCount() / cv::getTickFrequency();

    reset_game();
}

void CAsteroidGame::gpio()
{
    _control.get_analog_percent(JOYSTICK_X, _joy_x);
    _control.get_analog_percent(JOYSTICK_Y, _joy_y);

    if (_control.get_button_debounced(BUTTON_S2, BULLET_COOLDOWN))
        _fire_requested = true;

    if (_control.get_button_debounced(BUTTON_S1))
        _reset_requested = true;

    _micro_connected = _control.is_connected();
}

void CAsteroidGame::update()
{
    update_timing(); //update _dt
    
    if (!_micro_connected)
    {
        _control.init_com(5);
        _micro_connected = _control.is_connected();
        return;
    }

    if (_game_over)
    {
        handle_game_reset();
        return;
    }

    process_joystick(); // ship accel
    update_ship(); // movement + clamping
    
    handle_fire_request(); //makes bullet
    
    update_bullets(); // movement of bullet
    update_asteroids();

    handle_collisions();
    remove_dead_bullets(); //erase when exit screen
    remove_dead_asteroids();

    handle_game_reset(); //resets the game
}

void CAsteroidGame::draw()
{
    _canvas.setTo(Scalar(0, 0, 0));

    draw_ship();
    draw_bullets();
    draw_asteroids();
    draw_points();

    handle_micro_not_connected();
    draw_game_over();
    cv::imshow(_window_name, _canvas);
}

////////////////////////////////////
//// Function used in Class
///////////////////////////////////
void CAsteroidGame::update_timing()
{
    double now = (double)cv::getTickCount() / cv::getTickFrequency();
    _dt = now - _last_time;
    _last_time = now;
}

void CAsteroidGame::process_joystick()
{
    float centered_x = _joy_x - 50.0f;
    float centered_y = -(_joy_y - 50.0f);

    cv::Point2f accel(0, 0);

    if (abs(centered_x) > JOY_DEADZONE)
        accel.x = centered_x;

    if (abs(centered_y) > JOY_DEADZONE)
        accel.y = centered_y;

    _ship.thrust(accel * _accel_scale, _dt);
}
void CAsteroidGame::update_ship()
{
    _ship.move(_dt);
    clamp_object(_ship);
}
void CAsteroidGame::clamp_object(CGameObject& obj)
{
    cv::Point2f pos = obj.get_pos();
    int r = obj.get_radius();

    // Left wall
    if (pos.x - r < 0)
        pos.x = r;

    // Right wall
    if (pos.x + r > _canvas.cols)
        pos.x = _canvas.cols - r;

    // Top wall
    if (pos.y - r < 0)
        pos.y = r;

    // Bottom wall
    if (pos.y + r > _canvas.rows)
        pos.y = _canvas.rows - r;

    obj.set_pos(pos);
}
void CAsteroidGame::draw_ship()
{
    _ship.draw(_canvas);
    std::string lives_text = "Lives: " + std::to_string(_ship.get_lives());

    cv::putText(
        _canvas,
        lives_text,
        cv::Point(20, 60),
        cv::FONT_HERSHEY_SIMPLEX,
        0.8,
        cv::Scalar(255, 255, 255),
        2
    );
}

void CAsteroidGame::update_bullets() {
    for (auto& b : _bullets)
        b.move(_dt);
}
void CAsteroidGame::remove_dead_bullets()
{
    _bullets.erase(
        std::remove_if(_bullets.begin(), _bullets.end(),
            [&](CBullet& b)
            {
                return b.is_off_screen(_canvas.size());
            }),
        _bullets.end());
}
void CAsteroidGame::handle_fire_request()
{
    if (_fire_requested)
    {
        _bullets.push_back(
            CBullet(_ship.get_pos(), _ship.get_angle())
        );

        _fire_requested = false;
    }
}
void CAsteroidGame::draw_bullets() 
{    
    for (auto& b : _bullets)
        b.draw(_canvas);

    std::string text = "Bullets: " + std::to_string(_bullets.size());

    cv::putText(
        _canvas,
        text,
        cv::Point(20, 30),              // position
        cv::FONT_HERSHEY_SIMPLEX,
        0.8,                            // scale
        cv::Scalar(255, 255, 255),
        2
    );
}

void CAsteroidGame::update_asteroids()
{
    _asteroid_spawn_timer += _dt;

    if (_asteroid_spawn_timer >= _asteroid_spawn_interval)
    {
        spawn_asteroid();
        _asteroid_spawn_timer = 0.0;
    }

    for (auto& a : _asteroids)
    {
        a.move(_dt);
        wrap_object(a);
    }
}
void CAsteroidGame::spawn_asteroid()
{
    // Random size
    int radius = 20 + rand() % 40;   // 20 to 60

    // Random position (spawn at edge)
    int edge = rand() % 4;

    cv::Point2f pos;

    if (edge == 0)      pos = cv::Point2f(0, rand() % _canvas.rows);                // left
    else if (edge == 1) pos = cv::Point2f(_canvas.cols, rand() % _canvas.rows);     // right
    else if (edge == 2) pos = cv::Point2f(rand() % _canvas.cols, 0);                // top
    else                pos = cv::Point2f(rand() % _canvas.cols, _canvas.rows);     // bottom

    // Random direction
    float angle = (rand() % 360) * 3.14f / 180.0f;

    float speed = 50 + rand() % 150;   // 50 to 200

    cv::Point2f vel(
        speed * cos(angle),
        speed * sin(angle)
    );

    _asteroids.push_back(CAsteroid(pos, vel, radius));
}
void CAsteroidGame::wrap_object(CGameObject& obj)
{
    cv::Point2f pos = obj.get_pos();

    if (pos.x < 0)
        pos.x = _canvas.cols;
    else if (pos.x > _canvas.cols)
        pos.x = 0;

    if (pos.y < 0)
        pos.y = _canvas.rows;
    else if (pos.y > _canvas.rows)
        pos.y = 0;

    obj.set_pos(pos);
}
void CAsteroidGame::draw_asteroids()
{
    for (auto& a : _asteroids)
        a.draw(_canvas);
}

void CAsteroidGame::handle_collisions()
{
    // Bullet vs Asteroid
    for (auto& b : _bullets)
    {
        for (auto& a : _asteroids)
        {
            if (b.collide(a))
            {
                b.hit();
                a.hit();

                if (a.get_lives() <= 0)
                    _score += 10;   // 10 points per asteroid
            }
        }
    }

    // Ship vs Asteroid
    for (auto& a : _asteroids)
    {
        if (_ship.collide(a))
        {
            _ship.hit();
            a.hit();
        }

        if (_ship.get_lives() <= 0)
        {
            _ship.set_vel(Point2f(0, 0));
            _game_over = true;
        }
    }
}
void CAsteroidGame::remove_dead_asteroids()
{
    _asteroids.erase(
        std::remove_if(_asteroids.begin(), _asteroids.end(),
            [](CAsteroid& a)
            {
                return a.get_lives() <= 0;
            }),
        _asteroids.end());
}

void CAsteroidGame::handle_micro_not_connected() {
    if (!_micro_connected)
    {
        cv::putText(
            _canvas,
            "MICRO DISCONNECTED",
            cv::Point(_canvas.cols / 4, _canvas.rows / 2),
            cv::FONT_HERSHEY_SIMPLEX,
            1.2,
            cv::Scalar(0, 0, 255),
            3
        );
    }
}

void CAsteroidGame::draw_points() {
    std::string score_text = "Score: " + std::to_string(_score);

    cv::putText(
        _canvas,
        score_text,
        cv::Point(20, 90),
        cv::FONT_HERSHEY_SIMPLEX,
        0.8,
        cv::Scalar(255, 255, 255),
        2
    );
}

void CAsteroidGame::draw_game_over()
{
    if (_game_over)
    {
        cv::putText(
            _canvas,
            "GAME OVER - Press Reset",
            cv::Point(_canvas.cols / 4, _canvas.rows / 2),
            cv::FONT_HERSHEY_SIMPLEX,
            1.2,
            cv::Scalar(0, 0, 255),
            3
        );
    }
}
void CAsteroidGame::handle_game_reset()
{
    if (_reset_requested)
    {
        reset_game();
        _game_over = false;
        _reset_requested = false;
    }
}
void CAsteroidGame::reset_game()
{
    // Clear objects
    _asteroids.clear();
    _bullets.clear();

    // Reset ship
    _ship.set_pos(Point2f(_canvas.cols / 2.0f, _canvas.rows / 2.0f));
    _ship.set_vel(Point2f(0.0f, 0.0f));
    _ship.set_angle(0.0f);
    _ship.set_lives(3);
    _score = 0;
}

CAsteroidGame::~CAsteroidGame()
{
    cv::destroyWindow(_window_name);
}
