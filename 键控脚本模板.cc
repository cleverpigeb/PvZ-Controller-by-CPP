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

// 以下为修改区（玉米加农炮的位置和存冰位的位置）

int kCannonList[kMaxCannonNum + 5][5] = { 0 };
int kIceList[kMaxIceNum + 5][5] = { 0 };

void ChoosingCards(void);

// 以上为修改区

int main(void) {
  // 记录用过的炮，并同时记录发炮时的时间戳
  std::queue<std::pair<time_t, int> >* used_cannons;

  // 以下为修改区（地图类型、玉米加农炮的数量、存冰位的数量和各个植物卡的位次）
  if (InitController('P', 12, kCannonList, &used_cannons,
                     0, kIceList, false, 0, 0, 0)) {
  // 以上为修改区
    // 如果想要开启自动补冰可以把下面一行前面的“//”去掉
    // StartIceFiller();

    // 如果想要手动选卡可以在下面一行前面加上“//”
    ChoosingCards();

    std::puts("\n======= 战斗开始 =======");

    for (int wave(1); wave < 21; ++wave) {
      printf_s("\n正在准备处理第 %d 波的敌人\n", wave);

      // 以下为修改区（主体节奏）



      // 以上为修改区
    }

    std::puts("\n======= 战斗结束 =======");
    QuitController(&used_cannons);
  }

  return 0;
}

void ChoosingCards(void) {
  // 以下为修改区（要选的植物卡）
  std::array<std::array<int, 3>, 10> cards{ 0 };
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