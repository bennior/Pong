#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

#define SIZE 20
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 30
#define UPS 30

#define LOG(x) std::cout << x << std::endl;

struct KeyBinds
{
    private:
        int up_keybind, down_keybind;
    
    public:
        KeyBinds(int _up_keybind, int _down_keybind) : up_keybind(_up_keybind), down_keybind(_down_keybind) {}
        bool up() {return sf::Keyboard::isKeyPressed((sf::Keyboard::Key) up_keybind);}
        bool down() {return sf::Keyboard::isKeyPressed((sf::Keyboard::Key) down_keybind);}
};


class Bat {
        
    private:
        int x, y;
        int dAcc = 3, velocity = 0;

        sf::RenderWindow& m_window;
        KeyBinds m_binds;

    public:
        sf::RectangleShape m_rect;

        Bat(sf::RenderWindow& _window, KeyBinds _binds, int _x) : m_window(_window), m_binds(_binds)  {
            x = _x;
            y = (SCREEN_HEIGHT - 4 * SIZE) / 2;

            m_rect = sf::RectangleShape(sf::Vector2f(SIZE, 4 * SIZE));
            m_rect.setPosition(x, y);
            m_rect.setFillColor(sf::Color::White);
        }   

        void render() {
            m_window.draw(m_rect);
        }

        void update() {

            //**check for keyboard inputs and increase or decrease velocity
            if(m_binds.up()) {
                velocity -= dAcc;    
            }

            if(m_binds.down()) {
                velocity += dAcc;
            }

            //**change y position and check for collisions with borders of screen
            if(y + velocity >= 0 && y + velocity <= SCREEN_HEIGHT - m_rect.getGlobalBounds().height) {
                y += velocity;
            }else {
                if(velocity < 0) {
                    y = 0;
                }else {
                    y = SCREEN_HEIGHT - m_rect.getGlobalBounds().height;
                }

                velocity = 0;
                
                //**could also be used instead of if statement but is less efficient
                // y = (1 + get_direction(velocity)) * SCREEN_HEIGHT / 2 - (1 + get_direction(velocity)) * m_rect.getGlobalBounds().height / 2;
            }

            //**decrease velocity by factor 0.94
            velocity *= 0.94f;

            //**update position of bat
            m_rect.setPosition(x, y);
        }
};

struct Bats
{
    public:
        sf::RectangleShape& m_bat_1;
        sf::RectangleShape& m_bat_2;    
        Bats(Bat& _bat1, Bat& _bat2) : m_bat_1(_bat1.m_rect), m_bat_2(_bat2.m_rect) {}
};

class Ball 
{
    private:
        double random_angle;
        
        sf::Vector2f m_direction;
        sf::RenderWindow& m_window;
        Bats m_bats;
        sf::RectangleShape m_ball;

    public:
        Ball(sf::RenderWindow& _window, Bats _bats) : m_window(_window), m_bats(_bats), m_ball(sf::Vector2f(SIZE, SIZE)) {
            set_random_direction();

            m_ball.setFillColor(sf::Color::White);
            m_ball.setPosition((SCREEN_WIDTH - SIZE) / 2, (SCREEN_HEIGHT - SIZE) / 2);
        }

        void render() {
            m_window.draw(m_ball);
        }

        void update() {
            //**move ball            
            m_ball.setPosition(m_ball.getPosition() + m_direction);

            //**reflect ball 
            reflect_ball();

            //**check ball's collision
            check_collision();
        }

        void check_collision() {
            //**check for collison with bat
            if(m_ball.getGlobalBounds().intersects(m_bats.m_bat_1.getGlobalBounds()))
                m_direction.x *= -1;

            if(m_ball.getGlobalBounds().intersects(m_bats.m_bat_2.getGlobalBounds()))
                m_direction.x *= -1;

            //**check for player points
            if(sf::FloatRect(0, 0, SCREEN_WIDTH / 8, SCREEN_HEIGHT).contains(m_ball.getPosition()))
                m_ball.setPosition((SCREEN_WIDTH - SIZE) / 2, (SCREEN_HEIGHT - SIZE) / 2);

            if(sf::FloatRect(SCREEN_WIDTH * 7 / 8, 0, SCREEN_WIDTH / 8, SCREEN_HEIGHT).contains(m_ball.getPosition()))
                m_ball.setPosition((SCREEN_WIDTH - SIZE) / 2, (SCREEN_HEIGHT - SIZE) / 2);
    
        }
        
        void reflect_ball() {
            if(m_ball.getGlobalBounds().top <= 0)
                m_direction.y *= -1;
            
            if(m_ball.getGlobalBounds().top + SIZE >= SCREEN_HEIGHT)
                m_direction.y *= -1;
        }

        void set_random_direction() {
            srand(time(0));

            random_angle = (double) rand() / RAND_MAX * M_PI * 2;

            m_direction.x = 10 * cos(random_angle);
            m_direction.y = 10 * sin(random_angle);
        }
};

int main()
{
    //===================INITIALISE====================
    sf::RenderWindow m_window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Pong");
    sf::Event m_event;

    Bat m_bat_1(m_window, KeyBinds(sf::Keyboard::W, sf::Keyboard::S), SCREEN_WIDTH / 6), m_bat_2(m_window, KeyBinds(sf::Keyboard::Up, sf::Keyboard::Down), SCREEN_WIDTH * 5 / 6);
    Ball m_ball(m_window, Bats(m_bat_1, m_bat_2));

    clock_t last_time = clock();

    const double time_per_frame = 1.0 / UPS, time_per_update = 1.0 / FPS;
    double dt_update = 0, dt_frame = 0;

    //===================INITIALISE====================


    while (m_window.isOpen()) {
        //*******************UPDATE*******************
        while (m_window.pollEvent(m_event)) {
            if(m_event.type == sf::Event::Closed) m_window.close();
        //*******************UPDATE*******************
        }

        dt_update += (double) (clock() - last_time) / CLOCKS_PER_SEC;
        dt_frame += (double) (clock() - last_time) / CLOCKS_PER_SEC;
        last_time = clock();

        //===================UPDATE====================
        if(dt_update >= time_per_update) {
            m_bat_1.update();
            m_bat_2.update();
            m_ball.update();
            
            dt_update -= time_per_update;
        }
        //===================UPDATE====================

        //===================DRAW====================
        if(dt_frame >= time_per_frame) {
            m_bat_1.render();
            m_bat_2.render();
            m_ball.render();

            m_window.display();
            m_window.clear();

            dt_frame -= time_per_frame;
        }
        //===================DRAW====================p

    }
    return 0;
}
