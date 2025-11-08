#include "ScoreboardManager.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

ScoreboardManager::ScoreboardManager(const std::string& filepath) : filepath_(filepath) {
    loadScores();
}

void ScoreboardManager::addScore(const std::string& name, int score) {
    // Get current date
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    std::string date_str = oss.str();

    scores_.push_back({name, score, date_str});

    // Sort scores in descending order
    std::sort(scores_.begin(), scores_.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
        return a.score > b.score;
    });

    // Optional: limit the number of scores
    if (scores_.size() > 10) {
        scores_.resize(10);
    }

    saveScores();
}

const std::vector<ScoreEntry>& ScoreboardManager::getScores() const {
    return scores_;
}

void ScoreboardManager::loadScores() {
    std::ifstream f(filepath_);
    if (f.is_open()) {
        nlohmann::json data = nlohmann::json::parse(f, nullptr, false);
        if (!data.is_discarded()) {
            scores_ = data.get<std::vector<ScoreEntry>>();
        }
    }
}

void ScoreboardManager::saveScores() const {
    std::ofstream o(filepath_);
    nlohmann::json data = scores_;
    o << std::setw(4) << data << std::endl;
}