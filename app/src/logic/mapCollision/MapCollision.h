#pragma once
#include <object/baseObject2d/BaseObject2d.h>
#include <vector>
#include <memory>

class MapCollision
{
public:
    MapCollision();
    virtual ~MapCollision();

    // 移動＆押し出し処理
    // currentMap: mapData_にオブジェクトの位置情報を合成した最新の2次元マップ
    // objects: マップ上に存在する全てのBaseObject2dリスト
    // subject: 移動しようとしている主体（プレイヤーなど）
    // dir: 移動方向のベクトル (例: {0, -1}, {1, 0} など)
    // 戻り値: 移動が成功したら true, 壁や押せない障害物で阻止されたら false
    bool TryMove(const std::vector<std::vector<int>>& currentMap,
                 const std::vector<std::unique_ptr<BaseObject2d>>& objects,
                 BaseObject2d& subject,
                 const Vector2Int& dir);

    // マップ上の座標からCheckTypeを取得
    int CheckType(const std::vector<std::vector<int>>& map, const Vector2Int& pos);

private:
    // 指定座標にあるオブジェクトを検索
    BaseObject2d* FindObjectAtPos(const std::vector<std::unique_ptr<BaseObject2d>>& objects, const Vector2Int& pos);
};



