//
//  statistics.hpp
//  madn
//
//  Created by Thomas Schmitt on 19.12.25.
//

#pragma once

struct statistics {
    int moves = 0;
    int games = 0;
    int state_size = sizeof(int)*3+sizeof(std::array<std::array<int, 4>, 4>);
    void save(int player_id, int roll, int pawn_to_move_index, const std::array<std::array<int, 4>, 4> & pawn)
    {
        moves++;
    }
};
