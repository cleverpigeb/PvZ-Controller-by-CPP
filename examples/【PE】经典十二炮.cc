// 【PE】经典十二炮.cc：PCC键控工具的演示脚本，用于演示泳池经典十二炮
// 作者：him君

#include <cstdio>
#include <cstring>

#include <array>
#include <thread>
#include <queue>
#include <utility>

#include <Windows.h>

#include "pvz_controller.h"

constexpr int kMaxCannonNum(30);

// 以下为修改区（玉米加农炮的位置）

int kCannonList[kMaxCannonNum][5] = { {1, 5}, {2, 5}, {3, 1}, {3, 3},
                                      {3, 5}, {3, 7}, {4, 1}, {4, 3},
                                      {4, 5}, {4, 7}, {5, 5}, {6, 5} };

void ChoosingCards(void);

// 以上为修改区

int main(void) {
  // 记录用过的炮，并同时记录发炮时的时间戳
  std::queue<std::pair<time_t, int> >* used_cannons;

  // 以下为修改区（地图类型和玉米加农炮的数量）
  if (InitController('P', 12, kCannonList, &used_cannons)) {
  // 以上为修改区
    // 如果想要手动选卡可以在下面一行前面加上“//”
    ChoosingCards();

    std::puts("\n======= 战斗开始 =======");

    for (int wave(1); wave < 21; ++wave) {
      printf_s("\n正在准备处理第 %d 波的敌人\n", wave);

      // 以下为修改区（主体节奏）

      switch (wave) {
        case 10:
          PreJudge(55, true);
          Cannon(2, 9, used_cannons);
          Cannon(5, 9, used_cannons);

          Sleep(3730 - 1000);
          Card(7);
          Pnt(std::make_pair(2., 9.));
          std::puts("在第2行、第9列放置了一颗樱桃炸弹");

          break;

        case 20:
          PreJudge(150, true);
          Cannon(4, 7, used_cannons);

          Sleep(950);
          Cannon(2, 9, used_cannons);
          Cannon(5, 9, used_cannons);

          Sleep(550 + 4000);
          Cannon(2, 9, used_cannons);
          Cannon(5, 9, used_cannons);
          Cannon(2, 9, used_cannons);
          Cannon(5, 9, used_cannons);

          break;

        default:
          PreJudge(95, false);
          Cannon(2, 9, used_cannons);
          Cannon(5, 9, used_cannons);

          if (9 == wave || 19 == wave) {
            Sleep(4000);
            RecoverCannon(2, 9, used_cannons);
            RecoverCannon(5, 9, used_cannons);
            RecoverCannon(2, 9, used_cannons);
            RecoverCannon(5, 9, used_cannons);
          } else {
            Sleep(1000);
          }

          break;
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
    4, 7, 0,
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
