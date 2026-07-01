#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp::Registry {

class EventRegister {
public:
    std::unordered_set<std::string> VisitedMaps;
    std::unordered_set<std::string> ActivatedNexusPortals;
    std::unordered_set<std::string> BossDefeated;
    std::unordered_set<std::string> LeftAfterBoss;
    bool SpookedMonster = false;
    std::unordered_map<std::string, int> eventKeys;

    void IncEvent(const std::string& e) {
        auto it = eventKeys.find(e);
        if (it != eventKeys.end()) it->second++;
        else eventKeys[e] = 1;
    }
    void SetEvent(const std::string& e, int value) { eventKeys[e] = value; }
    int GetEvent(const std::string& e) const {
        if (e.empty()) return 0;
        auto it = eventKeys.find(e);
        return it != eventKeys.end() ? it->second : 0;
    }
};

} // namespace AnodyneSharp::Registry

using AnodyneSharp::Registry::EventRegister;
