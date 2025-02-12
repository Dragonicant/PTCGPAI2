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

ActivePokemon::ActivePokemon(const ActivePokemon& other)
    : pokemonCard(other.pokemonCard),  // Shared ownership of the Card
    currentHP(other.currentHP),
    currentEnergy(other.currentEnergy) {
    // No need for deep copy of pokemonCard since it's a shared_ptr
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

Game::Game(shared_ptr<Deck> player1Deck, shared_ptr<Deck> player2Deck, bool silent)
    : silent(silent) {
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

    damageDealt->push_back(0);
    damageDealt->push_back(0);
}

Game::Game(const std::shared_ptr<GameState>& state, bool silent) 
    : silent(silent) {
    // Restore player points
    playerPoints[0] = state->playerPoints[0];
    playerPoints[1] = state->playerPoints[1];

    // Restore hands
    playerHands[0] = state->playerHands[0];
    playerHands[1] = state->playerHands[1];

    // Restore active Pokémon (deep copy)
    playerActiveSpots[0] = state->playerActiveSpots[0] ?
        std::make_shared<ActivePokemon>(*state->playerActiveSpots[0]) : nullptr;
    playerActiveSpots[1] = state->playerActiveSpots[1] ?
        std::make_shared<ActivePokemon>(*state->playerActiveSpots[1]) : nullptr;

    // Restore bench Pokémon (deep copy)
    for (const auto& pokemon : state->playerBenchSpots[0]) {
        playerBenchSpots[0].push_back(std::make_shared<ActivePokemon>(*pokemon));
    }
    for (const auto& pokemon : state->playerBenchSpots[1]) {
        playerBenchSpots[1].push_back(std::make_shared<ActivePokemon>(*pokemon));
    }

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
    damageDealt->push_back(state->damageDealt[0]);
    damageDealt->push_back(state->damageDealt[1]);
}

// Getter for playerActiveSpots
const std::shared_ptr<ActivePokemon>& Game::getPlayerActiveSpot(int player) const {
    return playerActiveSpots[player];
}

// Getter for playerBenchSpots
const std::vector<std::shared_ptr<ActivePokemon>>& Game::getPlayerBenchSpots(int player) const {
    return playerBenchSpots[player];
}

// Set silent mode
void Game::setSilent(bool silent) {
    this->silent = silent;
}

std::shared_ptr<GameState> Game::getGameState() {
    auto state = make_shared<GameState>();

    // Set player points
    state->playerPoints[0] = playerPoints[0];
    state->playerPoints[1] = playerPoints[1];

    // Set hand sizes
    state->playerHandSize[0] = playerHands[0].size();
    state->playerHandSize[1] = playerHands[1].size();

    // Set active Pokémon (deep copy)
    state->playerActiveSpots[0] = playerActiveSpots[0] ?
        std::make_shared<ActivePokemon>(*playerActiveSpots[0]) : nullptr;
    state->playerActiveSpots[1] = playerActiveSpots[1] ?
        std::make_shared<ActivePokemon>(*playerActiveSpots[1]) : nullptr;

    // Set bench Pokémon (deep copy)
    for (const auto& pokemon : playerBenchSpots[0]) {
        state->playerBenchSpots[0].push_back(std::make_shared<ActivePokemon>(*pokemon));
    }
    for (const auto& pokemon : playerBenchSpots[1]) {
        state->playerBenchSpots[1].push_back(std::make_shared<ActivePokemon>(*pokemon));
    }

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

    // Save damageDealt
    state->damageDealt[0] = damageDealt[0].empty() ? 0 : damageDealt[0][0];
    state->damageDealt[1] = damageDealt[1].empty() ? 0 : damageDealt[1][0];

    return state;
}

// Function to check if a player has no Pokemon left (active or bench)
bool Game::hasNoPokemon(int player) {
    return playerActiveSpots[player] == nullptr && playerBenchSpots[player].empty();
}

// Function to check for a winner (either 3 points or no Pokemon left for a player)
void Game::checkForWinner() {
    if (playerPoints[0] >= 3) {
        if(!silent)
            cout << "Player 1 wins with 3 points!" << endl;
        winner = 0;  // Set winner to Player 1
        gameOver = true;
    }
    else if (playerPoints[1] >= 3) {
        if (!silent)
            cout << "Player 2 wins with 3 points!" << endl;
        winner = 1;  // Set winner to Player 2
        gameOver = true;
    }
    else if (hasNoPokemon(0)) {
        if (!silent)
            cout << "Player 1 has no Pokemon left. Player 2 wins!" << endl;
        winner = 1;  // Set winner to Player 2
        gameOver = true;
    }
    else if (hasNoPokemon(1)) {
        if (!silent)
            cout << "Player 2 has no Pokemon left. Player 1 wins!" << endl;
        winner = 0;  // Set winner to Player 1
        gameOver = true;
    }
}

bool Game::isWinner() {
    return gameOver;
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
    if (playerAvailableEnergy[currentPlayer] != 'X') {
        for (auto& pokemon : playerBenchSpots[currentPlayer]) {
            validActions.push_back(Action(ActionType::ENERGY, pokemon));
        }
        if (playerActiveSpots[currentPlayer] != nullptr) {
            validActions.push_back(Action(ActionType::ENERGY, playerActiveSpots[currentPlayer]));
        }
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

    if (!silent)
        cout << "Player " << currentPlayer + 1 << " can perform the following actions:" << endl;
    for (const Action& action : actions) {
        switch (action.type) {
        case ActionType::PLAY:
            if (!silent)
                cout << "Play " << action.targetCard->name << endl;
            break;
        case ActionType::ATTACK:
            if (!silent)
                cout << "Attack with " << playerActiveSpots[currentPlayer]->pokemonCard->name << " using " << action.targetAttack.name << " for " << action.targetAttack.damage << endl;
            break;
        case ActionType::ENERGY:
            if (!silent)
                cout << "Attach " << playerAvailableEnergy[currentPlayer] << " to " << action.targetPokemon->pokemonCard->name << endl;
            break;
        case ActionType::END_TURN:
            if (!silent)
                cout << "End turn" << endl;
            break;

        }
    }
}

void Game::shuffleDeck(int player) {
    shuffle(gameDecks[player].begin(), gameDecks[player].end(), default_random_engine(random_device()()));
    if (!silent)
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

    if (!silent)
        cout << "Player " << player + 1 << " has drawn 5 cards." << endl;
}

// Draws a card from the deck and returns it
shared_ptr<Card> Game::drawCard(int player) {
    if (gameDecks[player].empty()) {
        if (!silent)
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
        if (!silent)
            cout << "Player " << i + 1 << " hand:" << endl;
        for (const auto& card : playerHands[i]) {
            if (!silent)
                cout << card->name << endl;  // Assuming Card has a name field
        }
        if (!silent)
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
        if (!silent)
            cout << "Player " << player + 1 << " does not have the specified card in hand." << endl;
        return false;
    }

    // Check if there is an open spot in the player's active or bench positions
    if (playerActiveSpots[player] == nullptr && playerBenchSpots[player].size() < 5) {
        // If no Pokemon is in the active spot, create an ActivePokemon and place it there
        playerActiveSpots[player] = make_shared<ActivePokemon>(card); // Assume the original card has an `hp` field
        if (!silent)
            cout << "Player " << player + 1 << " played "
            << "\033[1;32m" << card->name << "\033[0m"  // Green color for the card name
            << " to their active spot." << endl;
    }
    else if (playerActiveSpots[player] != nullptr && playerBenchSpots[player].size() < 5) {
        // If there is a Pokemon in the active spot, create an ActivePokemon and place it on the bench
        playerBenchSpots[player].push_back(make_shared<ActivePokemon>(card));
        if (!silent)
            cout << "Player " << player + 1 << " played "
            << "\033[1;32m" << card->name << "\033[0m"  // Green color for the card name
            << " to their bench." << endl;
    }
    else {
        // No space to play Pokemon
        if (!silent)
            cout << "Player " << player + 1 << " cannot play " << card->name << " due to no available spots." << endl;
        return false;
    }

    // Remove the card from the player's hand
    removeCardFromHand(player, card);
    return true;
}

// for moving pokemon from bench to active when pokemon is knocked out
void Game::playPokemonFromBench(int player, shared_ptr<ActivePokemon> targetPokemon) {
    // Find the target Pokémon in the bench
    auto it = find_if(playerBenchSpots[currentPlayer].begin(), playerBenchSpots[currentPlayer].end(), [&](const shared_ptr<ActivePokemon>& pokemon) {
        return pokemon == targetPokemon;
        });

    // If the target Pokémon is found in the bench
    if (it != playerBenchSpots[currentPlayer].end()) {
        // Remove the target Pokémon from the bench
        playerBenchSpots[currentPlayer].erase(it);

        // Set the target Pokémon as the new active Pokémon
        playerActiveSpots[currentPlayer] = targetPokemon;
    }
    else {
        cerr << "Error: Target Pokemon not found in bench!" << endl;
    }
}

bool Game::attachEnergy(shared_ptr<ActivePokemon> targetPokemon) {
    // Check if the player has energy available
    if (playerAvailableEnergy[currentPlayer] == 'X') {
        if (!silent)
            cout << "Player " << currentPlayer + 1 << " does not have energy available.\n";
        return false;
    }

    // Add energy to the chosen Pokemon
    targetPokemon->currentEnergy.push_back(playerAvailableEnergy[currentPlayer]);

    if (!silent) {
        string energyColor;
        switch (playerAvailableEnergy[currentPlayer]) {
        case 'G': energyColor = "\033[32m"; break; // Green (Grass)
        case 'F': energyColor = "\033[31m"; break; // Red (Fire)
        case 'W': energyColor = "\033[34m"; break; // Blue (Water)
        case 'L': energyColor = "\033[33m"; break; // Yellow (Lightning)
        case 'P': energyColor = "\033[35m"; break; // Magenta (Psychic)
        case 'I': energyColor = "\033[38;5;130m"; break; // Brown/Orange (Fighting)
        case 'D': energyColor = "\033[90m"; break; // Dark Gray (Darkness)
        case 'M': energyColor = "\033[37m"; break; // Light Gray (Metal)
        default:  energyColor = "\033[0m"; break; // Reset
        }

        string pokemonColor = "\033[32m"; // Green for Pokémon names
        string resetColor = "\033[0m";    // Reset color

        cout << "Player " << currentPlayer + 1 << " attached "
            << energyColor << playerAvailableEnergy[currentPlayer] << resetColor
            << " energy to " << pokemonColor << targetPokemon->pokemonCard->name
            << resetColor << ".\n";
    }
    playerAvailableEnergy[currentPlayer] = 'X';
    return true;
}

void Game::performAttack(Attack attack) {
    shared_ptr<ActivePokemon> attacker = playerActiveSpots[currentPlayer];
    shared_ptr<ActivePokemon> defender = playerActiveSpots[1 - currentPlayer];

    if (!attacker || !defender) {
        if (!silent)
            cout << "Attack not possible: One or both active Pokemon are missing!" << endl;
        return;
    }

    string attackName = attack.name;  // Use the provided attack
    int damage = attack.damage;       // Use the damage from the provided attack

    if (!silent)
        cout << "Player " << currentPlayer + 1 << "'s \033[32m" << attacker->pokemonCard->name << "\033[0m "
        << "\033[31mattacks\033[0m "
        << "\033[32m" << defender->pokemonCard->name << "\033[0m "
        << "using \033[31m" << attackName << "\033[0m for \033[31m" << damage << "\033[0m damage!" << endl;

    // Reduce defender's HP
    defender->currentHP -= damage;
    damageDealt->at(currentPlayer) += damage;
    if (defender->currentHP <= 0) {
        if (!silent)
            cout << defender->pokemonCard->name << " is knocked out!" << endl;
        playerPoints[currentPlayer]++;

        // Remove the defeated Pokemon
        playerActiveSpots[1 - currentPlayer] = nullptr;

        // Check if the opponent has any Pokemon left
        if (playerBenchSpots[1 - currentPlayer].empty()) {
            if (!silent)
                cout << "Player " << currentPlayer + 1 << " wins the game!" << endl;
            gameOver = true;
            winner = currentPlayer;
        }
        else {
            // Promote a Pokemon from the bench to active
            playerActiveSpots[1 - currentPlayer] = playerBenchSpots[1 - currentPlayer].front();
            playerBenchSpots[1 - currentPlayer].erase(playerBenchSpots[1 - currentPlayer].begin());
            if(!silent)
                cout << playerActiveSpots[1 - currentPlayer]->pokemonCard->name << " moves to the active spot!" << endl;
        }
        checkForWinner();
    }
    endTurn(); // attacks always end the turn
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
    if (!silent)
        cout << "Player " << currentPlayer + 1 << "'s \033[35mturn\033[0m has \033[35mended\033[0m." << endl;
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
        if (!silent) {
            string colorCode;
            switch (selectedEnergy) {
            case 'G': colorCode = "\033[32m"; break; // Green (Grass)
            case 'F': colorCode = "\033[31m"; break; // Red (Fire)
            case 'W': colorCode = "\033[34m"; break; // Blue (Water)
            case 'L': colorCode = "\033[33m"; break; // Yellow (Lightning)
            case 'P': colorCode = "\033[35m"; break; // Magenta (Psychic)
            case 'I': colorCode = "\033[38;5;130m"; break; // Brown/Orange (Fighting)
            case 'D': colorCode = "\033[90m"; break; // Dark Gray (Darkness)
            case 'M': colorCode = "\033[37m"; break; // Light Gray (Metal)
            default:  colorCode = "\033[0m"; break; // Reset
            }

            cout << colorCode << selectedEnergy << "\033[0m has been added to Player "
                << currentPlayer + 1 << endl;
        }
    }
}