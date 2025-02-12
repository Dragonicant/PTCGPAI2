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
    case ActionType::BENCH:
        cout << COLOR_GREEN << "Promote " << targetPokemon->pokemonCard->name << " to active spot" << COLOR_RESET << endl;
        break;
    case ActionType::ROOT:
        cout << COLOR_MAGENTA << "ROOT CASE" << COLOR_RESET << endl;
        break;
    }
}

ActionNode::ActionNode(shared_ptr<GameState> state, Action action) : state(state), action(action) {}

string displayActionName(shared_ptr<ActionNode> node) {
    switch (node->action.type) {
    case ActionType::ROOT: return "Root";
    case ActionType::PLAY: return "Play";
    case ActionType::ATTACK: return "Attack";
    case ActionType::END_TURN: return "End Turn";
    case ActionType::ENERGY: return "Energy";
    case ActionType::BENCH: return "Play from Bench";
    default: return "Unknown";
    }
}

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
    case ActionType::BENCH:
        game.playPokemonFromBench(game.getGameState()->currentPlayer, action.targetPokemon);
        break;
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
void buildActionTree(shared_ptr<ActionNode> node, int maxTurns, int currentTurn, const vector<Action>& validActions) {
    // Base case: stop if we've reached the maximum number of turns
    if (currentTurn >= maxTurns) {
        return;
    }

    // Check if a forced action is required
    bool forcedActionRequired = isForcedActionRequired(node->state);

    if (forcedActionRequired) {
        // Generate only the actions that satisfy the forced action
        vector<Action> forcedActions = getForcedActions(node->state);

        for (const Action& action : forcedActions) {
            //cout << "Turn " << currentTurn << ": Processing FORCED action ";
            //action.display();

            // Apply the forced action to create a new game state
            auto [newState, nextValidActions] = applyAction(node->state, action);

            // Create a new child node for this forced action
            auto child = make_shared<ActionNode>(newState, action);
            node->children.push_back(child);

            // Recursively build the tree for the next state
            buildActionTree(child, maxTurns, currentTurn, nextValidActions);
        }
    }
    else {
        // No forced action required; process all valid actions
        for (const Action& action : validActions) {

            // Apply the action to create a new game state
            auto [newState, nextValidActions] = applyAction(node->state, action);

            // Create a new child node for this action
            auto child = make_shared<ActionNode>(newState, action);
            node->children.push_back(child);

            // If the action ends the turn, increment the turn counter
            int nextTurn = currentTurn;
            if (action.type == ActionType::END_TURN) {
                nextTurn++;
            }

            // Recursively build the tree for the next state, but check if the game is over
            if (!newState->gameOver) {
                buildActionTree(child, maxTurns, nextTurn, nextValidActions);
            }
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

bool isForcedActionRequired(const shared_ptr<GameState>& state) {
    // Check if the active spot is empty for the current player
    return state->playerActiveSpots[state->currentPlayer] == nullptr;
}
vector<Action> getForcedActions(const shared_ptr<GameState>& state) {
    vector<Action> forcedActions;

    // Scenario 1: At the start of the game (no Pokémon on bench)
    if (state->playerBenchSpots[state->currentPlayer].empty()) {
        // Get the current player's hand
        const auto& hand = state->playerHands[state->currentPlayer];

        // Generate actions for playing a Basic Pokémon from hand to the active spot
        for (const auto& card : hand) {
            if (card->stage == 0) { // Ensure it's a Basic Pokémon
                forcedActions.push_back(Action(ActionType::PLAY, card));
            }
        }
    }
    // Scenario 2: During the game (active Pokémon was knocked out)
    else {
        // Get the current player's bench
        const auto& bench = state->playerBenchSpots[state->currentPlayer];

        // Generate actions for promoting a Pokémon from the bench to the active spot
        for (const auto& card : bench) {
            forcedActions.push_back(Action(ActionType::BENCH, card));
        }
    }

    return forcedActions;
}