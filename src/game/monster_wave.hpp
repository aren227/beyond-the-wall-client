#pragma once

namespace aren {

enum class MonsterWaveCondition : int {
    Always = 0,
    MonsterCleared,
    EnterZone,
    ConsumedConsumable,
};

struct MonsterWave {
public:
    std::vector<std::string> zones;
    std::string monsterName;
    MonsterWaveCondition conditionType = MonsterWaveCondition::Always;
    std::string conditionValue;
};

} // namespace aren