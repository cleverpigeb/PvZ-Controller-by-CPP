// 【PE】二十四炮.cc：PCC键控工具的演示脚本，用于演示泳池二十四炮
// 作者：him君

#include <cassert>
#include <cstdio>
#include <cstring>

#include <array>
#include <thread>
#include <queue>
#include <utility>

#include <Windows.h>

#include "pvz_controller.h"

constexpr int kMaxCannonNum(24);
constexpr int kMaxIceNum(54);

// 以下为修改区（玉米加农炮的位置）

int kCannonList[kMaxCannonNum + 5][5] = { {1, 1}, {1, 3}, {1, 5}, {1, 7},
                                          {2, 1}, {2, 3}, {2, 5}, {2, 7},
                                          {3, 1}, {3, 3}, {3, 5}, {3, 7},
                                          {4, 1}, {4, 3}, {4, 5}, {4, 7},
                                          {5, 1}, {5, 3}, {5, 5}, {5, 7},
                                          {6, 1}, {6, 3}, {6, 5}, {6, 7} };
int kIceList[kMaxIceNum + 5][5] = { {4, 9} };

void ChoosingCards(void);

// 以上为修改区

int main(void) {
  // 记录用过的炮，并同时记录发炮时的时间戳
  std::queue<std::pair<time_t, int> >* used_cannons;

  // 以下为修改区（地图类型和玉米加农炮的数量）
  if (InitController('P', 24, kCannonList, &used_cannons,
    1, kIceList, 3, 0, 1, 2)) {
    // 以上为修改区
      // 如果想要关闭自动补冰可以在下面一行前面加上“//”
    StartIceFiller();

    // 如果想要手动选卡可以在下面一行前面加上“//”
    ChoosingCards();

    std::puts("\n======= 战斗开始 =======");

    for (int wave(1); wave < 21; ++wave) {
      printf_s("\n正在准备处理第 %d 波的敌人\n", wave);

      // 以下为修改区（主体节奏）

      switch (wave) {
      case 1: case 4: case 7: case 11: case 14: case 17:
        PreJudge(15, false);
        Cannon(2, 9, used_cannons);
        Cannon(5, 9, used_cannons);

        Sleep(1080);
        Cannon(1, 7.7, used_cannons);
        Cannon(5, 7.7, used_cannons);

        break;

      case 2: case 5: case 8: case 12: case 15: case 18:
        PreJudge(95, false);
        Cannon(2, 9, used_cannons);
        Cannon(5, 9, used_cannons);

        Sleep(3730 - 2980);
        WakeIce();

        Sleep(300);

        break;

      case 3: case 6: case 13: case 16:
        PreJudge(95, false);
        Cannon(2, 9, used_cannons);
        Cannon(2, 9, used_cannons);
        Cannon(5, 9, used_cannons);
        Cannon(5, 9, used_cannons);

        Sleep(1080);
        Cannon(1, 8.8, used_cannons);
        Cannon(5, 8.8, used_cannons);

        break;

      case 9: case 19:
        PreJudge(95, false);
        Cannon(2, 9, used_cannons);
        Cannon(5, 9, used_cannons);
        Cannon(2, 9, used_cannons);
        Cannon(5, 9, used_cannons);

        Sleep(1080);
        Cannon(1, 8.8, used_cannons);
        Cannon(5, 8.8, used_cannons);

        Sleep(7000 - 1080 + 950);
        Cannon(2, 9, used_cannons);
        Cannon(5, 9, used_cannons);

        break;

      case 10:
        PreJudge(55, true);
        Cannon(2, 9, used_cannons);
        Cannon(5, 9, used_cannons);
        Cannon(2, 9, used_cannons);
        Cannon(5, 9, used_cannons);

        Sleep(1080);
        Cannon(1, 8.8, used_cannons);
        Cannon(5, 8.8, used_cannons);

        break;

      case 20:
        PreJudge(140, true);
        Cannon(4, 6, used_cannons);
        Cannon(4, 8, used_cannons);

        Sleep(1400 - 550);
        Cannon(1, 9, used_cannons);
        Cannon(2, 9, used_cannons);
        Cannon(5, 9, used_cannons);
        Cannon(5, 9, used_cannons);

        Sleep(1080);
        Cannon(1, 9, used_cannons);
        Cannon(2, 9, used_cannons);
        Cannon(5, 9, used_cannons);
        Cannon(5, 9, used_cannons);

        break;

      default:
        assert(false);
      }

      // 以上为修改区
    }

    std::puts("\n======= 战斗结束 =======");
    QuitController(&used_cannons);
  }

  return 0;
}

void ChoosingCards(void) {
  // 以下为修改区（要选的植物卡）
  std::array<std::array<int, 3>, 10> cards{
    2, 7, 0,
    2, 7, 1,
    5, 4, 0,
    2, 8, 0,
    3, 1, 0,
    3, 2, 0,
    1, 3, 0,
    4, 4, 0,
    1, 4, 0,
    2, 1, 0
  };
  // 以上为修改区

  std::puts("\n======= 开始选卡 =======");

  for (auto& iter : cards) {
    ChooseCard(iter.at(0), iter.at(1), iter.at(2));
    Sleep(100);
  }

  std::puts("======= 选卡结束 =======");

  Sleep(200);
  LetsRock();
}