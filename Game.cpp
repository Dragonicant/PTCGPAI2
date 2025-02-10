#include "Game.hpp"
#include "deck.hpp"
#include "Action.hpp"
#include "GameState.hpp"

#include <random>
#include <iostream>
#include <algorithm>

using namespace std;

// Constructor for ActivePokemon, initializes currentHP from the pokemonCard's HP
ActivePokemon::ActivePokemon(std::shared_ptr<Card> card)
    : pokemonCard(card), currentHP(card->hp), currentEnergy() {  // Assuming Card has an `hp` member
}

// Method to add energy to the Pokemon's energy pool
void ActivePokemon::addEnergy(char energyType) {
    currentEnergy.push_back(energyType);
}

// Method to remove energy from the Pokemon's energy pool
void ActivePokemon::removeEnergy(char energyType) {
    auto it = std::find(currentEnergy.begin(), currentEnergy.end(), energyType);
    if (it != currentEnergy.end()) {
        currentEnergy.erase(it);
    }
}

// Display the active Pokemon's information
void ActivePokemon::displayActivePokemon() const {
    std::cout << "Active Pokemon: " << pokemonCard->name << std::endl;
    std::cout << "HP: " << currentHP << std::endl;
    std::cout << "Energy: ";
    for (char energy : currentEnergy) {
        std::cout << energy << " ";
    }
    std::cout << std::endl;
}

Game::Game(shared_ptr<Deck> player1Deck, shared_ptr<Deck> player2Deck) {
    // Create shallow copies of the player decks for use in the game state
    playerDecks[0] = player1Deck;
    playerDecks[1] = player2Deck;

    // Initialize GameState with shallow copies of the decks
    for (int i = 0; i < 2; i++) {
        for (const auto& card : playerDecks[i]->cards) {
            gameDecks[i].push_back(card);  // Copy each card into the game's deck
        }
    }

    for (int i = 0; i < 2; i++) {
        // Initialize points for both players
        playerPoints[i] = 0;
        // Set energy availible to X (placeholder value) for safety
        playerAvailableEnergy[i] = 'X';
    }

    // Randomly select who goes first
    random_device rd;
    default_random_engine rng(rd());
    currentPlayer = uniform_int_distribution<int>(0, 1)(rng);  // Randomly pick 0 or 1 for first player

    cout << "Player " << currentPlayer + 1 << " will go first!" << endl;

    // Draw 5 cards for each player
    drawInitialCards(0);  // Draw 5 cards for Player 1
    drawInitialCards(1);  // Draw 5 cards for Player 2
}

Game::Game(std::shared_ptr<GameState>& state) {
    // Restore player points
    playerPoints[0] = state->playerPoints[0];
    playerPoints[1] = state->playerPoints[1];

    // Restore hands
    playerHands[0] = state->playerHands[0];
    playerHands[1] = state->playerHands[1];

    // Restore active Pokémon
    playerActiveSpots[0] = state->playerActiveSpots[0];
    playerActiveSpots[1] = state->playerActiveSpots[1];

    // Restore bench Pokémon
    playerBenchSpots[0] = state->playerBenchSpots[0];
    playerBenchSpots[1] = state->playerBenchSpots[1];

    // Restore available energy
    playerAvailableEnergy[0] = state->playerAvailableEnergy[0];
    playerAvailableEnergy[1] = state->playerAvailableEnergy[1];

    // Restore decks
    playerDecks[0] = make_shared<Deck>();
    playerDecks[1] = make_shared<Deck>();
    playerDecks[0]->cards = state->playerDecks[0];
    playerDecks[1]->cards = state->playerDecks[1];

    gameDecks[0] = state->gameDecks[0];
    gameDecks[1] = state->gameDecks[1];

    // Restore game state
    gameOver = state->gameOver;
    currentPlayer = state->currentPlayer;
    winner = state->winner;
}

