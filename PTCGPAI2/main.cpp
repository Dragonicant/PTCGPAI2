#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib> // For system()
#include <algorithm>    
#include <random>

#include "types.hpp"
#include "deck.hpp"
#include "utilities.hpp"
#include "Game.hpp"
#include "Action.hpp"
#include "GameState.hpp"
#include "aiFunctions.hpp"

using namespace std;


int main() {
    //runScraper();
    CardCollection cardCollection;
    readCSVAndPopulateDeck("pokemon_cards.csv", cardCollection);  // Adjust the reading function accordingly
    //cardCollection.displayCollection();  // Display all the cards

    // Manually create another two decks
    Deck manualDeck1;
    Deck manualDeck2;

    // Manually add 6 unique cards to each deck
    const vector<string> manualCards1 = { "Hitmonchan", "Hitmonchan", "Rhyhorn", "Hitmontop", "Farfetchd", "Farfetchd" };
    const vector<string> manualCards2 = { "Scyther", "Scyther", "Bulbasaur", "Bulbasaur", "Farfetchd", "Farfetchd" };

    // Add cards to manualDeck1
    for (const auto& cardName : manualCards1) {
        const Card* card = cardCollection.findCardByName(cardName);
        if (card) {
            manualDeck1.addCard(make_shared<Card>(*card));
        }
        else {
            cout << "Card " << cardName << " not found!" << endl;
        }
    }

    // Add cards to manualDeck2
    for (const auto& cardName : manualCards2) {
        const Card* card = cardCollection.findCardByName(cardName);
        if (card) {
            manualDeck2.addCard(make_shared<Card>(*card));
        }
        else {
            cout << "Card " << cardName << " not found!" << endl;
        }
    }

    // Display manually created decks
    manualDeck1.displayCondensedDeck();
    manualDeck2.displayCondensedDeck();

    // Instantiate game with the manually created decks
    shared_ptr<Deck> manualDeck1Ptr = make_shared<Deck>(manualDeck1);
    shared_ptr<Deck> manualDeck2Ptr = make_shared<Deck>(manualDeck2);

    Game manualGame(manualDeck1Ptr, manualDeck2Ptr);

    int i = 0;
    cout << "\n\n!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!\n\n" << endl;
    cout << "Turn " << i + 1 << endl;
    while (i < 20 && !manualGame.isWinner()) {
        shared_ptr<ActionNode> root = make_shared<ActionNode>(manualGame.getGameState(), Action(ActionType::ROOT));
        buildActionTree(root, 4, 0, manualGame.getValidActions());
        //displayActionTree(root);
        Action bestAction = findBestAction(root, 20, manualGame.getGameState()->currentPlayer);
        
        cout << "\n";
        applyAction(manualGame, bestAction);
        if ((bestAction.type == ActionType::END_TURN || bestAction.type == ActionType::ATTACK) && !manualGame.isWinner()) {
            displayGameState(manualGame.getGameState());
            cout << "\n\n!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!\n\n" << endl;
            i++;
            cout << "Turn " << i + 1 << endl;
        }
    }

    return 0;
}