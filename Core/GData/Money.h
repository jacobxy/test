
#ifndef MONEY_H_
#define MONEY_H_

#include <vector>

namespace GData
{

struct Money
{
    Money() : gold(0), tael(0), ticket(0) {}
    unsigned int gold;
    unsigned int tael;
    unsigned int ticket;
};

enum
{

// 创建帮派
    CLAN_CREATE = 1,
// 挂机加速
    INSTANTAUTOBATTLE,
// 额外阵图
    FRONTMAP_ENTER1,
    FRONTMAP_ENTER2,
    FRONTMAP_ENTER3,
// 额外副本
    COPY_ENTER1,
    COPY_ENTER2,
    COPY_ENTER3,
    COPY_IM,
    COPY_AUTO1,
    COPY_AUTO2,
    COPY_AUTO3,
    COPY_AUTO4,
    COPY_AUTO5,
    COPY_AUTO6,
// 通天塔
    DUNGEON_IM,
    DUNGEON_AUTO1,
    DUNGEON_AUTO2,
    DUNGEON_AUTO3,
    DUNGEON_AUTO4,
    DUNGEON_AUTO5,
// 师门立即完成
    SHIMEN_IM,
// 衙门立即完成
    YAMEN_IM,
// 帮派立即完成
    CLAN_IM,
// 师门刷新
    SHIMEN_FRESH,
// 衙门刷新
    YAMEN_FRESH,
// 酒馆刷新
    RECRUIT,
// 地图跳转
    JUMP_MAP,
// 书商刷新
    BOOK_LIST,

    MONEY_MAX,
};

extern std::vector<Money> moneyNeed;

} // namespace GData

#endif // MONEY_H_

/* vim: set ai si nu sm smd hls is ts=4 sm=4 bs=indent,eol,start */

