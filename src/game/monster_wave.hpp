#pragma once

namespace aren {

enum class MonsterWaveCondition {
    Always = 0,
    MonsterCleared,
    EnterZone,
};

struct MonsterWave {
public:
    std::vector<std::string> zones;
    std::string monsterName;
    MonsterWaveCondition conditionType = MonsterWaveCondition::Always;
    std::string conditionValue;
};

} // namespace aren