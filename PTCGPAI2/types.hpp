#ifndef TYPES_HPP
#define TYPES_HPP

#include <unordered_map>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <iostream>

using namespace std;

class PTCGPTypes {
public:
    // Method to get the full type name from one-character abbreviation
    static std::string getTypeName(char abbreviation) {
        // Mapping of one-character abbreviations to full Pokémon type names
        static const std::unordered_map<char, std::string> abbreviationToType = {
            {'G', "Grass"},
            {'F', "Fire"},
            {'W', "Water"},
            {'L', "Lightning"},
            {'P', "Psychic"},
            {'I', "Fighting"},
            {'D', "Darkness"},
            {'M', "Metal"},
            {'N', "Normal"},
            {'R', "Dragon"}
        };

        // Check if the abbreviation exists in the map and return the full type name
        auto it = abbreviationToType.find(abbreviation);
        if (it != abbreviationToType.end()) {
            return it->second;
        }
        else {
            throw std::invalid_argument("Invalid abbreviation: " + std::string(1, abbreviation));
        }
    }
};

struct EnergyRequirement {
    char type; // Energy type (e.g., "F", "G", etc.)
    int amount;  // Number of energy required

    // Convert the energy requirement to a string in the form of "GXX" or "FF"
    string toEnergyString() const {
        return string(amount, type);  // Repeat the energy type 'amount' times
    }

    void display() const {
        cout << amount << " " << PTCGPTypes::getTypeName(type);
    }
};

struct Attack {
    string name;  // Name of the attack
    int damage = 0;   // Damage dealt by the attack
    vector<EnergyRequirement> energyRequirement;  // Energy required to use the attack
    int effectId = -1; // ID of the effect (use -1 for no effect)

    // Convert the energy requirements to a string like "GXX", "FF"
    string toEnergyString() const {
        string energyStr;
        for (const auto& req : energyRequirement) {
            energyStr += req.toEnergyString();  // Combine all energy types for this attack
        }
        return energyStr;
    }

    void display() const {
        cout << "    - " << name << " (Damage: " << damage << ", Energy Required: ";
        string energyStr = toEnergyString();  // Get the energy string for display
        cout << energyStr;
        if (effectId != -1) {
            cout << ", Effect ID: " << effectId;
        }
        cout << ")\n";
    }
};

// Basic card structure
class Card {
public:
    string name;
    int cardID;
    int hp;
    char type; // Type as a char
    int stage;
    vector<Attack> attacks;
    int abilID;
    char weakness; // Weakness as a char now
    int retreatCost;

    // Updated constructor to accept char for type and weakness
    Card(string n, int id, int health, char t, int s, vector<Attack> atk, int abilID, char weak, int retreat)
        : name(n), cardID(id), hp(health), type(t), stage(s), attacks(atk), abilID(abilID), weakness(weak), retreatCost(retreat) {}

    // Updated display method to use PTCGPTypes::getTypeName for type and weakness
    void display() const {
        cout << name << "\n";
        cout << "HP: " << hp << " | Type: " << type << " | Stage: " << stage << "\n";
        cout << "Weakness: " << weakness << " | Retreat Cost: " << retreatCost << "\n";

        for (const auto& attack : attacks) {
            string energyStr = attack.toEnergyString();  // Get energy string for the attack

            cout << left << setw(6) << energyStr   // Left-align, width 6
                << setw(20) << attack.name.substr(0, 20)  // Left-align, truncate to 20 chars
                << right << setw(4) << attack.damage  // Right-align, width 4
                << "\n";
        }
        cout << endl;
    }
};

#endif // TYPES_HPP