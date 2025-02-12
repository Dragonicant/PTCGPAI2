#pragma once

//forward declaration
class CardCollection;

int safeStoi(const string& str, int defaultValue = 0);
vector<EnergyRequirement> parseEnergyCost(const string& energyCostStr);
void readCSVAndPopulateDeck(const string& filename, CardCollection& cardCollection);
void runScraper();
void printError(const std::string& text);