#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <algorithm> 

#define SIZE 10
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 600
#define FPS 30
#define UPS 30

#define LOG(x) std::cout << x << std::endl;

enum GameState {
    START_SCREEN,
    PLAYING,
    GAME_OVER
};

struct KeyBinds
{
    private:
        int up_keybind, down_keybind;
    
    public:
        KeyBinds(int _up_keybind, int _down_keybind) : up_keybind(_up_keybind), down_keybind(_down_keybind) {}
        bool up() {return sf::Keyboard::isKeyPressed((sf::Keyboard::Key) up_keybind);}
        bool down() {return sf::Keyboard::isKeyPressed((sf::Keyboard::Key) down_keybind);}
};

struct Position
{        
    public:
        double x, y;
        Position() {}
        Position(double _x, double _y) : x(_x), y(_y) {}

        Position operator+(const Position& _pos) {
            return Position{this->x + _pos.x, this->y + _pos.y};
        }

        Position operator+(const sf::Vector2f& _vec) {
            return Position{this->x + _vec.x, this->y + _vec.y};
        }

        Position operator-(const Position& _pos) {
            return Position{this->x - _pos.x, this->y - _pos.y};
        }

        Position operator-(const sf::Vector2f& _vec) {
            return Position{this->x - _vec.x, this->y - _vec.y};
        }

        void operator+=(const Position& _pos) {
            x += _pos.x;
            y += _pos.y;
        }

        void operator+=(const sf::Vector2f& _vec) {
            x += _vec.x;
            y += _vec.y;
        }

        void operator-=(const Position& _pos) {
            x -= _pos.x;
            y -= _pos.y;
        }

        void operator-=(const sf::Vector2f& _vec) {
            x += _vec.x;
            y += _vec.y;
        }

        sf::Vector2f to_vector2f() {
            return sf::Vector2f(this->x, this->y);
        }
};

class Bat {
        
    private:
        int x, y;
        int dAcc = 3, velocity = 0, size_num = 5;

        sf::RenderWindow& m_window;
        KeyBinds m_binds;

    public:
        sf::RectangleShape m_rect;
        int score = 0;

        Bat(sf::RenderWindow& _window, KeyBinds _binds, int _x) :
         m_window(_window),
         m_binds(_binds)
        {
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
            }

            //**decrease velocity by factor 0.94
            velocity *= 0.94f;

            //**update position of bat
            m_rect.setPosition(x, y);
        }

        void reset() {
            y = (SCREEN_HEIGHT - size_num * SIZE) / 2;
            velocity = 0;
        }
};



struct Bats
{
    public:
        Bat& bat_1; Bat& bat_2;    
        Bats(Bat& _bat1, Bat& _bat2) : bat_1(_bat1), bat_2(_bat2) {}
        
        void reset() {
            bat_1.reset();
            bat_2.reset();
        }
};

class Ball 
{
    private:
        int last_hit = clock(), hit_limit = 5 * 1000000 / UPS;
            
        // PlayState m_playstate;
        sf::RenderWindow& m_window;
        Bats m_bats;
        sf::RectangleShape m_ball;
        sf::Vector2f m_direction;
        Position m_position;

    public:
        Ball(sf::RenderWindow& _window, Bats _bats) :
        //  m_playstate(PLAY),
         m_window(_window),
         m_bats(_bats),
         m_ball(sf::Vector2f(SIZE, SIZE))
        {
            set_random_direction();

            m_position = Position((SCREEN_WIDTH - SIZE) / 2, (SCREEN_HEIGHT - SIZE) / 2);

            m_ball.setFillColor(sf::Color::White);
            m_ball.setPosition(m_position.to_vector2f());
        }

        void render() {
            m_window.draw(m_ball);
        }

        void update() {
            // switch (m_playstate)
            // {
            // case COUNT_DOWN:

            // break;

            // case PLAY:
            // //**move ball
            m_position += m_direction;
            m_ball.setPosition(m_position.to_vector2f());

            //**rebound ball 
            rebound_ball();

            //**check ball's collision
            if(clock() - last_hit > hit_limit) check_hits();
            check_misses();
            // break;

            // default:
            // break;
            // }
        }

        void rebound_ball() {
            if(m_ball.getGlobalBounds().top <= 0)
                m_direction.y *= -1;
            
            if(m_ball.getGlobalBounds().top + SIZE >= SCREEN_HEIGHT)
                m_direction.y *= -1;
        }

        void check_hits() {
            //**check for player hits
            if(m_ball.getGlobalBounds().intersects(m_bats.bat_1.m_rect.getGlobalBounds())) {
                m_direction.x *= -1.02;
                last_hit = clock();

                sf::Vector2f offset(fmod(m_bats.bat_2.m_rect.getPosition().x - m_position.x, m_direction.x), 0);
                m_position.x += offset.x;
            }

            if(m_ball.getGlobalBounds().intersects(m_bats.bat_2.m_rect.getGlobalBounds())) {
                m_direction.x *= -1.02f;
                last_hit = clock();
    
                sf::Vector2f offset(fmod(m_bats.bat_1.m_rect.getPosition().x - m_position.x, m_direction.x), 0);
                m_position.x += offset.x;
            }
        }

