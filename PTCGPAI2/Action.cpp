#include "Action.hpp"

#include "GameState.hpp"
#include "Game.hpp"
#include "types.hpp"

Action::Action(ActionType type) : type(type) {}
Action::Action(ActionType type, shared_ptr<Card> card) : type(type), targetCard(card) {}
Action::Action(ActionType type, shared_ptr<ActivePokemon> targetPokemon) : type(type), targetPokemon(targetPokemon) {}
Action::Action(ActionType type, Attack targetAttack) : type(type), targetAttack(targetAttack) {}

// Function definition
void Action::display() const {
    switch (type) {
    case ActionType::PLAY:
        cout << "Play " << targetCard->name << endl;
        break;
    case ActionType::ATTACK:
        cout << "Attack with " << targetAttack.name << endl;
        break;
    case ActionType::END_TURN:
        cout << "End turn" << endl;
        break;
    case ActionType::ENERGY:
        cout << "Attach energy to " << targetPokemon->pokemonCard->name << endl;
        break;
    case ActionType::ROOT:
        cout << "ROOT CASE" << endl;
        break;
    }
}

ActionNode::ActionNode(shared_ptr<GameState> state, Action action) : state(state), action(action) {}

pair<shared_ptr<GameState>, vector<Action>> applyAction(shared_ptr<GameState>& currentState, const Action& action) {
    Game newGame = Game(currentState); // Copy the current game state

    // Apply the action based on type
    switch (action.type) {
    case ActionType::PLAY:
        newGame.playPokemon(currentState->currentPlayer, action.targetCard);
        break;
    case ActionType::ATTACK:
        newGame.performAttack(action.targetAttack);
        break;
    case ActionType::ENERGY:
        newGame.attachEnergy(action.targetPokemon);
        break;
    case ActionType::END_TURN:
        newGame.endTurn();
        break;
    case ActionType::ROOT:
        break;
    }

    // Generate the next set of valid actions from the new state
    vector<Action> nextValidActions = newGame.getValidActions();
    shared_ptr<GameState> newState = newGame.getGameState();

    return { newState, nextValidActions };
}

vector<shared_ptr<ActionNode>> generateActionTree(const shared_ptr<GameState>& currentState, vector<Action> validActions) {
    vector<shared_ptr<ActionNode>> actionNodes;

    for (const Action& action : validActions) {
        shared_ptr<GameState> nextState = currentState; // Copy current state
        applyAction(nextState, action);  // Modify the state with the action

        // Create a new node for this action and add it to the tree
        actionNodes.push_back(make_shared<ActionNode>(nextState, action));
    }

    return actionNodes;
}

// Recursively build the action tree up to a specified depth
void buildActionTree(shared_ptr<ActionNode> node, int depth, const vector<Action>& validActions) {
    if (depth == 0) return;  // Stop expanding after a set depth

    // Generate possible actions from the current state
    vector<shared_ptr<ActionNode>> children = generateActionTree(node->state, validActions);
    node->children = children;

    // Recursively expand the children
    for (auto& child : children) {
        buildActionTree(child, depth - 1, validActions);  // Recursively expand
    }
}

void displayActionTree(const shared_ptr<ActionNode>& node, int depth) {
    // Base case: stop if there's no node
    if (!node) return;

    // Indentation based on depth to make the tree visually hierarchical
    std::string indent(depth * 2, ' ');  // Indentation is two spaces per depth level

    // Display the current action
    node->action.display();

    // Recurse through the children
    for (const auto& child : node->children) {
        displayActionTree(child, depth + 1);  // Increase depth for child nodes
    }
}