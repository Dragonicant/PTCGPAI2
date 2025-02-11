#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include <memory>   


// Forward declaration to avoid circular dependency
class Card;
struct Action;
class Deck;
struct GameState;
struct Attack;

class ActivePokemon {
public:
    std::shared_ptr<Card> pokemonCard;
    int currentHP;
    std::vector<char> currentEnergy;

    ActivePokemon(std::shared_ptr<Card> card);
    // Copy constructor
    ActivePokemon(const ActivePokemon& other);

    void addEnergy(char energyType);
    void removeEnergy(char energyType);
    void displayActivePokemon() const;
};

class Game {
public:
    Game(std::shared_ptr<Deck> player1Deck, std::shared_ptr<Deck> player2Deck, bool silent = false);
    Game(const std::shared_ptr<GameState>& state, bool silent = false);

    const std::shared_ptr<ActivePokemon>& getPlayerActiveSpot(int player) const;
    const std::vector<std::shared_ptr<ActivePokemon>>& getPlayerBenchSpots(int player) const;

    void setSilent(bool silent);

    std::shared_ptr<GameState> getGameState();
    bool hasNoPokemon(int player);
    void checkForWinner();
    std::vector<Action> getValidActions();
    void displayValidActions();
    void shuffleDeck(int player);
    void drawInitialCards(int player);
    std::shared_ptr<Card> drawCard(int player);
    void showHands() const;
    bool playPokemon(int player, int cardFromHand);
    bool playPokemon(int player, std::shared_ptr<Card> card);
    bool attachEnergy(std::shared_ptr<ActivePokemon> targetPokemon);
    void performAttack(Attack attack);
    void removeCardFromHand(int player, std::shared_ptr<Card> cardToRemove);    
    void displayBoard() const;
    void endTurn();

private:
    std::shared_ptr<Deck> playerDecks[2];
    std::vector<std::shared_ptr<Card>> gameDecks[2];

    std::vector<std::shared_ptr<Card>> playerHands[2];
    std::shared_ptr<ActivePokemon> playerActiveSpots[2];
    std::vector<std::shared_ptr<ActivePokemon>> playerBenchSpots[2];

    int playerPoints[2];
    char playerAvailableEnergy[2];

    bool gameOver = false;
    int currentPlayer = 0;
    int winner = -1;

    std::vector<int> damageDealt[2];  // Total damage dealt by each player

    bool silent;

    void addEnergyToPlayer(int player);
};

#endif // GAME_HPP