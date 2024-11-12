#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

#define SIZE 10
#define SCREEN_WIDTH 1200
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
        int dAcc = 3, velocity = 0, size_num = 5;

        sf::RenderWindow& m_window;
        KeyBinds m_binds;

    public:
        sf::RectangleShape m_rect;

        Bat(sf::RenderWindow& _window, KeyBinds _binds, int _x) : m_window(_window), m_binds(_binds)  {
            x = _x;
            y = (SCREEN_HEIGHT - size_num * SIZE) / 2;

            m_rect = sf::RectangleShape(sf::Vector2f(SIZE, size_num * SIZE));
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

        int get_velocity() {
            return velocity;
        }
};

struct Bats
{
    public:
        Bat& m_bat_1; Bat& m_bat_2;
        sf::RectangleShape& m_rect_1; sf::RectangleShape& m_rect_2;    
        Bats(Bat& _bat1, Bat& _bat2) : m_bat_1(_bat1), m_bat_2(_bat2), m_rect_1(_bat1.m_rect), m_rect_2(_bat2.m_rect) {}
};

class Ball 
{
    private:
        int last_hit = clock(), hit_limit = 5 * 1000000 / UPS;
        float random_angle, velocity = 1.f;
        
        sf::Vector2f m_direction;
        sf::RenderWindow& m_window;
        Bats m_bats;
        sf::RectangleShape m_ball;
        sf::RectangleShape m_hitbox;

    public:
        Ball(sf::RenderWindow& _window, Bats _bats) : m_window(_window), m_bats(_bats), m_ball(sf::Vector2f(SIZE, SIZE)), m_hitbox(sf::Vector2f(SIZE, SIZE)) {
            set_random_direction();

            m_ball.setFillColor(sf::Color::White);
            m_ball.setPosition((SCREEN_WIDTH - SIZE) / 2, (SCREEN_HEIGHT - SIZE) / 2);

            m_hitbox.setFillColor(sf::Color::Red);
        }

        void render() {
            m_window.draw(m_ball);
            m_window.draw(m_hitbox);
        }

        void update() {
            //**move ball            
            m_ball.setPosition(m_ball.getPosition() + m_direction);

            //**rebound ball 
            rebound_ball();

            //**check ball's collision
            if(clock() - last_hit > hit_limit) check_hits();
            check_misses();
        }

        void check_hits() {
            //**check for player hits
            if(m_ball.getGlobalBounds().intersects(m_bats.m_rect_1.getGlobalBounds())) {
                m_direction.x *= -1.02f;
                last_hit = clock();
                calc_hitbox(m_bats.m_bat_2.m_rect.getPosition().x);
            }

            if(m_ball.getGlobalBounds().intersects(m_bats.m_rect_2.getGlobalBounds())) {
                m_direction.x *= -1.02f;
                last_hit = clock();
                calc_hitbox(m_bats.m_bat_1.m_rect.getPosition().x);
            }
        }

        void check_misses() {
            //**check for player points
            if(sf::FloatRect(0, 0, SCREEN_WIDTH / 9, SCREEN_HEIGHT).contains(m_ball.getPosition()))
                m_ball.setPosition((SCREEN_WIDTH - SIZE) / 2, (SCREEN_HEIGHT - SIZE) / 2);

            if(sf::FloatRect(SCREEN_WIDTH * 9 / 10, 0, SCREEN_WIDTH / 10, SCREEN_HEIGHT).contains(m_ball.getPosition()))
                m_ball.setPosition((SCREEN_WIDTH - SIZE) / 2, (SCREEN_HEIGHT - SIZE) / 2);
    
        }
        
        void rebound_ball() {
            if(m_ball.getGlobalBounds().top <= 0) {
                m_ball.setPosition(m_ball.getPosition().x, -m_ball.getPosition().y);
                m_direction.y *= -1;
            }
            
            if(m_ball.getGlobalBounds().top + SIZE >= SCREEN_HEIGHT) {
                m_ball.setPosition(m_ball.getPosition().x, 2 * SCREEN_HEIGHT - (m_ball.getPosition().y + SIZE));
                m_direction.y *= -1;
            }
        }

        void calc_hitbox(const int& _bat_pos) {
            float y = (_bat_pos - m_ball.getPosition().x) / m_direction.x * m_direction.y + m_ball.getPosition().y;

            if(y < 0) 
                y = -y;
            else if (y >= SCREEN_HEIGHT)
                y = 2 * SCREEN_HEIGHT - y - SIZE;

            m_hitbox.setPosition(_bat_pos, y);
        }

        void set_random_direction() {
            srand(clock());

            random_angle = (float) (rand() % 4 * 2 + 1) / 4 * M_PI; 

            m_direction.x = 10 * cos(random_angle);
            m_direction.y = 10 * sin(random_angle);
        }
};

int main()
{
    //===================INITIALISE====================
    sf::Font m_font;
    m_font.loadFromFile("src/prstartk.ttf");

    sf::Text m_player_name_1, m_player_name_2, m_score_1, m_score_2;

    m_player_name_1.setFont(m_font);
    m_player_name_2.setFont(m_font);
    m_score_1.setFont(m_font);
    m_score_2.setFont(m_font);

    m_player_name_1.setPosition(0, 0);
    m_player_name_2.setPosition(950, 0);

    m_player_name_1.setCharacterSize(15);
    m_player_name_2.setCharacterSize(15);

        //===================INPUT====================
        std::string name;
        std::cout << "ENTER FIRST PLAYER'S NAME: ";
        std::cin >> name;
        m_player_name_1.setString(name);
        std::cout << "ENTER SECOND PLAYER'S NAME: ";
        std::cin >> name;
        m_player_name_2.setString(name);
        //===================INPUT====================

    sf::RenderWindow m_window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Pong");
    sf::Event m_event;

    Bat m_bat_1(m_window, KeyBinds(sf::Keyboard::W, sf::Keyboard::S), SCREEN_WIDTH / 4), m_bat_2(m_window, KeyBinds(sf::Keyboard::Up, sf::Keyboard::Down), SCREEN_WIDTH * 3 / 4);
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
            m_window.draw(m_player_name_1);
            m_window.draw(m_player_name_2);

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
