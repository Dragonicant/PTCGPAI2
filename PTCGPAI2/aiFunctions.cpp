#include "aiFunctions.hpp"
#include "GameState.hpp"
#include "Action.hpp"
#include "Game.hpp"

#include <memory>

int evaluateGameState(const shared_ptr<GameState>& state, int currentPlayer) {
    int opponent = 1 - currentPlayer;

    int score = 0;

    // Give points for player's progress
    score += state->playerPoints[currentPlayer] * 100;
    score -= state->playerPoints[opponent] * 100;  // Opponent gaining points is bad

    // Reward having benched Pokémon (more options later)
    score += state->playerBenchSpots[currentPlayer].size() * 10;
    score -= state->playerBenchSpots[opponent].size() * 10;

    // Reward damage dealt (assuming 'damageDealt' holds total damage by player)
    score += state->playerActiveSpots[opponent]->pokemonCard->hp - state->playerActiveSpots[opponent]->currentHP;
    score -= state->playerActiveSpots[currentPlayer]->pokemonCard->hp - state->playerActiveSpots[currentPlayer]->currentHP;

    return score;
}

pair<int, Action> minimax(shared_ptr<ActionNode> node, int depth, bool maximizingPlayer, int currentPlayer) {
    if (depth == 0 || node->children.empty()) {
        int evaluation = evaluateGameState(node->state, currentPlayer);
        cout << "Leaf Node Score: " << evaluation << " " << displayActionName(node) << endl;
        return { evaluation, node->action };
    }

    if (maximizingPlayer) {
        int maxEval = INT_MIN;
        Action bestAction = node->children[0]->action;
        cout << "Checking all children of";
        node->action.display();
        displayActionTree(node);
        for (auto& child : node->children) {
            cout << "Evaluating ";
            child->action.display();
            int eval = minimax(child, depth - 1, false, currentPlayer).first;
            cout << eval << endl;
            if (eval > maxEval) {
                maxEval = eval;
                bestAction = child->action;
            }
        }
        bestAction.display();
        return { maxEval, bestAction };
    }
    else { // Opponent's turn (minimizing)
        int minEval = INT_MAX;
        Action worstAction = node->children[0]->action;

        for (auto& child : node->children) {
            int eval = minimax(child, depth - 1, true, currentPlayer).first;
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
