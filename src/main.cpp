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
        bool is_freeze = true;

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
            if(is_freeze) return;
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
            m_rect.setPosition(x, y);
            velocity = 0;
        }

        void freeze() {
            is_freeze = true;
        }

        void unfreeze() {
            is_freeze = false;
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

        void freeze() {
            bat_1.freeze();
            bat_2.freeze();
        }

        void unfreeze() {
            bat_1.unfreeze();
            bat_2.unfreeze();
        }
};

class Ball 
{
    private:
        int last_hit = clock(), hit_limit = 5 * 1000000 / UPS;
        int count = 0; bool is_count_down = true;
            
        // PlayState m_playstate;
        sf::RenderWindow& m_window;
        Bats m_bats;
        sf::RectangleShape m_ball;
        sf::Vector2f m_direction;
        Position m_position;
        sf::Text m_count_down;
        sf::Font m_font;

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

            m_font.loadFromFile("src/prstartk.ttf");

            m_count_down.setFont(m_font);
            m_count_down.setString("3");
            m_count_down.setCharacterSize(50);
            m_count_down.setPosition((SCREEN_WIDTH - m_count_down.getGlobalBounds().width) / 2, (SCREEN_HEIGHT - m_count_down.getGlobalBounds().height) / 2);
        }

        void render() {
            if(is_count_down) { m_window.draw(m_count_down); return; }
            m_window.draw(m_ball);
        }

        void update() {
            //**check for count_down
            if(is_count_down) {do_count_down(); return;}

            //**move ball
            m_position += m_direction;
            m_ball.setPosition(m_position.to_vector2f());

            //**rebound ball 
            rebound_ball();

            //**check ball's collision
            if(clock() - last_hit > hit_limit) check_hits();
            check_misses();
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

        void do_count_down() {
            count++;
            m_count_down.setString(std::to_string(3 - count / UPS));

            if(count >= UPS * 3) {is_count_down = !is_count_down; m_bats.unfreeze();}
        }

        void reset() {
            set_random_direction();

            m_position.x = (SCREEN_WIDTH - SIZE) / 2;
            m_position.y = (SCREEN_HEIGHT - SIZE) / 2;

            m_ball.setPosition(m_position.to_vector2f());

            m_bats.reset();
            m_bats.freeze();

            count = 0;
            is_count_down = !is_count_down;    
        }

        void set_random_direction() {
            srand(clock());

            float random_angle = (float) (rand() % 4 * 2 + 1) / 4 * M_PI; 
            
            m_direction.y = 10 * sin(random_angle);
            m_direction.x = 10 * cos(random_angle);
        }
};

void loading_animation(sf::RenderWindow& _window);
bool check_if_pressed();
void cycle_through_font(sf::Text& _text);

