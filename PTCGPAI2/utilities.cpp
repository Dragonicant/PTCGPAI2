#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

#include "deck.hpp"

using namespace std;

// Function to safely convert string to integer with error checking
int safeStoi(const string& str, int defaultValue = 0) {
    try {
        return stoi(str);
    }
    catch (const std::invalid_argument& e) {
        cout << "Invalid argument for stoi: " << str << ", using default value: " << defaultValue << endl;
        return defaultValue; // Return default value in case of error
    }
    catch (const std::out_of_range& e) {
        cout << "Out of range error for stoi: " << str << ", using default value: " << defaultValue << endl;
        return defaultValue; // Return default value in case of error
    }
}

// Helper function to parse energy cost string into vector of EnergyRequirement
vector<EnergyRequirement> parseEnergyCost(const string& energyCostStr) {
    vector<EnergyRequirement> energyRequirements;

    if (energyCostStr.empty()) {
        return energyRequirements;  // Return empty if no energy
    }

    char currentType = energyCostStr[0];  // Start with the first energy type
    int count = 1;  // Start counting the energy

    // Loop through the string starting from the second character
    for (size_t i = 1; i < energyCostStr.size(); ++i) {
        if (energyCostStr[i] == currentType) {
            ++count;  // If the same type, increase the count
        }
        else {
            // If a different energy type is encountered, store the previous one
            energyRequirements.push_back({ currentType, count });
            currentType = energyCostStr[i];  // Update to the new type
            count = 1;  // Reset count for the new type
        }
    }

    // Don't forget to add the last one
    energyRequirements.push_back({ currentType, count });

    return energyRequirements;
}

// Read CSV and populate deck
void readCSVAndPopulateDeck(const string& filename, CardCollection& cardCollection) {
    ifstream file(filename);
    string line;

    // Skip the header line
    getline(file, line);

    while (getline(file, line)) {
        stringstream ss(line);
        string cardName, hpStr, typeStr, stageStr, weaknessStr, retreatCostStr;
        string attackName1, attackDamage1, energyCosts1;
        string attackName2, attackDamage2, energyCosts2;

        // Parse the CSV line
        getline(ss, cardName, ',');
        getline(ss, hpStr, ',');
        getline(ss, typeStr, ',');
        getline(ss, stageStr, ',');
        getline(ss, weaknessStr, ',');
        getline(ss, retreatCostStr, ',');
        getline(ss, attackName1, ',');
        getline(ss, attackDamage1, ',');
        getline(ss, energyCosts1, ',');
        getline(ss, attackName2, ',');
        getline(ss, attackDamage2, ',');
        getline(ss, energyCosts2, ',');

        // Convert string data into appropriate types with error handling
        int hp = safeStoi(hpStr);
        char type = typeStr[0];  // Just use the first character (G, F, etc.)
        int stage = safeStoi(stageStr);
        char weakness = weaknessStr[0];  // Just use the first character (G, F, etc.)
        int retreatCost = safeStoi(retreatCostStr);

        // Create attacks (up to 2 attacks per card)
        vector<Attack> attacks;

        Attack attack1 = {
            attackName1,
            safeStoi(attackDamage1),
            parseEnergyCost(energyCosts1)  // Parse the energy cost string into requirements
        };
        attacks.push_back(attack1);

        if (!attackName2.empty()) {
            Attack attack2 = {
                attackName2,
                safeStoi(attackDamage2),
                parseEnergyCost(energyCosts2)  // Parse the energy cost string into requirements
            };
            attacks.push_back(attack2);
        }

        // Create the card object
        Card card(cardName, 0, hp, type, stage, attacks, 0, weakness, retreatCost);

        // Add the card to the deck
        cardCollection.addCard(card);
    }
}

// Function to run the Python script and get URLs
void runScraper() {
    std::string pythonScript = "python scraper.py";  // Path to your Python script
    std::cout << "Running Python Scraper...\n";
    int result = system(pythonScript.c_str());
    if (result != 0) {
        std::cout << "Error running the Python script.\n";
    }
}

// Define ANSI color codes
#define COLOR_CYAN    "\033[36m"
#define COLOR_RESET   "\033[0m"

// Function to print a string in cyan
void printError(const std::string& text) {
    std::cout << COLOR_CYAN << text << COLOR_RESET << std::endl;
}