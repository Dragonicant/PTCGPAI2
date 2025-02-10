#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include <iostream>
#include <vector>
#include <memory>

// Forward declaration to avoid circular dependency
class Card;
class ActivePokemon;

using namespace std;

struct GameState {
    // Player-related info
    int playerPoints[2];  // Points of both players
    int playerHandSize[2];  // Hand sizes of both players
    shared_ptr<ActivePokemon> playerActiveSpots[2];  // Active Pokemon spots
    vector<shared_ptr<ActivePokemon>> playerBenchSpots[2];  // Bench Pokemon spots

    // Available energy for both players
    char playerAvailableEnergy[2];  // Available energy for both players

    // Player hands
    vector<shared_ptr<Card>> playerHands[2];  // Player hands

    // Decks
    vector<shared_ptr<Card>> playerDecks[2];  // Original unchanging decks
    vector<shared_ptr<Card>> gameDecks[2];  // Shuffled and modified decks

    // Game-related info
    bool gameOver = 0;  // Flag indicating whether the game is over
    int currentPlayer = -1;  // Index of the current player (0 or 1)
    int winner;  // Index of the winner (0 for Player 1, 1 for Player 2, -1 if no winner yet)
};

void displayGameState(const GameState& state);

#endif //GAMESTATE.HPP