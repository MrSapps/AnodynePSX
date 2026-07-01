#include "AnodyneSharp/Registry/InventoryManager.hpp"
#include "AnodyneSharp/Registry/GlobalState.hpp"

namespace AnodyneSharp::Registry {

using BT = AnodyneSharp::Entities::BroomType;

bool InventoryManager::HasBroomType(BT type) const {
    switch(type) {
        case BT::Normal:      return HasBroom;
        case BT::Long:        return HasLengthen;
        case BT::Wide:        return HasWiden;
        case BT::Transformer: return HasTransformer;
        default:              return false;
    }
}

BT InventoryManager::EquippedBroom() const { return _equippedBroom; }

void InventoryManager::SetEquippedBroom(BT type) {
    _equippedBroom = type;
    EquippedBroomChanged = true;
}

int InventoryManager::CardCount() const {
    int count = 0;
    for (auto b : CardStatus) if (b) ++count;
    return count;
}

bool InventoryManager::UnlockedSecretz() const {
    for (auto b : SecretStatus) if (!b) return false;
    return true;
}

bool InventoryManager::UnlockedAllSecretz() const {
    return UnlockedSecretz();
}

int InventoryManager::GetCurrentMapKeys() const {
    return GetMapKeys(GlobalState::CURRENT_MAP_NAME);
}

int InventoryManager::GetMapKeys(const std::string& mapName) const {
    auto it = _mapKeys.find(mapName);
    return it != _mapKeys.end() ? it->second : 0;
}

bool InventoryManager::SetMapKeys(const std::string& mapName, int count) {
    _mapKeys[mapName] = count;
    return true;
}

int InventoryManager::AddCurrentMapKey() {
    auto& k = _mapKeys[GlobalState::CURRENT_MAP_NAME];
    return ++k;
}

int InventoryManager::RemoveCurrentMapKey() {
    auto& k = _mapKeys[GlobalState::CURRENT_MAP_NAME];
    if (k > 0) --k;
    return k;
}

int InventoryManager::AddMapKey(const std::string& mapName, int addition) {
    auto& k = _mapKeys[mapName];
    k += addition;
    return k;
}

} // namespace AnodyneSharp::Registry


