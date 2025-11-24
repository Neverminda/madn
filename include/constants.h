#pragma once

#include <array>

constexpr int POS_HOME = -1;        // In home
constexpr int POS_TRACK_START = 0;  // Start field (relative position 0)
constexpr int POS_GOAL_START = 40;  // First goal field
constexpr int POS_GOAL_END = 43;    // Last goal field
constexpr int TRACK_SIZE = 40;      // Size of the field
constexpr std::array PLAYER_START_SQUARE = {0, 10, 20, 30}; // The absolute starting positions on the 40-space field
