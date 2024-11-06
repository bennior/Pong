#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

#define SIZE 6
#define SCREEN_WIDTH 600
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
        sf::RectangleShape m_rect;

    public:
        Bat(sf::RenderWindow& _window, KeyBinds _binds, int _x) : m_window(_window), m_binds(_binds)  {
            x = _x;
            y = 300;

            m_rect = sf::RectangleShape(sf::Vector2f(SIZE, SIZE * 6));
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
        Bat& m_bat1; Bat& m_bat2;    
        Bats(Bat& _bat1, Bat& _bat2) : m_bat1(_bat1), m_bat2(_bat2) {}
};

class Ball
{
private:
    sf::Vector2f m_direction;
    sf::RenderWindow& m_window;
    Bats m_bats;
    sf::RectangleShape m_ball;

public:
    Ball(sf::RenderWindow& _window, Bats _bats) : m_window(_window), m_bats(_bats), m_ball(sf::Vector2f(SIZE, SIZE)) {
        set_random_direction();

        m_ball.setFillColor(sf::Color::Blue);
        m_ball.setPosition(300, 300);
    }

    void render() {
        m_window.draw(m_ball);
    }

    void update() {
        m_ball.setPosition(m_ball.getPosition() + m_direction);
    }

    void set_random_direction() {
        srand(time(0));

        double random_angle = (double) rand() / RAND_MAX * M_PI * 2;

        m_direction.x = 10 * cos(random_angle);
        m_direction.y = 10 * sin(random_angle);
    }

};

int main()
{
    //===================INITIALISE====================
    sf::RenderWindow m_window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Pong");
    sf::Event m_event;

    Bat m_bat1(m_window, KeyBinds(sf::Keyboard::W, sf::Keyboard::S), 100), m_bat2(m_window, KeyBinds(sf::Keyboard::Up, sf::Keyboard::Down), 500);
    Ball m_ball(m_window, Bats(m_bat1, m_bat2));

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
            m_bat1.update();
            m_bat2.update();
            m_ball.update();
            
            dt_update -= time_per_update;
        }
        //===================UPDATE====================

        //===================DRAW====================
        if(dt_frame >= time_per_frame) {
            m_bat1.render();
            m_bat2.render();
            m_ball.render();

            m_window.display();
            m_window.clear();

            dt_frame -= time_per_frame;
        }
        //===================DRAW====================p

    }
    return 0;
}