        void check_misses() {
            //**check for player points
            if(sf::FloatRect(0, 0, SCREEN_WIDTH / 10, SCREEN_HEIGHT).contains(m_ball.getPosition())) {
                m_bats.bat_2.score++;
                reset();
            }

            if(sf::FloatRect(SCREEN_WIDTH * 9 / 10, 0, SCREEN_WIDTH / 10, SCREEN_HEIGHT).contains(m_ball.getPosition())) {
                m_bats.bat_1.score++;
                reset();
            }
        }

        void reset() {
            set_random_direction();
            m_bats.reset();

            m_position.x = (SCREEN_WIDTH - SIZE) / 2;
            m_position.y = (SCREEN_HEIGHT - SIZE) / 2;
        }

        // void reset() {
        //     set_random_direction();

        //     m_position.x = (SCREEN_WIDTH - SIZE) / 2;
        //     m_position.y = (SCREEN_HEIGHT - SIZE) / 2;
            
        // }

        void set_random_direction() {
            srand(clock());

            float random_angle = (float) (rand() % 4 * 2 + 1) / 4 * M_PI; 
            
            m_direction.y = 10 * sin(random_angle);
            m_direction.x = 10 * cos(random_angle);
        }
};

int main()
{
    //===================INITIALISE====================
    
    //**fonts
    sf::Font m_font;
    m_font.loadFromFile("src/prstartk.ttf");

    //**texts
    sf::Text m_player_name_1, m_player_name_2, m_score_1, m_score_2;

    //**set fonts
    m_player_name_1.setFont(m_font);
    m_player_name_2.setFont(m_font);
    m_score_1.setFont(m_font);
    m_score_2.setFont(m_font);


    //**set character sizes
    m_player_name_1.setCharacterSize(25);
    m_player_name_2.setCharacterSize(25);
    m_score_1.setCharacterSize(30);
    m_score_2.setCharacterSize(30);

        //===================INPUT====================
        std::string* name = new std::string;
        std::cout << "ENTER FIRST PLAYER'S NAME: ";
        std::cin >> *name;
        std::transform(name->begin(), name->end(), name->begin(), ::toupper);
        m_player_name_1.setString(*name);
        std::cout << "ENTER SECOND PLAYER'S NAME: ";
        std::cin >> *name;
        std::transform(name->begin(), name->end(), name->begin(), ::toupper);
        m_player_name_2.setString(*name);
        delete name;
        //===================INPUT====================
    
    //**set positions
    m_player_name_1.setPosition(15, 15);
    m_player_name_2.setPosition(SCREEN_WIDTH - 15 - m_player_name_2.getGlobalBounds().width, 15);
    m_score_1.setPosition(20, 15 + 40 + m_player_name_1.getGlobalBounds().height);
    m_score_2.setPosition(SCREEN_WIDTH - 20 - sf::Text("0", m_font, m_score_2.getCharacterSize()).getGlobalBounds().width, 15 + 40 + m_player_name_1.getGlobalBounds().height);

    sf::RenderWindow m_window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Pong");
    sf::Event m_event;

    Bat m_bat_1(m_window, KeyBinds(sf::Keyboard::W, sf::Keyboard::S), SCREEN_WIDTH / 4), m_bat_2(m_window, KeyBinds(sf::Keyboard::Up, sf::Keyboard::Down), SCREEN_WIDTH * 3 / 4);
    Ball m_ball(m_window, Bats(m_bat_1, m_bat_2));

    clock_t last_time = clock();

    const double time_per_frame = 1.0 / UPS, time_per_update = 1.0 / FPS;
    double dt_update = 0, dt_frame = 0;
    //===================INITIALISE====================

    // sf::Text m_title("PONG", m_font, 25);

    // int csize = 0;

    // while (m_window.isOpen()) {
    //     //*******************UPDATE*******************
    //     while (m_window.pollEvent(m_event)) {
    //         if(m_event.type == sf::Event::Closed) m_window.close();
    //     }
    //     //*******************UPDATE*******************

    //     dt_update += (double) (clock() - last_time) / CLOCKS_PER_SEC;
    //     dt_frame += (double) (clock() - last_time) / CLOCKS_PER_SEC;
    //     last_time = clock();

    //     //===================UPDATE====================
    //     if(dt_update >= time_per_update) {
    //         csize++;
    //         m_title.setCharacterSize(m_title.getCharacterSize() + 1);
    //         m_title.setPosition((SCREEN_WIDTH - m_title.getGlobalBounds().width) / 2, SCREEN_HEIGHT / 3);
    //         m_window.draw(m_title);

    //         LOG(m_title.getCharacterSize())
    //         m_window.display();
    //         m_window.clear();

    //         dt_update -= time_per_update;
    //     }


    //     if(m_title.getCharacterSize() >= 95) {
    //         break;
    //     }
    // }
    
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
            m_score_1.setString(std::to_string(m_bat_1.score));
            m_score_2.setString(std::to_string(m_bat_2.score));

            m_bat_1.update();
            m_bat_2.update();
            m_ball.update();

            if(m_bat_1.score >= 8 || m_bat_2.score >= 8) {
                m_bat_1.reset();
                m_bat_2.reset();
                m_bat_1.score = 0;
                m_bat_2.score = 0;
            }
            
            dt_update -= time_per_update;
        }
        //===================UPDATE====================

        //===================DRAW====================
        if(dt_frame >= time_per_frame) {
            m_window.draw(m_player_name_1);
            m_window.draw(m_player_name_2);
            m_window.draw(m_score_1);
            m_window.draw(m_score_2);
            
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
