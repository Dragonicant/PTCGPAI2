#pragma once

#include <memory>

//forward declarations
struct GameState;
struct Action;
struct ActionNode;

int evaluateGameState(const std::shared_ptr<GameState>& state, int currentPlayer);

std::pair<int, Action> minimax(std::shared_ptr<ActionNode> node, int depth, bool maximizingPlayer, int currentPlayer);

Action findBestAction(std::shared_ptr<ActionNode> rootNode, int depth, int currentPlayer);