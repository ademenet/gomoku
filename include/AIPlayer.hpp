#ifndef AIPLAYER_HPP
# define AIPLAYER_HPP

# include <iostream>
# include <Eigen/Dense>
# include <unordered_map>
# include <array>
# include "BitBoard.hpp"
# include "ZobristTable.hpp"

# define SIZE 361   // the number of cells on the board (19*19)
# define S 3        // the number of states
# define INF 2147483647

typedef struct      s_node {
    BitBoard        player;
    BitBoard        player_forbidden;
    BitBoard        opponent;
    BitBoard        opponent_forbidden;
    uint8_t         pid;
    uint8_t         player_pairs_captured;
    uint8_t         opponent_pairs_captured;
}                   t_node;

typedef struct      s_best {
    int             score;
    int             pos;
}                   t_best;

typedef struct  s_stored {
    int     depth;
    t_best  value;
    /* Flags used for negamax with memory: 1 is exact, 2 is lowerbound and 3 is upperbound */
    int     flag;
}               t_stored;

class AIPlayer {

public:
    AIPlayer(void);
    AIPlayer(AIPlayer const &src);
    ~AIPlayer(void);
    AIPlayer    &operator=(AIPlayer const &rhs);

    int             nbnode; // DEBUG
    int             nbleaf; // DEBUG

    virtual t_best  getmove(t_node node, int depth) = 0;

protected:
    std::unordered_map<ZobristTable::Key, t_stored, ZobristTable::KeyHash>    _TT;
    
    BitBoard        get_moves(BitBoard const& player, BitBoard const& opponent,
                    BitBoard const& player_forbidden, int player_pairs_captured,
                    int opponent_pairs_captured);
    t_node          simulate_move(t_node const &node, int i);
    int32_t         score_function(t_node const &node, uint8_t depth);
    int32_t         player_score(t_node const &node, uint8_t depth);
    int32_t         opponent_score(t_node const &node, uint8_t depth);
    bool            check_end(t_node const& node);

    t_best          max(t_best const& a, t_best const& b) { return (a.score > b.score ? a : b); };
    t_best          min(t_best const& a, t_best const& b) { return (a.score < b.score ? a : b); };
    int             max(int const& a, t_best const& b)   { return (a > b.score ? a : b.score); };
    int             min(int const& a, t_best const& b)   { return (a < b.score ? a : b.score); };
    int             max(int const& a, int const& b)     { return (a > b ? a : b); };
    int             min(int const& a, int const& b)     { return (a < b ? a : b); };

};

#endif