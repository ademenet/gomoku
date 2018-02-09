#include "Human.hpp"

Human::Human(GameEngine *game_engine, unsigned short id) : Player(game_engine, id) {
}

Human::Human(Human const &src) : Player(src) {
    *this = src;
}

Human::~Human(void) {
}

Human	&Human::operator=(Human const &src) {
    this->_game_engine = src.get_game_engine();
    this->_id = src.get_id();
    return (*this);
}

t_action    Human::play(void) {
    t_action    action;

    /*  Await the players input, check if those are valid, if not,
        loop again.
    */
    action.pos = {6, 10}; // TMP
    action.player_id = this->_id;
    action.id = this->_game_engine->get_history_size() + 1;
    action.timepoint = std::chrono::steady_clock::now() - this->_game_engine->get_initial_timepoint();
    return action;
}
