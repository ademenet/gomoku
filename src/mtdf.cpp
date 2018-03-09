#include "mtdf.hpp"
#include "ZobristTable.hpp"

bool            times_up(std::chrono::steady_clock::time_point start, uint32_t limit) { // THIS FUNCTION WORKS
    return (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() > limit);
}

Eigen::Array2i  iterative_deepening(t_node *root, int8_t max_depth) { // THIS FUNCTION WORKS
    std::chrono::steady_clock::time_point   start = std::chrono::steady_clock::now();
    t_ret                                   ret = { 0, 0 };

    std::cout << std::endl;
    for (int depth = 1; depth < max_depth; depth++) {
        // ret = mtdf(root, ret.score, depth);
        ret = alphaBetaWithMemory(*root, -INF, INF, depth);
        std::cout << ret.score << std::endl;
        if (times_up(start, 500)) {
            std::cout << "TIMES UP, reached depth : " << depth << std::endl;
            break;
        }
    }
    std::cout << "score: " << ret.score << ", pos: (" << ret.p / 19 << ", " << ret.p % 19 << ")\n";
    return ((Eigen::Array2i){ ret.p / 19, ret.p % 19 });
}

t_ret   mtdf(t_node *root, int32_t firstguess, int8_t depth) { // THIS FUNCTION WORKS?
    int32_t bound[2] = { -INF, INF };
    int32_t beta;
    t_ret   ret = { firstguess, 0 };

    do {
        beta = ret.score + (ret.score == bound[0]);
        ret = alphaBetaWithMemory(*root, beta-1, beta, depth);
        std::cout << "score: " << ret.score << ", pos: (" << ret.p / 19 << ", " << ret.p % 19 << ")\n";
        bound[(ret.score < beta)] = ret.score;
    } while (bound[0] >= bound[1]);
    return (ret);
}

t_ret   alphaBetaWithMemory(t_node root, int32_t alpha, int32_t beta, int8_t depth) {
    BitBoard    moves = get_player_open_adjacent_positions(root.player, root.opponent) & ~root.player_forbidden;
    if (moves.set_count() == 0) {
        moves = get_player_open_adjacent_positions(root.opponent, root.player) & ~root.opponent_forbidden;
        if (moves.set_count() == 0)
            moves.write(9, 9);
    }

    int32_t     v;
    uint16_t    pos;
    t_node      tmp = root;

    for (int i = 0; i < 361; ++i) {
        if (moves.check_bit(i)) {
            simulate_move(&root, i);
            v = min(&root, alpha, beta, depth-1);
            root = tmp;
            if (v > alpha) {
                alpha = v;
                pos = i;
            }
        }
    }
    return ((t_ret){ alpha, pos });
}

int32_t        min(t_node *node, int32_t alpha, int32_t beta, int8_t depth) {
//     int32_t lookup = TT_lookup(node, alpha, beta, depth);
//     if (lookup != -INF)
//         return (lookup);

    if (depth == 0 || check_end(node->player, node->opponent, node->player_pairs_captured, node->opponent_pairs_captured)) {
        int32_t score = score_function(node, depth+1);
        // TT_store(node, score, alpha, beta, depth);
        return (score);
    }

    BitBoard    moves = get_player_open_adjacent_positions(node->opponent, node->player) & ~node->opponent_forbidden;
    t_node      tmp = *node;
    // int32_t     b = beta; /* backup original */

    for (int i = 0; i < 361; ++i) {
        if (moves.check_bit(i)) {
            simulate_move(node, i);
            beta = min_val(beta, max(node, alpha, beta, depth-1));
            *node = tmp;
            if (beta <= alpha)
                break;
        }
    }
    // TT_store(node, beta, alpha, b, depth);
    return (beta);
}

int32_t        max(t_node *node, int32_t alpha, int32_t beta, int8_t depth) {
    // int32_t lookup = TT_lookup(node, alpha, beta, depth);
    // if (lookup != -INF)
    //     return (lookup);

    if (depth == 0 || check_end(node->opponent, node->player, node->opponent_pairs_captured, node->player_pairs_captured)) {
        int32_t score = score_function(node, depth+1);
        // TT_store(node, score, alpha, beta, depth);
        return (score);
    }

    BitBoard    moves = get_player_open_adjacent_positions(node->player, node->opponent) & ~node->player_forbidden;
    t_node      tmp = *node;
    // int32_t     a = alpha; /* backup original */

    for (int i = 0; i < 361; ++i) {
        if (moves.check_bit(i)) {
            simulate_move(node, i);
            alpha = max_val(alpha, min(node, alpha, beta, depth-1));
            *node = tmp;
            if (beta <= alpha)
                break;
        }
    }
    // TT_store(node, alpha, a, beta, depth);
    return (alpha);
}


void        TT_store(t_node *node, int32_t best, int32_t alpha, int32_t beta, int8_t depth) {
    t_stored    entry;

    entry.depth = depth;
    entry.score = best;
    if (best <= alpha)
        entry.flag = ZobristTable::flag::upperbound;
    else if (best >= beta)
        entry.flag = ZobristTable::flag::lowerbound;
    else
        entry.flag = ZobristTable::flag::exact;
    ZobristTable::map[{node->player, node->opponent}] = entry;
    // std::cout << "TT storing : " << entry.score << " " << std::to_string(entry.flag) << " " << std::to_string(entry.depth) << std::endl;
}

