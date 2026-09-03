#pragma once
#include <object/baseObject2d/BaseObject2d.h>
#include <vector>
#include <memory>
#include <set>

class SignalSystem
{
public:
    static constexpr int kInitialSignalStrength = 10; // 初期電波強度: 10マス分

    SignalSystem();
    ~SignalSystem();

    // 電波の伝搬計算
    // outStrengthMap: 各マスの電波強度 (0: なし, 1~10: 強度)
    // outPcStrength: PCに到達した最大強度
    bool UpdateSignal(int mapWidth, int mapHeight,
                      const std::vector<std::vector<int>>& currentMap,
                      const std::vector<std::unique_ptr<BaseObject2d>>& objects,
                      std::vector<std::vector<int>>& outStrengthMap,
                      int& outPcStrength);

private:
    struct RayState
    {
        Vector2Int pos;
        Vector2Int dir;
        int strength;

        bool operator<(const RayState& other) const
        {
            if (pos.x != other.pos.x) return pos.x < other.pos.x;
            if (pos.y != other.pos.y) return pos.y < other.pos.y;
            if (dir.x != other.dir.x) return dir.x < other.dir.x;
            return strength < other.strength;
        }
    };

    BaseObject2d* FindObjectAtPos(const std::vector<std::unique_ptr<BaseObject2d>>& objects, const Vector2Int& pos);
    int CheckType(const std::vector<std::vector<int>>& map, const Vector2Int& pos);
};

