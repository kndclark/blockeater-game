#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include "nlohmann/json.hpp"

struct ScoreEntry {
    std::string name;
    int score;
    std::string date;

    // Serialization helpers for nlohmann::json
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ScoreEntry, name, score, date)
};

class ScoreboardManager {
public:
    explicit ScoreboardManager(const std::string& filepath);

    void addScore(const std::string& name, int score);
    const std::vector<ScoreEntry>& getScores() const;

private:
    void loadScores();
    void saveScores() const;

    std::string filepath_;
    std::vector<ScoreEntry> scores_;
};