int32_t        TT_lookup(t_node *node, int32_t alpha, int32_t beta, int8_t depth) {
    t_stored    entry;

    if (ZobristTable::map.count({node->player, node->opponent})) {
        entry = ZobristTable::map[{node->player, node->opponent}];
        // std::cout << "TT lookup : " << entry.score << " " << std::to_string(entry.flag) << " " << std::to_string(entry.depth) << std::endl;
        // std::cout << "TT lookup at " << std::to_string(depth) << std::endl;
        if (entry.depth >= depth) {
            if (entry.flag == ZobristTable::flag::exact)
                return (entry.score);
            else if (entry.flag == ZobristTable::flag::lowerbound && entry.score > alpha)
                alpha = entry.score;
            else if (entry.flag == ZobristTable::flag::upperbound && entry.score < beta)
                beta = entry.score;
            if (alpha >= beta)
                return (entry.score);
        }
    }
    return (-INF);
}

void            simulate_move(t_node *node, uint16_t i) { // this is the biggest performance hit
    /* simulate player move */
    if (node->pid == 1) {
        BitBoard pairs = pair_capture_detector(node->player, node->opponent);
        node->player.write(i);
        if ((pairs & node->player).is_empty() == false) {
            pairs = highlight_captured_stones(node->opponent, node->player, pairs);
            node->player_pairs_captured += pairs.set_count() / 2;
            node->opponent &= ~pairs;
            node->opponent_forbidden = forbidden_detector(node->opponent, node->player);
        }
        node->player_forbidden = forbidden_detector(node->player, node->opponent);
        node->pid = 2;
    }/* simulate opponent move */
    else {
        BitBoard pairs = pair_capture_detector(node->opponent, node->player);
        node->opponent.write(i);
        if ((pairs & node->opponent).is_empty() == false) {
            pairs = highlight_captured_stones(node->player, node->opponent, pairs);
            node->opponent_pairs_captured += pairs.set_count() / 2;
            node->player &= ~pairs;
            node->player_forbidden = forbidden_detector(node->player, node->opponent);
        }
        node->opponent_forbidden = forbidden_detector(node->opponent, node->player);
        node->pid = 1;
    }
}

int32_t        score_function(t_node *node, uint8_t depth) {
    int32_t     score = 0;

    score += player_score(node, depth);
    score -= opponent_score(node, depth) * 2;
    return (score);
}

int32_t    player_score(t_node *node, uint8_t depth) {
    BitBoard    board;
    int32_t     score = 0;

    for (int i = 0; i < 10; ++i) {
        board = current_pattern_detector(node->player, node->opponent, BitBoard::patterns[i]);
        score += (board.is_empty() == false ? board.set_count() * BitBoard::patterns[i].value : 0);
        // Bonus x10 if patterns ends on opponent_forbidden, so he cannot counter
        // score += ((board & node->opponent_forbidden).is_empty() == false ? board.set_count() * BitBoard::patterns[i].value * 10 : 0);
    }
    if (detect_five_aligned(node->player)) score += 10000000 * depth; // 10,000,000pts for win by alignement
    if (node->player_pairs_captured == 5)  score +=  5000000 * depth; //  5,000,000pts for win by pairs captures
    score += node->player_pairs_captured * 50000;                     //     50,000pts/captures
    return (score);
}

int32_t    opponent_score(t_node *node, uint8_t depth) {
    BitBoard    board;
    int32_t     score = 0;

    for (int i = 0; i < 10; ++i) {
        board = current_pattern_detector(node->opponent, node->player, BitBoard::patterns[i]);
        if (0 <= i && i <= 1)
            score += (board.is_empty() == false ? board.set_count() * BitBoard::patterns[i].value * 2 : 0);
        else
            score += (board.is_empty() == false ? board.set_count() * BitBoard::patterns[i].value : 0);
        // Bonus x10 if patterns ends on opponent_forbidden, so he cannot counter
        // score += ((board & node->player_forbidden).is_empty() == false ? board.set_count() * BitBoard::patterns[i].value * 10 : 0);
    }
    if (detect_five_aligned(node->opponent)) score += 10000000 * depth; // 10,000,000pts for win by alignement
    if (node->opponent_pairs_captured == 5)  score +=  5000000 * depth; //  5,000,000pts for win by pairs captures
    score += node->opponent_pairs_captured * 50000;                     //     50,000pts/captures
    return (score);
}


// Does not handle the case where we continue to play without having done the capture of the pairs that breaks the five !
// uint8_t check_end(BitBoard const& p1, BitBoard const& p2, uint8_t const& p1_pairs_captured, uint8_t const& p2_pairs_captured) {
//     if (p1_pairs_captured >= 5)
//         return (1);
//     if (detect_five_aligned(p1)) {
//         BitBoard    pairs = pair_capture_detector(p2, p1); // good
//         /* p2 wins next turn by capturing a fifth pair even though p1 has 5 aligned */
//         if (!pairs.is_empty() && p2_pairs_captured == 4)
//             return (0);
//         /* game continue by capturing stones forming the 5 alignment */
//         if ( detect_five_aligned(highlight_five_aligned(p1) ^ highlight_captured_stones(p1, p2 ^ pairs, pairs)) == false )
//             return (0);
//         return (1);
//     }
//     if (((p1 | p2) ^ BitBoard::full).is_empty() == true)
//         return (2);
//     return (0);
// }

/* old version but simpler so we use it for now */
bool check_end(BitBoard const& player, BitBoard const& opponent, uint8_t const& player_pairs_captured, uint8_t const& opponent_pairs_captured) {
    if (player_pairs_captured >= 5)
        return (true);
    if (detect_five_aligned(player) == true)
        return (true);
    if (((player | opponent) ^ BitBoard::full).is_empty() == true)
        return (true);
    return (false);
}

int32_t         min_val(int32_t const &a, int32_t const &b) {
    return (a < b ? a : b);
}

int32_t         max_val(int32_t const &a, int32_t const &b) {
    return (a > b ? a : b);
}
