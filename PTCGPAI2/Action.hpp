#ifndef ACTION_HPP
#define ACTION_HPP

#include <memory>
#include <vector>

#include "types.hpp"

// Forward declaration to avoid circular dependency
class ActivePokemon;
struct GameState;
class Card;

enum class ActionType { PLAY, ATTACK, END_TURN, ENERGY, ROOT };

struct Action {
    ActionType type;    
    std::shared_ptr<Card> targetCard;  // for use with PLAY
    std::shared_ptr<ActivePokemon> targetPokemon; // for use with ENERGY
    Attack targetAttack; // for use with ATTACK

    Action(ActionType type);
    Action(ActionType type, std::shared_ptr<Card> card);
    Action(ActionType type, std::shared_ptr<ActivePokemon> targetPokemon);
    Action(ActionType type, Attack targetAttack);

    void display() const;
};

struct ActionNode {
    std::shared_ptr<GameState> state;
    Action action;
    std::vector<std::shared_ptr<ActionNode>> children;

    ActionNode(std::shared_ptr<GameState> state, Action action);
};

std::pair<std::shared_ptr<GameState>, std::vector<Action>> applyAction(const std::shared_ptr<GameState>& currentState, const Action& action);

std::vector<std::shared_ptr<ActionNode>> generateActionTree(const std::shared_ptr<GameState>& currentState, std::vector<Action> validActions);

void buildActionTree(std::shared_ptr<ActionNode> node, int depth, const std::vector<Action>& validActions);

void displayActionTree(const std::shared_ptr<ActionNode>& node, int depth);

#endif // ACTION_HPP