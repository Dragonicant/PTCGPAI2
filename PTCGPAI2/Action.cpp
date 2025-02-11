#include "Action.hpp"

#include "GameState.hpp"
#include "Game.hpp"
#include "types.hpp"

Action::Action(ActionType type) : type(type) {}
Action::Action(ActionType type, shared_ptr<Card> card) : type(type), targetCard(card) {}
Action::Action(ActionType type, shared_ptr<ActivePokemon> targetPokemon) : type(type), targetPokemon(targetPokemon) {}
Action::Action(ActionType type, Attack targetAttack) : type(type), targetAttack(targetAttack) {}

// Define ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

void Action::display() const {
    switch (type) {
    case ActionType::PLAY:
        cout << COLOR_GREEN << "Play " << targetCard->name << COLOR_RESET << endl;
        break;
    case ActionType::ATTACK:
        cout << COLOR_RED << "Attack with " << targetAttack.name << COLOR_RESET << endl;
        break;
    case ActionType::END_TURN:
        cout << COLOR_YELLOW << "End turn" << COLOR_RESET << endl;
        break;
    case ActionType::ENERGY:
        cout << COLOR_BLUE << "Attach energy to " << targetPokemon->pokemonCard->name << COLOR_RESET << endl;
        break;
    case ActionType::ROOT:
        cout << COLOR_MAGENTA << "ROOT CASE" << COLOR_RESET << endl;
        break;
    }
}

ActionNode::ActionNode(shared_ptr<GameState> state, Action action) : state(state), action(action) {}

void applyAction(Game& game, const Action& action) {
    // Apply the action based on type
    switch (action.type) {
    case ActionType::PLAY:
        game.playPokemon(game.getGameState()->currentPlayer, action.targetCard);
        break;
    case ActionType::ATTACK:
        game.performAttack(action.targetAttack);
        break;
    case ActionType::ENERGY: {
        shared_ptr<ActivePokemon> newTargetPokemon = nullptr;

        // Check active spot
        auto activeSpot = game.getPlayerActiveSpot(game.getGameState()->currentPlayer);
        if (activeSpot && activeSpot->pokemonCard == action.targetPokemon->pokemonCard) {
            newTargetPokemon = activeSpot;
        }

        // Check bench spots
        if (!newTargetPokemon) {
            auto benchSpots = game.getPlayerBenchSpots(game.getGameState()->currentPlayer);
            for (const auto& pokemon : benchSpots) {
                if (pokemon->pokemonCard == action.targetPokemon->pokemonCard) {
                    newTargetPokemon = pokemon;
                    break;
                }
            }
        }

        if (newTargetPokemon) {
            game.attachEnergy(newTargetPokemon);
        }
        else {
            cerr << "Error: Target Pokemon not found in new state!" << endl;
        }
        break;
    }
    case ActionType::END_TURN:
        game.endTurn();
        break;
    case ActionType::ROOT:
        break;
    }
}

pair<shared_ptr<GameState>, vector<Action>> applyAction(const shared_ptr<GameState>& currentState, const Action& action) {
    // Create a new game state from the current state
    Game newGame(currentState, true); // Silent mode enabled   
    
    // Apply the action based on type
    switch (action.type) {
    case ActionType::PLAY:
        newGame.playPokemon(currentState->currentPlayer, action.targetCard);
        break;
    case ActionType::ATTACK:
        newGame.performAttack(action.targetAttack);
        break;
    case ActionType::ENERGY: {
        // Find the corresponding targetPokemon in the new state
        shared_ptr<ActivePokemon> newTargetPokemon = nullptr;

        // Check active spot
        auto activeSpot = newGame.getPlayerActiveSpot(currentState->currentPlayer);
        if (activeSpot &&
            activeSpot->pokemonCard ==
            action.targetPokemon->pokemonCard) {
            newTargetPokemon = activeSpot;
        }

        // Check bench spots
        if (!newTargetPokemon) {
            auto benchSpots = newGame.getPlayerBenchSpots(currentState->currentPlayer);
            for (const auto& pokemon : benchSpots) {
                if (pokemon->pokemonCard == action.targetPokemon->pokemonCard) {
                    newTargetPokemon = pokemon;
                    break;
                }
            }
        }

        if (newTargetPokemon) {
            newGame.attachEnergy(newTargetPokemon);
        }
        else {
            cerr << "Error: Target Pokemon not found in new state!" << endl;
        }
        break;
    }
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
        // Apply the action and get the new state and next valid actions
        auto [nextState, nextValidActions] = applyAction(currentState, action);

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

    // Recursively expand the children, unless the action is END_TURN
    for (auto& child : children) {
        if (child->action.type != ActionType::END_TURN && child->action.type != ActionType::ATTACK) {
            // Get the next valid actions for this child's state
            auto [nextState, nextValidActions] = applyAction(node->state, child->action);

            // Recursively expand with the new valid actions
            buildActionTree(child, depth - 1, nextValidActions);
        }
    }
}

// Overloaded function for calling display without knowing depth
void displayActionTree(const shared_ptr<ActionNode>& node) {
    if (!node) return; // Handle empty tree

    // Find the maximum depth of the tree
    int maxDepth = findMaxDepth(node);

    // Call the original function with the calculated depth
    displayActionTree(node, maxDepth, "");
}

void displayActionTree(const shared_ptr<ActionNode>& node, int depth, const string& prefix) {
    if (!node) return;

    // Display the current action with appropriate indentation and prefix
    cout << prefix;
    if (depth > 0) {
        cout << (node->children.empty() ? "`-- " : "|-- ");
    }
    node->action.display();

    // Recurse through the children
    for (size_t i = 0; i < node->children.size(); ++i) {
        bool isLastChild = (i == node->children.size() - 1);
        string newPrefix = prefix + (depth > 0 ? (isLastChild ? "    " : "|   ") : "");
        displayActionTree(node->children[i], depth + 1, newPrefix);
    }
}

int findMaxDepth(const shared_ptr<ActionNode>& node) {
    if (!node) return 0; // Base case: empty node has depth 0

    int maxChildDepth = 0;
    for (const auto& child : node->children) {
        int childDepth = findMaxDepth(child);
        if (childDepth > maxChildDepth) {
            maxChildDepth = childDepth;
        }
    }

    return 1 + maxChildDepth; // Add 1 for the current node
}