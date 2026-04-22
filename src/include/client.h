#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <cstdlib>
#include <ctime>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// Global variables for storing game state
char current_map[30][30];        // Current map state from server
bool is_mine[30][30];            // true if we believe there's a mine
bool is_safe[30][30];            // true if we believe it's safe
bool client_visited[30][30];     // true if visited (renamed to avoid conflict)
int adjacent_mines[30][30];      // Number of adjacent mines if revealed
std::set<std::pair<int, int>> unknown_cells;  // Cells we haven't explored yet

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // TODO (student): Initialize all your global variables!
  // Initialize random seed
  std::srand(std::time(0));

  // Clear all arrays
  for (int i = 0; i < 30; i++) {
    for (int j = 0; j < 30; j++) {
      current_map[i][j] = '?';
      is_mine[i][j] = false;
      is_safe[i][j] = false;
      client_visited[i][j] = false;
      adjacent_mines[i][j] = 0;
    }
  }

  unknown_cells.clear();

  // Read first move from input
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  // TODO (student): Implement me!
  // Read the current map state from stdin
  for (int i = 0; i < rows; i++) {
    std::string row;
    std::cin >> row;
    for (int j = 0; j < columns; j++) {
      current_map[i][j] = row[j];
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  // TODO (student): Implement me!

  // First, analyze the current map to update our knowledge
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (current_map[i][j] >= '0' && current_map[i][j] <= '8') {
        // This is a revealed number
        int num = current_map[i][j] - '0';
        adjacent_mines[i][j] = num;
        client_visited[i][j] = true;

        // Count unknown neighbors and marked neighbors
        int unknown_neighbors = 0;
        int marked_neighbors = 0;
        std::vector<std::pair<int, int>> unknown_list;

        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            int ni = i + di;
            int nj = j + dj;
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && (di != 0 || dj != 0)) {
              if (current_map[ni][nj] == '?') {
                unknown_neighbors++;
                unknown_list.push_back({ni, nj});
              } else if (is_mine[ni][nj]) {
                marked_neighbors++;
              }
            }
          }
        }

        // If number of unknown neighbors equals the remaining mines, mark them all
        int remaining_mines = num - marked_neighbors;
        if (remaining_mines == unknown_neighbors && remaining_mines > 0) {
          for (auto& cell : unknown_list) {
            is_mine[cell.first][cell.second] = true;
            Execute(cell.first, cell.second, 1);  // Mark as mine
            return;
          }
        }

        // If all mines are found, explore the rest
        if (remaining_mines == 0 && unknown_neighbors > 0) {
          for (auto& cell : unknown_list) {
            is_safe[cell.first][cell.second] = true;
            Execute(cell.first, cell.second, 0);  // Visit
            return;
          }
        }
      }
    }
  }

  // Try auto-explore on visited cells with numbers
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (current_map[i][j] >= '1' && current_map[i][j] <= '8') {
        // Check if auto-explore might work
        int num = current_map[i][j] - '0';
        int marked_neighbors = 0;

        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            int ni = i + di;
            int nj = j + dj;
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && (di != 0 || dj != 0)) {
              if (is_mine[ni][nj]) {
                marked_neighbors++;
              }
            }
          }
        }

        if (marked_neighbors == num) {
          Execute(i, j, 2);  // Auto-explore
          return;
        }
      }
    }
  }

  // If no logical moves found, pick a random unknown cell
  std::vector<std::pair<int, int>> unknown_cells;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (current_map[i][j] == '?') {
        unknown_cells.push_back({i, j});
      }
    }
  }

  if (!unknown_cells.empty()) {
    int idx = std::rand() % unknown_cells.size();
    Execute(unknown_cells[idx].first, unknown_cells[idx].second, 0);
  }
}

#endif