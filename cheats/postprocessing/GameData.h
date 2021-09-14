#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "imgui/imgui.h"
#include <utils/util.hpp>


struct LocalPlayerData;

struct PlayerData;
struct ObserverData;
struct WeaponData;
struct EntityData;
struct LootCrateData;
struct ProjectileData;
struct InfernoData;
struct BombData;
struct SmokeData;

namespace GameData
{

    class Lock {
    private:
        static inline std::mutex mutex;
        std::scoped_lock<std::mutex> lock{ mutex };
    };

}