std::shared_ptr<GameState> Game::getGameState() {
    auto state = make_shared<GameState>();

    // Set player points
    state->playerPoints[0] = playerPoints[0];
    state->playerPoints[1] = playerPoints[1];

    // Set hand sizes
    state->playerHandSize[0] = playerHands[0].size();
    state->playerHandSize[1] = playerHands[1].size();

    // Set active Pokémon
    state->playerActiveSpots[0] = playerActiveSpots[0];
    state->playerActiveSpots[1] = playerActiveSpots[1];

    // Set bench Pokémon
    state->playerBenchSpots[0] = playerBenchSpots[0];
    state->playerBenchSpots[1] = playerBenchSpots[1];

    // Set available energy
    state->playerAvailableEnergy[0] = playerAvailableEnergy[0];
    state->playerAvailableEnergy[1] = playerAvailableEnergy[1];

    // Set player hands
    state->playerHands[0] = playerHands[0];
    state->playerHands[1] = playerHands[1];

    // Set decks
    state->playerDecks[0] = playerDecks[0]->cards;
    state->playerDecks[1] = playerDecks[1]->cards;
    state->gameDecks[0] = gameDecks[0];
    state->gameDecks[1] = gameDecks[1];

    // Set game-over status
    state->gameOver = gameOver;

    // Set current player
    state->currentPlayer = currentPlayer;

    // Set winner
    state->winner = winner;

    return state;
}

// Function to check if a player has no Pokemon left (active or bench)
bool Game::hasNoPokemon(int player) {
    return playerActiveSpots[player] == nullptr && playerBenchSpots[player].empty();
}

// Function to check for a winner (either 3 points or no Pokemon left for a player)
void Game::checkForWinner() {
    if (playerPoints[0] >= 3) {
        cout << "Player 1 wins with 3 points!" << endl;
        winner = 0;  // Set winner to Player 1
        gameOver = true;
    }
    else if (playerPoints[1] >= 3) {
        cout << "Player 2 wins with 3 points!" << endl;
        winner = 1;  // Set winner to Player 2
        gameOver = true;
    }
    else if (hasNoPokemon(0)) {
        cout << "Player 1 has no Pokemon left. Player 2 wins!" << endl;
        winner = 1;  // Set winner to Player 2
        gameOver = true;
    }
    else if (hasNoPokemon(1)) {
        cout << "Player 2 has no Pokemon left. Player 1 wins!" << endl;
        winner = 0;  // Set winner to Player 1
        gameOver = true;
    }
}

vector<Action> Game::getValidActions() {
    vector<Action> validActions;

    // Check if the player can play a Pokemon card (they have cards in hand and space in active or bench)
    if (!playerHands[currentPlayer].empty()) {
        bool canPlayToActive = playerActiveSpots[currentPlayer] == nullptr;  // Active spot must be empty
        bool canPlayToBench = playerBenchSpots[currentPlayer].size() < 3;  // Bench must have fewer than 3 Pokemon

        if (canPlayToActive || canPlayToBench) {
            for (size_t i = 0; i < playerHands[currentPlayer].size(); ++i) {
                validActions.push_back(Action(ActionType::PLAY, playerHands[currentPlayer].at(i)));
            }
        }
    }

    //Attaching energy actions
    if (playerAvailableEnergy[currentPlayer] != 'X')
        for (auto& pokemon : playerBenchSpots[currentPlayer]) {
            validActions.push_back(Action(ActionType::ENERGY, pokemon));
        }
    if (playerActiveSpots[currentPlayer] != nullptr) {
        validActions.push_back(Action(ActionType::ENERGY, playerActiveSpots[currentPlayer]));
    }

    //Attacking actions
    if (playerActiveSpots[currentPlayer]) {
        shared_ptr<ActivePokemon> activePokemon = playerActiveSpots[currentPlayer];

        // Assume the Pokemon has one main attack with a fixed energy requirement (simplified)
        vector<EnergyRequirement> attackCost = activePokemon->pokemonCard->attacks.at(0).energyRequirement;

        // Check if the active Pokemon has the required energy
        bool hasEnoughEnergy = true;
        vector<char> tempEnergy = activePokemon->currentEnergy; // Copy energy pool for manipulation

        for (const EnergyRequirement& requirement : attackCost) {
            if (requirement.type == 'X') {
                // X energy can be satisfied by any energy
                if (tempEnergy.size() < requirement.amount) {
                    hasEnoughEnergy = false;
                    break;
                }
                // Remove any energies to satisfy the X cost
                tempEnergy.erase(tempEnergy.begin(), tempEnergy.begin() + requirement.amount);
            }
            else {
                // Count required type
                int availableCount = count(tempEnergy.begin(), tempEnergy.end(), requirement.type);
                if (availableCount < requirement.amount) {
                    hasEnoughEnergy = false;
                    break;
                }
                // Remove used energy
                int removed = 0;
                tempEnergy.erase(remove_if(tempEnergy.begin(), tempEnergy.end(),
                    [&](char e) { return e == requirement.type && removed++ < requirement.amount; }),
                    tempEnergy.end());
            }
        }

        if (hasEnoughEnergy) {
            validActions.push_back(Action(ActionType::ATTACK, activePokemon->pokemonCard->attacks.at(0)));
        }
    }

    // Check if the player can end their turn (they always can if no other action is required)
    validActions.push_back(Action(ActionType::END_TURN));

    return validActions;
}

