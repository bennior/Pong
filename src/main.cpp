#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

#define SIZE 6
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define FPS 30
#define UPS 30

enum LAST_PRESSED_KEY {
    UP,
    DOWN,
    DEFAULT
};

int add(int a, int b) {
    return a + b;
}

int substract(int a, int b) {
    return a - b;
}

struct str_keybinds
{
    private:
        int up_keybind, down_keybind;
    
    public:
        str_keybinds(int _up_keybind, int _down_keybind) : up_keybind(_up_keybind), down_keybind(_down_keybind) {}
        bool is_up() {return sf::Keyboard::isKeyPressed((sf::Keyboard::Key) up_keybind);}
        bool is_down() {return sf::Keyboard::isKeyPressed((sf::Keyboard::Key) down_keybind);}
};

class acceleration {
    int x = 0;

    public:
        int current_acceleration(float k, float c) {
            int out = (int) (k * x * x + c); x = x < (int) (-sqrt(-4*k*c) / (k*2)) ? add(x, 1) : x;
            return out;
        }

        int current_deacceleration(float k, float c) {
            int out = (int) (k * x * x + c); x = x > 0 ? substract(x, 1) : x;
            return out;
        }

};

// struct  str_acceleration_manager
// {
//     private:
//         str_keybinds key_binds;
//         acceleration accelerate;
//         LAST_PRESSED_KEY LAST_KEY = DEFAULT;

//     public:
//         str_acceleration_manager(str_keybinds& _key_binds, acceleration& _acceleration) : key_binds(_key_binds), accelerate(_acceleration) {} 
//         void set_last_key() {
//             if(key_binds.is_up()) LAST_KEY = UP;
//             if(key_binds.is_down()) LAST_KEY = DOWN;
//         }
        
//         void mange_deacceleration(int& up_speed, int& down_speed) {
//             switch(LAST_KEY) {
//                 case UP: down_speed = 0; up_speed = up_speed >= 8 ? substract(up_speed , accelerate.current_deacceleration(-0.5f, 8.0f)) : up_speed; if(!up_speed) LAST_KEY = DEFAULT; 
//                     break;
//                 case DOWN: up_speed = 0; up_speed = down_speed >= 8 ? substract(down_speed , accelerate.current_deacceleration(-0.5f, 8.0f)) : down_speed; if(!down_speed) LAST_KEY = DEFAULT;
//                     break;
//                 case DEFAULT: 
//                     break;
//             }
//         }
// };


class Player {
    private:
        int x = 0, y = 500, up_speed = 0, down_speed = 0;
        sf::RenderWindow& m_window;
        str_keybinds binds;
        sf::RectangleShape m_rect;
        acceleration accelerate_up, accelerate_down;
        LAST_PRESSED_KEY LAST_KEY = DEFAULT;

    public:
        Player(sf::RenderWindow& _window, str_keybinds& _binds, int _x) : m_window(_window), binds(_binds), m_rect(sf::Vector2f(SIZE, SIZE * 4))  {
            x = _x;
            m_rect.setPosition(x, 500);
            m_rect.setFillColor(sf::Color::White);
        }   

        void render() {
            m_rect.setPosition(x, y);
            m_window.draw(m_rect);
        }

        void update() {
            if(binds.is_up()) {
                up_speed += accelerate_up.current_acceleration(-0.5f, 8.0f);
                y -= up_speed;
                LAST_KEY = UP;
                std::cout << up_speed << std::endl;
                return;
            }

            // up_speed -= accelerate_up.current_deacceleration(-0.5f, 8.0f);
            // up_speed = up_speed >= 8 ? substract(up_speed , accelerate_up.current_deacceleration(-0.5f, 8.0f)) : up_speed;
            // y -= up_speed;

            // std::cout << accelerate_up.x << std::endl;
             
            if(binds.is_down()) {
                down_speed += accelerate_down.current_acceleration(-0.5f, 8.0f);
                y += down_speed;
                LAST_KEY = DOWN;
                return;
            }

            LAST_KEY = up_speed == 0 && down_speed == 0 ? DEFAULT : LAST_KEY;
            up_speed = LAST_KEY == UP ? substract(up_speed, accelerate_up.current_deacceleration(-0.5f, 8.0f)) : 0;
            down_speed = LAST_KEY == DOWN ? substract(down_speed, accelerate_down.current_deacceleration(-0.5f, 8.0f)) : 0;
        
            y += (down_speed - up_speed);
            std::cout << up_speed << std::endl;

        }
};

int main()
{
    //===================INITIALISE====================
    sf::RenderWindow m_window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Pong");
    sf::Event m_event;
    str_keybinds player1_binds(sf::Keyboard::Up, sf::Keyboard::Down), player2_binds(sf::Keyboard::W, sf::Keyboard::S);
    Player player1(m_window, player1_binds, 100), player2(m_window, player2_binds, 500);

    clock_t last_time = clock();

    const double time_per_frame = 1.0 / UPS, time_per_update = 1.0 / FPS;
    double dt_update = 0, dt_frame = 0;

    //===================INITIALISE====================


    while (m_window.isOpen()) {
        //*******************UPDATE*******************
        while (m_window.pollEvent(m_event)) {
            if(m_event.type == sf::Event::Closed) m_window.close();
        }
        //*******************UPDATE*******************

        dt_update += (double) (clock() - last_time) / CLOCKS_PER_SEC;
        dt_frame += (double) (clock() - last_time) / CLOCKS_PER_SEC;
        last_time = clock();

        //===================UPDATE====================
        if(dt_update >= time_per_update) {
            player1.update();
            player2.update();

            dt_update -= time_per_update;
        }
        //===================UPDATE====================

        //===================DRAW====================
        if(dt_frame >= time_per_frame) {
            player1.render();
            player2.render();
            m_window.display();
            m_window.clear();

            dt_frame -= time_per_frame;
        }
        //===================DRAW====================p

    }
    return 0;
}
