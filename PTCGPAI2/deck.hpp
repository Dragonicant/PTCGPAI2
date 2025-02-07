#ifndef DECK_HPP
#define DECK_HPP

#include "types.hpp"
#include <vector>
#include <memory>
#include <unordered_map>
#include <set>
#include <algorithm>

using namespace std;

class Deck {
public:
    vector<shared_ptr<Card>> cards;  // A deck contains shared pointers to cards

    static const int MAX_DECK_SIZE = 20;  // Maximum number of cards in a deck
    static const int MAX_CARD_DUPLICATES = 2;  // Maximum duplicates of the same card
    static const int MIN_ENERGY_TYPES = 1;
    static const int MAX_ENERGY_TYPES = 3;

    Deck() {

    }

    // Copy constructor
    Deck(const Deck& other) {
        for (const auto& card : other.cards) {
            cards.push_back(make_shared<Card>(*card));
        }
        energyTypes = other.energyTypes;
    }

    // Function to add a card to the deck
    bool addCard(shared_ptr<Card> card) {
        if (cards.size() >= MAX_DECK_SIZE) {
            cout << "Cannot add card. Deck size limit of " << MAX_DECK_SIZE << " reached." << endl;
            return false;
        }

        // Check for duplicates
        int count = 0;
        for (const auto& existingCard : cards) {
            if (existingCard->name == card->name) {
                ++count;
            }
        }

        if (count >= MAX_CARD_DUPLICATES) {
            cout << "Cannot add more than " << MAX_CARD_DUPLICATES << " copies of the card: " << card->name << endl;
            return false;
        }

        cards.push_back(card);
        determineEnergyTypes();
        return true;
    }

    // Function to display the deck
    void displayDeck() const {
        cout << "Deck (" << cards.size() << " cards):" << endl;
        for (const auto& card : cards) {
            card->display();
            cout << "\n";  // Add a newline between cards for better separation
        }
        cout << endl;
    }

    // Function to verify the deck's validity
    bool verifyDeck() const {
        if (cards.size() > MAX_DECK_SIZE) {
            cout << "Deck exceeds the maximum size of " << MAX_DECK_SIZE << " cards!" << endl;
            return false;
        }

        unordered_map<string, int> cardCounts;
        for (const auto& card : cards) {
            cardCounts[card->name]++;
        }

        for (const auto& entry : cardCounts) {
            if (entry.second > MAX_CARD_DUPLICATES) {
                cout << "Card " << entry.first << " exceeds the maximum limit of " << MAX_CARD_DUPLICATES << " copies!" << endl;
                return false;
            }
        }

        return true;
    }
    
    void determineEnergyTypes() {
        unordered_map<char, int> energyCount; // Tracks total energy per type

        for (const auto& card : cards) {
            auto highestEnergy = getHighestEnergyRequirement(card);
            for (const auto& [energyType, count] : highestEnergy) {  // Use energyType instead of type
                energyCount[energyType] += count;
            }
        }

        // Sort energy types by total count
        vector<pair<char, int>> sortedEnergy(energyCount.begin(), energyCount.end());
        sort(sortedEnergy.begin(), sortedEnergy.end(), [](const auto& a, const auto& b) {
            return a.second > b.second; // Sort descending by count
            });

        // Output sorted energy counts
        cout << "Energy Type Counts:\n";
        for (size_t i = 0; i < sortedEnergy.size(); i++) {
            cout << sortedEnergy[i].first << " - " << sortedEnergy[i].second;
            if (i < sortedEnergy.size() - 1) {
                cout << ", ";
            }
        }
        cout << endl;

        // Compute total energy count
        int totalEnergy = 0;
        for (const auto& [type, count] : sortedEnergy) {
            totalEnergy += count;
        }

        // Determine number of energy types based on 66% rule
        energyTypes.clear();
        if (!sortedEnergy.empty() && totalEnergy > 0) {
            double firstRatio = (double)sortedEnergy[0].second / totalEnergy;
            double topTwoRatio = (sortedEnergy.size() > 1) ? (double)(sortedEnergy[0].second + sortedEnergy[1].second) / totalEnergy : 1.0;

            int maxTypes = 3; // Default to 3
            if (firstRatio > 0.66) {
                maxTypes = 1; // If top energy is over 66%, only use 1 type
            }
            else if (topTwoRatio > 0.66) {
                maxTypes = 2; // If top two combined exceed 66%, use 2 types
            }

            for (size_t i = 0; i < sortedEnergy.size() && i < maxTypes; i++) {
                energyTypes.push_back(sortedEnergy[i].first);
            }
        }

        // Ensure at least one energy type is chosen
        if (energyTypes.empty() && !sortedEnergy.empty()) {
            energyTypes.push_back(sortedEnergy[0].first);
        }
    }

    static unordered_map<char, int> getHighestEnergyRequirement(shared_ptr<Card> card) {
        unordered_map<char, int> maxEnergyPerType;

        for (const auto& attack : card->attacks) {
            unordered_map<char, int> energyCount;

            // Count energy per type for this attack
            for (const auto& requirement : attack.energyRequirement) {
                char energyType = requirement.type; // Extract the energy type
                if (energyType != 'X') { // Ignore colorless
                    energyCount[energyType] += requirement.amount;
                }
            }

            // Merge into maxEnergyPerType, keeping the highest value per type
            for (const auto& [energyType, count] : energyCount) {
                maxEnergyPerType[energyType] = max(maxEnergyPerType[energyType], count);
            }
        }

        return maxEnergyPerType;
    }

    // Display deck energy types
    void displayEnergyTypes() const {
        cout << "Deck Energy Types: ";
        for (char type : energyTypes) {
            cout << type;
        }
        cout << endl;
    }

    // Function to display a condensed view of the deck (card name and quantity)
    void displayCondensedDeck() const {
        unordered_map<string, int> cardCounts;

        // Count occurrences of each card in the deck
        for (const auto& card : cards) {
            cardCounts[card->name]++;
        }

        // Display the condensed deck
        cout << "Condensed Deck View:" << endl;
        displayEnergyTypes();
        for (const auto& entry : cardCounts) {
            cout << entry.second << "x " << entry.first << endl;
        }
        cout << endl;
    }

    // Method to get the energy types selected for the deck
    vector<char> getEnergyTypes() const {
        return energyTypes;
    }

private:
    vector<char> energyTypes;  // Set of energy types chosen for this deck (1-3 types)
};

class CardCollection {
public:
    vector<Card> cards;  // Collection of cards

    // Add a card to the collection
    void addCard(const Card& card) {
        cards.push_back(card);
    }

    // Display all cards in the collection
    void displayCollection() const {
        for (const auto& card : cards) {
            card.display();
            cout << "\n"; // Add a newline between cards for better separation
        }
    }

    // Optional: method to find a card by name
    const Card* findCardByName(const string& cardName) const {
        for (const auto& card : cards) {
            if (card.name == cardName) {
                return &card;
            }
        }
        return nullptr;  // Return null if not found
    }

    // Optional: method to remove a card by name
    void removeCardByName(const string& cardName) {
        cards.erase(remove_if(cards.begin(), cards.end(),
            [&cardName](const Card& card) { return card.name == cardName; }),
            cards.end());
    }

    // Get total number of cards in the collection
    size_t getCardCount() const {
        return cards.size();
    }
};

#endif // DECK_HPP