// Function to display the valid actions for a player
void Game::displayValidActions() {
    vector<Action> actions = getValidActions();

    cout << "Player " << currentPlayer + 1 << " can perform the following actions:" << endl;
    for (const Action& action : actions) {
        switch (action.type) {
        case ActionType::PLAY:
            cout << "Play " << action.targetCard->name << endl;
            break;
        case ActionType::ATTACK:
            cout << "Attack with " << playerActiveSpots[currentPlayer]->pokemonCard->name << " using " << action.targetAttack.name << " for " << action.targetAttack.damage << endl;
            break;
        case ActionType::ENERGY:
            cout << "Attach " << playerAvailableEnergy[currentPlayer] << " to " << action.targetPokemon->pokemonCard->name << endl;
            break;
        case ActionType::END_TURN:
            cout << "End turn" << endl;
            break;

        }
    }
}

void Game::shuffleDeck(int player) {
    shuffle(gameDecks[player].begin(), gameDecks[player].end(), default_random_engine(random_device()()));
    cout << "Player " << player + 1 << "'s deck has been shuffled.\n";
}

// Function to draw 5 cards for a player
void Game::drawInitialCards(int player) {
    shuffleDeck(player);
    for (int i = 0; i < 5; ++i) {
        shared_ptr<Card> drawnCard = drawCard(player);
        if (drawnCard) {
            playerHands[player].push_back(drawnCard);  // Add drawn card to player's hand
        }
    }

    cout << "Player " << player + 1 << " has drawn 5 cards." << endl;
}

// Draws a card from the deck and returns it
shared_ptr<Card> Game::drawCard(int player) {
    if (gameDecks[player].empty()) {
        cout << "Player " << player + 1 << "'s deck is empty.\n";
        return nullptr;
    }
    shared_ptr<Card> cardToDraw = gameDecks[player].back();
    gameDecks[player].pop_back();  // Remove the card from the deck
    return cardToDraw;
}

// Method to show each player's hand
void Game::showHands() const {
    for (int i = 0; i < 2; ++i) {
        cout << "Player " << i + 1 << " hand:" << endl;
        for (const auto& card : playerHands[i]) {
            cout << card->name << endl;  // Assuming Card has a name field
        }
        cout << endl;
    }
}

//overload to allow playing card by index
bool Game::playPokemon(int player, int cardFromHand) {
    return playPokemon(player, playerHands[player].at(cardFromHand));
}

// Function to play a Pokemon card
bool Game::playPokemon(int player, shared_ptr<Card> card) {
    // Ensure the card exists in the player's hand
    auto it = find_if(playerHands[player].begin(), playerHands[player].end(),
        [&card](const shared_ptr<Card>& c) { return c == card; });

    if (it == playerHands[player].end()) {
        cout << "Player " << player + 1 << " does not have the specified card in hand." << endl;
        return false;
    }

    // Check if there is an open spot in the player's active or bench positions
    if (playerActiveSpots[player] == nullptr && playerBenchSpots[player].size() < 5) {
        // If no Pokemon is in the active spot, create an ActivePokemon and place it there
        playerActiveSpots[player] = make_shared<ActivePokemon>(card); // Assume the original card has an `hp` field
        cout << "Player " << player + 1 << " played " << card->name << " to their active spot." << endl;
    }
    else if (playerActiveSpots[player] != nullptr && playerBenchSpots[player].size() < 5) {
        // If there is a Pokemon in the active spot, create an ActivePokemon and place it on the bench
        playerBenchSpots[player].push_back(make_shared<ActivePokemon>(card));
        cout << "Player " << player + 1 << " played " << card->name << " to their bench." << endl;
    }
    else {
        // No space to play Pokemon
        cout << "Player " << player + 1 << " cannot play " << card->name << " due to no available spots." << endl;
        return false;
    }

    // Remove the card from the player's hand
    removeCardFromHand(player, card);
    return true;
}

bool Game::attachEnergy(shared_ptr<ActivePokemon> targetPokemon) {
    // Check if the player has energy available
    if (playerAvailableEnergy[currentPlayer] == 'X') {
        cout << "Player " << currentPlayer + 1 << " does not have energy available.\n";
        return false;
    }

    // Add energy to the chosen Pokemon
    targetPokemon->currentEnergy.push_back(playerAvailableEnergy[currentPlayer]);

    cout << "Player " << currentPlayer + 1 << " attached " << playerAvailableEnergy[currentPlayer] << " energy to " << targetPokemon->pokemonCard->name << ".\n";
    playerAvailableEnergy[currentPlayer] = 'X';
    return true;
}

