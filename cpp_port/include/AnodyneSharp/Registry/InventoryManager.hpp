#pragma once
#include "AnodyneSharp/Common.hpp"

namespace AnodyneSharp {
namespace Entities { enum class BroomType { NONE, Normal, Long, Wide, Transformer }; }

namespace Registry {

class GlobalState; // forward

class InventoryManager {
public:
    bool HasBroom       = false;
    bool HasLengthen    = false;
    bool HasWiden       = false;
    bool HasTransformer = false;
    bool CanJump        = false;
    bool EquippedBroomChanged = false;

    enum class TradeState { NONE, SHOES, BOX };
    TradeState tradeState = TradeState::NONE;

    std::array<bool, 49> CardStatus  = {};
    std::array<bool, 14> SecretStatus = {};
    std::array<bool,  3> BigKeyStatus = {};

    std::unordered_map<std::string, int> _mapKeys;
    AnodyneSharp::Entities::BroomType _equippedBroom = AnodyneSharp::Entities::BroomType::NONE;

    bool HasAnyBroom()   const { return HasBroom || HasLengthen || HasWiden || HasTransformer; }
    bool HasEveryBroom() const { return HasBroom && HasLengthen && HasWiden && HasTransformer; }
    bool HasBroomType(AnodyneSharp::Entities::BroomType type) const;
    AnodyneSharp::Entities::BroomType EquippedBroom() const;
    void SetEquippedBroom(AnodyneSharp::Entities::BroomType type);

    int  CardCount() const;
    bool UnlockedSecretz() const;
    bool UnlockedAllSecretz() const;

    int  GetCurrentMapKeys() const;
    int  GetMapKeys(const std::string& mapName) const;
    bool SetMapKeys(const std::string& mapName, int count);
    int  AddCurrentMapKey();
    int  RemoveCurrentMapKey();
    int  AddMapKey(const std::string& mapName, int addition);
};

} // namespace Registry
} // namespace AnodyneSharp

using AnodyneSharp::Registry::InventoryManager;
