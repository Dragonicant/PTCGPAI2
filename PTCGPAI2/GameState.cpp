#include "GameState.hpp"
#include "Game.hpp"
#include "types.hpp"

void displayGameState(const shared_ptr<GameState>& state) {
    // Display points
    cout << "Points: " << state->playerPoints[0] << " - " << state->playerPoints[1] << endl;

    // Display player 2 hand
    cout << "Player 2 Hand: ";
    for (const auto& card : state->playerHands[1]) {
        cout << card->name << " ";
    }
    cout << endl;

    cout << endl;
    /// Display Player 2's Bench (top row)
    cout << (state->playerBenchSpots[1].size() > 0 ? state->playerBenchSpots[1][0]->pokemonCard->name : "Empty") << "  "
        << (state->playerBenchSpots[1].size() > 1 ? state->playerBenchSpots[1][1]->pokemonCard->name : "Empty") << "  "
        << (state->playerBenchSpots[1].size() > 2 ? state->playerBenchSpots[1][2]->pokemonCard->name : "Empty") << endl;

    // Display Player 2's Active (middle row)
    cout << state->playerAvailableEnergy[1] << "       " << (state->playerActiveSpots[1] != nullptr ? state->playerActiveSpots[1]->pokemonCard->name : "Empty") << endl;

    // Display Player 1's Active (middle row)
    cout << "        " << (state->playerActiveSpots[0] != nullptr ? state->playerActiveSpots[0]->pokemonCard->name : "Empty") << "       " << state->playerAvailableEnergy[0] << endl;

    // Display Player 1's Bench (bottom row)
    cout << (state->playerBenchSpots[0].size() > 0 ? state->playerBenchSpots[0][0]->pokemonCard->name : "Empty") << "  "
        << (state->playerBenchSpots[0].size() > 1 ? state->playerBenchSpots[0][1]->pokemonCard->name : "Empty") << "  "
        << (state->playerBenchSpots[0].size() > 2 ? state->playerBenchSpots[0][2]->pokemonCard->name : "Empty") << endl;
    cout << endl;

    // Display player hands (card names)
    cout << "Player 1 Hand: ";
    for (const auto& card : state->playerHands[0]) {
        cout << card->name << " ";
    }
    cout << endl;

    for (int player = 0; player < 2; player++) {
        cout << "Player " << player + 1 << "'s Pokemon Energy:" << endl;

        // Active Pokemon
        if (state->playerActiveSpots[player]) {
            cout << "  Active: " << state->playerActiveSpots[player]->pokemonCard->name
                << " | Energy: ";
            for (char energy : state->playerActiveSpots[player]->currentEnergy) {
                cout << energy;
            }
            cout << endl;
        }

        // Bench Pokemon
        for (const auto& pokemon : state->playerBenchSpots[player]) {
            cout << "  Bench: " << pokemon->pokemonCard->name << " | Energy: ";
            for (char energy : pokemon->currentEnergy) {
                cout << energy;
            }
            cout << endl;
        }
    }

    // Display game-over status and winner
    if (state->gameOver) {
        if (state->winner != -1) {
            cout << "Game Over! Player " << state->winner + 1 << " wins!" << endl;
        }
        else {
            cout << "Game Over! It's a draw!" << endl;
        }
    }
    cout << endl;
}