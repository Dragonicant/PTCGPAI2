#ifndef STAGES_HPP
#define STAGES_HPP

#include <unordered_map>
#include <string>
#include <stdexcept>

// Enum to represent Pokémon stages
enum class StageID {
    Basic = 0,
    Stage1 = 1,
    Stage2 = 2
};

// Class to handle Pokémon stage operations
class PTCGPStages {
public:
    // Method to get the full name of the stage from the stage ID
    static std::string getStageName(StageID stage) {
        // Mapping from stage ID to full stage name
        static const std::unordered_map<StageID, std::string> stageToName = {
            {StageID::Basic, "Basic"},
            {StageID::Stage1, "Stage 1"},
            {StageID::Stage2, "Stage 2"}
        };

        // Return the stage name based on the StageID
        auto it = stageToName.find(stage);
        if (it != stageToName.end()) {
            return it->second;
        } else {
            throw std::invalid_argument("Invalid stage ID");
        }
    }

    // Method to get the StageID from an integer (0 for Basic, 1 for Stage 1, 2 for Stage 2)
    static StageID getStageFromInt(int stageInt) {
        switch (stageInt) {
            case 0: return StageID::Basic;
            case 1: return StageID::Stage1;
            case 2: return StageID::Stage2;
            default: throw std::invalid_argument("Invalid stage integer");
        }
    }
};

#endif // STAGES_HPP