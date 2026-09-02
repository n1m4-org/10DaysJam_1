#include "SignalSystem.h"
#include <queue>
#include <algorithm>

SignalSystem::SignalSystem()
{
}

SignalSystem::~SignalSystem()
{
}

bool SignalSystem::UpdateSignal(int mapWidth, int mapHeight,
                                const std::vector<std::vector<int>>& currentMap,
                                const std::vector<std::unique_ptr<BaseObject2d>>& objects,
                                std::vector<std::vector<int>>& outStrengthMap,
                                int& outPcStrength)
{
    outStrengthMap.assign(mapHeight, std::vector<int>(mapWidth, 0));
    outPcStrength = 0;

    std::set<RayState> visitedRays;
    std::queue<RayState> rayQueue;

    // 1. ルーター (Router) を初期強度 10 で追加
    for (const auto& obj : objects)
    {
        if (obj && obj->GetObjectType() == ObjectType2d::kRouter)
        {
            Vector2Int startPos = obj->GetPosition();
            Vector2Int startDir = obj->GetAngle();

            if (startDir.x == 0 && startDir.y == 0)
            {
                startDir = { 0, 1 }; // デフォルト下向き
            }

            RayState initialState{ startPos, startDir, kInitialSignalStrength };
            rayQueue.push(initialState);
            visitedRays.insert(initialState);
        }
    }

    // 2. 電波レイの伝搬と10マス減衰計算
    while (!rayQueue.empty())
    {
        RayState current = rayQueue.front();
        rayQueue.pop();

        Vector2Int checkPos = current.pos + current.dir;
        int currentStrength = current.strength;

        while (currentStrength > 0)
        {
            int tileType = CheckType(currentMap, checkPos);
            if (tileType == 1) // 壁または範囲外
            {
                break; // レイ終了
            }

            // マスの電波強度を記録 (最大強度を保持)
            outStrengthMap[checkPos.y][checkPos.x] = (std::max)(outStrengthMap[checkPos.y][checkPos.x], currentStrength);

            BaseObject2d* objOnCell = FindObjectAtPos(objects, checkPos);
            if (objOnCell)
            {
                ObjectType2d type = objOnCell->GetObjectType();

                // アルミホイル (AlumiWall): 電波を完全に遮断
                if (type == ObjectType2d::kAlumiWall)
                {
                    break;
                }
                // パソコン (PC): 到達した電波強度を記録
                else if (type == ObjectType2d::kPC)
                {
                    outPcStrength = (std::max)(outPcStrength, currentStrength);
                }
                // 中継器 (Repeater): 電波到達時に初期強さ(10)にリセットして中継発射！
                else if (type == ObjectType2d::kRepeater)
                {
                    Vector2Int repeaterDir = objOnCell->GetAngle();
                    if (repeaterDir.x == 0 && repeaterDir.y == 0)
                    {
                        repeaterDir = current.dir;
                    }

                    // 強度を初期値 kInitialSignalStrength (10) にリセット
                    RayState newRay{ checkPos, repeaterDir, kInitialSignalStrength };
                    if (visitedRays.find(newRay) == visitedRays.end())
                    {
                        visitedRays.insert(newRay);
                        rayQueue.push(newRay);
                    }
                    break; // このレイは中継器でリレーされるため、この直線判定はここで終了
                }
            }

            // 1マス進むごとに電波強度が 1 減衰
            currentStrength--;
            checkPos = checkPos + current.dir;
        }
    }

    return outPcStrength > 0;
}

BaseObject2d* SignalSystem::FindObjectAtPos(const std::vector<std::unique_ptr<BaseObject2d>>& objects, const Vector2Int& pos)
{
    for (const auto& obj : objects)
    {
        if (obj && obj->GetPosition() == pos)
        {
            return obj.get();
        }
    }
    return nullptr;
}

int SignalSystem::CheckType(const std::vector<std::vector<int>>& map, const Vector2Int& pos)
{
    if (pos.y >= 0 && pos.x >= 0 &&
        static_cast<size_t>(pos.y) < map.size() &&
        static_cast<size_t>(pos.x) < map[pos.y].size())
    {
        return map[pos.y][pos.x] / 100;
    }
    return 1;
}