int main()
{
    //===================INITIALISE====================
    
    //**fonts
    sf::Font m_font;
    m_font.loadFromFile("src/prstartk.ttf");

    //**texts
    sf::Text m_player_name_1, m_player_name_2, m_score_1, m_score_2, m_winner;
    sf::Text m_title("PONG", m_font, 95);
    sf::Text m_subtitle("PRESS SPACE TO START!", m_font, 30);

    //**set fonts
    m_player_name_1.setFont(m_font);
    m_player_name_2.setFont(m_font);
    m_score_1.setFont(m_font);
    m_score_2.setFont(m_font);
    m_winner.setFont(m_font);

    //**set character sizes
    m_player_name_1.setCharacterSize(25);
    m_player_name_2.setCharacterSize(25);
    m_score_1.setCharacterSize(30);
    m_score_2.setCharacterSize(30);
    m_winner.setCharacterSize(50);

    //**set fill color
    m_title.setFillColor(sf::Color(255, 255, 255, 215));
    m_subtitle.setFillColor(sf::Color(255, 255, 255, 215));

    //**initialise font iterator
    int* iterator = new int(0);

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
    m_title.setPosition((SCREEN_WIDTH - m_title.getGlobalBounds().width) / 2, SCREEN_HEIGHT / 3);
    m_subtitle.setPosition((SCREEN_WIDTH - m_subtitle.getGlobalBounds().width) / 2, SCREEN_HEIGHT / 2 + 2 * m_subtitle.getGlobalBounds().height);

    sf::RenderWindow m_window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Pong");
    sf::Event m_event;

    Bat m_bat_1(m_window, KeyBinds(sf::Keyboard::W, sf::Keyboard::S), SCREEN_WIDTH / 4), m_bat_2(m_window, KeyBinds(sf::Keyboard::Up, sf::Keyboard::Down), SCREEN_WIDTH * 3 / 4);
    Ball m_ball(m_window, Bats(m_bat_1, m_bat_2));
    sf::RectangleShape m_line(sf::Vector2f(2, SCREEN_HEIGHT / 60));

    GameState GAME_STATE = START_SCREEN;
    //===================INITIALISE====================


    //===================ANIMATION====================
    loading_animation(m_window);
    //===================ANIMATION====================
    
    //===================CLOCK=SETUP====================
    clock_t last_time = clock();

    const double time_per_frame = 1.0 / FPS, time_per_update = 1.0 / UPS;
    double dt_update = 0, dt_frame = 0;
    //===================CLOCK=SETUP====================
    
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
            switch (GAME_STATE)
            {
            case START_SCREEN:
                (*iterator)++;

                if(!(*iterator % UPS)) {cycle_through_font(m_subtitle); }
                if(check_if_pressed()) {GAME_STATE = PLAYING; delete iterator; }
                break;
            case PLAYING:
                m_score_1.setString(std::to_string(m_bat_1.score));
                m_score_2.setString(std::to_string(m_bat_2.score));

                m_bat_1.update();
                m_bat_2.update();
                m_ball.update();

                if(m_bat_1.score >= 8 || m_bat_2.score >= 8) GAME_STATE = GAME_OVER;
                break;
            case GAME_OVER:
                m_bat_1.reset();
                m_bat_2.reset();

                if(check_if_pressed()) {
                    GAME_STATE = PLAYING;
                    m_bat_1.score = 0;
                    m_bat_2.score = 0;
                }
                break;            
            }
            dt_update -= time_per_update;
        }
        //===================UPDATE====================

        //===================DRAW====================
        if(dt_frame >= time_per_frame) {
            switch (GAME_STATE)
            {
            case START_SCREEN:
                m_window.draw(m_title);
                m_window.draw(m_subtitle);
                break;
            
            case PLAYING:
                for (unsigned int i = 0; i < SCREEN_HEIGHT / m_line.getGlobalBounds().height * 2; i++)
                {
                    m_line.setPosition(SCREEN_WIDTH / 2 - 1, i * m_line.getGlobalBounds().height * 2);
                    m_window.draw(m_line);
                }
                
                m_window.draw(m_player_name_1);
                m_window.draw(m_player_name_2);
                m_window.draw(m_score_1);
                m_window.draw(m_score_2);
                
                m_bat_1.render();
                m_bat_2.render();
                m_ball.render();

                break;

            case GAME_OVER:
                if(m_bat_1.score > m_bat_2.score) {
                    m_winner.setString(std::string(m_player_name_1.getString()).append(" WON!"));
                }else {
                    m_winner.setString(std::string(m_player_name_2.getString()).append(" WON!"));
                }

                m_winner.setPosition((SCREEN_WIDTH - m_winner.getGlobalBounds().width) / 2, (SCREEN_HEIGHT - 2 * m_winner.getGlobalBounds().height) / 2);

                m_window.draw(m_winner);
                m_window.draw(m_subtitle);
                break;
            }
            m_window.display();
            m_window.clear();

            dt_frame -= time_per_frame;
        }
        //===================DRAW====================

    }
    return 0;
}

void loading_animation(sf::RenderWindow& _window) {

    clock_t last_time = clock();

    const double time_per_update = 1.0 / UPS;
    double dt_update = 0;

    sf::Font m_font; m_font.loadFromFile("src/prstartk.ttf");
    sf::Event _event;
    sf::Text _title("PONG", m_font, 95);

    int alpha = 0;

    while (_window.isOpen()) {
        //*******************UPDATE*******************
        while (_window.pollEvent(_event)) {
            if(_event.type == sf::Event::Closed) _window.close();
        }
        //*******************UPDATE*******************

        dt_update += (double) (clock() - last_time) / CLOCKS_PER_SEC;
        last_time = clock();

        //===================UPDATE====================
        if(dt_update >= time_per_update) {
            alpha += 5;
            _title.setPosition((SCREEN_WIDTH - _title.getGlobalBounds().width) / 2, SCREEN_HEIGHT / 3);
            _title.setFillColor(sf::Color(255, 255, 255, alpha));
            _window.draw(_title);

            _window.display();
            _window.clear();

            dt_update -= time_per_update;
        }


        if(alpha >= 215) {
            break;
        }
    }


}

bool check_if_pressed() {
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
}

void cycle_through_font(sf::Text& _subtitle) {
        if(_subtitle.getCharacterSize() % 2) {
            _subtitle.setCharacterSize(_subtitle.getCharacterSize() + 1);
        }else{
            _subtitle.setCharacterSize(_subtitle.getCharacterSize() - 1);
        }
        _subtitle.setPosition((SCREEN_WIDTH - _subtitle.getGlobalBounds().width) / 2, SCREEN_HEIGHT / 2 + 2 * _subtitle.getGlobalBounds().height);
}