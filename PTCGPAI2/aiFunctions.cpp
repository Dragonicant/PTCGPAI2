#include "aiFunctions.hpp"
#include "GameState.hpp"
#include "Action.hpp"
#include "Game.hpp"
#include "utilities.hpp"

#include <memory>

int evaluateGameState(const shared_ptr<GameState>& state, int currentPlayer) {
    int opponent = 1 - currentPlayer;

    int score = 0;

    // Give points for player's progress
    score += state->playerPoints[currentPlayer] * 100;
    score -= state->playerPoints[opponent] * 100;  // Opponent gaining points is bad

    // Reward having benched Pok�mon (more options later)
    score += state->playerBenchSpots[currentPlayer].size() * 10;
    score -= state->playerBenchSpots[opponent].size() * 10;

    // Reward damage dealt (assuming 'damageDealt' holds total damage by player)
    if (state->playerActiveSpots[currentPlayer] != nullptr) {
        score -= state->playerActiveSpots[currentPlayer]->pokemonCard->hp - state->playerActiveSpots[currentPlayer]->currentHP;
    }
    else {
        printError("ERROR NO ACTIVE POKEKMON");
        displayGameState(state);
    }

    if (state->playerActiveSpots[opponent] != nullptr) {
        score += state->playerActiveSpots[opponent]->pokemonCard->hp - state->playerActiveSpots[opponent]->currentHP;
    }
    else {
        printError("ERROR NO ACTIVE POKEKMON");
        displayGameState(state);
    }

    return score;
}

pair<int, Action> minimax(shared_ptr<ActionNode> node, int depth, bool maximizingPlayer, int currentPlayer) {
    if (depth == 0 || node->children.empty()) {
        int evaluation = evaluateGameState(node->state, currentPlayer);
        return { evaluation, node->action };
    }
    if (maximizingPlayer) {
        int maxEval = INT_MIN;
        Action bestAction = node->children[0]->action;
        for (auto& child : node->children) {
            int eval = minimax(child, depth - 1, true, currentPlayer).first;
            if (eval > maxEval) {
                maxEval = eval;
                bestAction = child->action;
            }
        }
        return { maxEval, bestAction };
    }
    else { // Opponent's turn (minimizing)
        int minEval = INT_MAX;
        Action worstAction = node->children[0]->action;

        for (auto& child : node->children) {
            int eval = minimax(child, depth - 1, false, currentPlayer).first;
            if (eval < minEval) {
                minEval = eval;
                worstAction = child->action;
            }
        }
        return { minEval, worstAction };
    }
}

Action findBestAction(shared_ptr<ActionNode> rootNode, int depth, int currentPlayer) {
    return minimax(rootNode, depth, true, currentPlayer).second;
}