void Game::performAttack(Attack attack) {
    shared_ptr<ActivePokemon> attacker = playerActiveSpots[currentPlayer];
    shared_ptr<ActivePokemon> defender = playerActiveSpots[1 - currentPlayer];

    if (!attacker || !defender) {
        cout << "Attack not possible: One or both active Pokemon are missing!" << endl;
        return;
    }

    string attackName = attack.name;  // Use the provided attack
    int damage = attack.damage;       // Use the damage from the provided attack

    cout << "Player " << currentPlayer + 1 << "'s " << attacker->pokemonCard->name
        << " attacks " << defender->pokemonCard->name
        << " using " << attackName << " for " << damage << " damage!" << endl;

    // Reduce defender's HP
    defender->currentHP -= damage;
    if (defender->currentHP <= 0) {
        cout << defender->pokemonCard->name << " is knocked out!" << endl;
        playerPoints[currentPlayer]++;

        // Remove the defeated Pokemon
        playerActiveSpots[1 - currentPlayer] = nullptr;

        // Check if the opponent has any Pokemon left
        if (playerBenchSpots[1 - currentPlayer].empty()) {
            cout << "Player " << currentPlayer + 1 << " wins the game!" << endl;
            gameOver = true;
            winner = currentPlayer;
        }
        else {
            // Promote a Pokemon from the bench to active
            playerActiveSpots[1 - currentPlayer] = playerBenchSpots[1 - currentPlayer].front();
            playerBenchSpots[1 - currentPlayer].erase(playerBenchSpots[1 - currentPlayer].begin());
            cout << playerActiveSpots[1 - currentPlayer]->pokemonCard->name << " moves to the active spot!" << endl;
        }
    }
}

// Method to remove the card from the player's hand
void Game::removeCardFromHand(int player, shared_ptr<Card> cardToRemove) {
    auto& hand = playerHands[player];
    auto it = find(hand.begin(), hand.end(), cardToRemove);
    if (it != hand.end()) {
        hand.erase(it);
    }
}

// Function to display the board with the specific ASCII art pattern
void Game::displayBoard() const {
    // Display Player 2's Bench (top row)
    cout << (playerBenchSpots[1].size() > 0 ? playerBenchSpots[1][0]->pokemonCard->name : "Empty") << "  "
        << (playerBenchSpots[1].size() > 1 ? playerBenchSpots[1][1]->pokemonCard->name : "Empty") << "  "
        << (playerBenchSpots[1].size() > 2 ? playerBenchSpots[1][2]->pokemonCard->name : "Empty") << endl;

    // Display Player 2's Active (middle row)
    cout << "        " << (playerActiveSpots[1] != nullptr ? playerActiveSpots[1]->pokemonCard->name : "Empty") << endl;

    // Display Player 1's Active (middle row)
    cout << "        " << (playerActiveSpots[0] != nullptr ? playerActiveSpots[0]->pokemonCard->name : "Empty") << endl;

    // Display Player 1's Bench (bottom row)
    cout << (playerBenchSpots[0].size() > 0 ? playerBenchSpots[0][0]->pokemonCard->name : "Empty") << "  "
        << (playerBenchSpots[0].size() > 1 ? playerBenchSpots[0][1]->pokemonCard->name : "Empty") << "  "
        << (playerBenchSpots[0].size() > 2 ? playerBenchSpots[0][2]->pokemonCard->name : "Empty") << endl;
}

// Method to start a new turn for the player
void Game::endTurn() {
    cout << "Player " << currentPlayer + 1 << "'s turn has ended." << endl;
    playerAvailableEnergy[currentPlayer] = 'X';  // Clear the available energy
    // Change turn to the next player
    currentPlayer = (currentPlayer + 1) % 2;

    // Add energy randomly from selected energy types for the current player
    addEnergyToPlayer(currentPlayer);
}


// Function to add energy to the current player
void Game::addEnergyToPlayer(int player) {
    // Get the set of energy types selected in the player's deck
    const auto& energyTypes = playerDecks[player]->getEnergyTypes();

    // Randomly select an energy type from the player's deck energy types
    if (!energyTypes.empty()) {
        char selectedEnergy = energyTypes[rand() % energyTypes.size()];  // Choose a random energy type
        playerAvailableEnergy[player] = selectedEnergy;  // Add the selected energy to the player's available energy
    }
}