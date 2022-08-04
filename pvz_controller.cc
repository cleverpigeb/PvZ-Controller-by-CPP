#include "pvz_controller.h"

#include <cinttypes>
#include <cstdio>

#include <algorithm>
#include <chrono>
#include <queue>
#include <thread>
#include <utility>

#include <Windows.h>
#include <WinUser.h>

constexpr uint32_t kBaseAddress(0x00731C50);  // Steam植僵年度版的内存基地址
constexpr int kMaxCannonNum(24);
constexpr int kMaxIceNum(54);

static bool kIsImCoffee(false);  // 是否要补模仿咖啡豆
static bool kIsImIce(false);  //是否要补模仿寒冰菇
static bool kIsRun(false);  // 标志PCC是否正在运行中
static char kScene('\0');  // 场地（泳池为“P”，白天为“D”，以此类推）
static double kScreenHeight(0.);  // 屏幕的高
static double kScreenWidth(0.);  // 屏幕的宽
static int kCannonNum(0);  // 炮的数量
static int kCoffeeCard(0);  // 咖啡豆是第几张卡
static int kIceCard(0);  // 冰是第几张卡
static int kIceNum(0);  // 冰的数量
static int kImCoffeeCard(0);  // 模仿咖啡豆是第几张卡
static int kImIceCard(0);  // 模仿冰是第几张卡
static int kNowCannon(0);  // 现在开到第几门炮了
static int kNowIce(0);  // 现在点到第几株冰了
static time_t kIceTime(0);  // 普通寒冰菇上次被使用的时间
static time_t kImIceTime(0);  // 模仿寒冰菇上次被使用的时间

static DWORD kMemoryAddress(0);  // 下一波僵尸刷新时间所对应的内存地址
static HANDLE kProcHandle;  // 植僵进程句柄
static HWND kPvzHwnd;  // 窗口句柄
static POINT kCliPos;  // 窗口服务区左上角坐标

static bool kIsUsed[kMaxCannonNum + 5]{ false };  // 炮是否被用过了
static int kCannonList[kMaxCannonNum + 5][5]{ 0 };  // 炮的位置
static int kIceList[kMaxIceNum + 5][5]{ 0 };  // 冰的位置

std::queue<std::pair<time_t, int> > kUsedIce;  // 等待补冰的请求

// 获取游戏进程句柄并检测是否成功
bool GetProcHandle(void) {
  DWORD pvz_pid(0);

  // 获取PID
  GetWindowThreadProcessId(kPvzHwnd, &pvz_pid);

  // 检测获取PID是否成功
  if (pvz_pid) {
    printf_s("游戏进程PID：%lu\n", pvz_pid);
  } else {
    printf_s("获取进程PID失败，错误代码：%lu\n", GetLastError());
    return false;
  }

  // 获取进程句柄
  kProcHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pvz_pid);

  // 检测获取进程句柄是否成功
  if (NULL == kProcHandle) {
    printf_s("打开进程失败，错误代码：%lu\n", GetLastError());
    return false;
  } else {
    std::puts("打开进程成功！");
    return true;
  }
}

// 获取游戏窗口句柄并检测游戏是否运行
bool GetPvzHwnd(void) {
  // 获取窗口句柄
  kPvzHwnd = FindWindow(L"MainWindow", L"Plants vs. Zombies");

  // 检测游戏是否运行
  if (NULL == kPvzHwnd) {
    std::puts("您似乎没有运行《植物大战僵尸》哦~");
    return false;
  } else {
    std::puts("获取游戏窗口句柄成功！");
    return true;
  }
}

// 获取时间戳
time_t GetTimeStamp(void) {
  std::chrono::time_point<
    std::chrono::system_clock,
    std::chrono::milliseconds
  > micro_time(std::chrono::time_point_cast<std::chrono::milliseconds>
               (std::chrono::system_clock::now()));

  return micro_time.time_since_epoch().count();
}

// 读取内存静态地址
uint32_t ReadMemory(DWORD mem_address) {
  DWORD memory_address(0);
  SIZE_T read_size(0);

  // 获取内存静态地址并判断是否成功
  if (ReadProcessMemory(kProcHandle, LPVOID(mem_address), &memory_address,
                        sizeof(memory_address), &read_size) == TRUE)
    std::puts("获取内存静态地址成功！");
  else
    printf_s("获取内存静态地址失败，错误代码：%lu\n", GetLastError());

  return memory_address;
}

// 补冰线程
void FillIce(void) {
  while (kIsRun) {
    if (!kUsedIce.empty()) {
      auto now_task(kUsedIce.front());
      kUsedIce.pop();

      if (GetTimeStamp() - now_task.first < 1990 + 1000)
        Sleep(1990 + 1000 - (GetTimeStamp() - now_task.first));

      if (kImIceCard) {
        if (kIsImIce) {
          if (kImIceTime && GetTimeStamp() - kImIceTime < 50010)
            Sleep(50010 - (GetTimeStamp() - kImIceTime));

          Card(kImIceCard);
          Pnt(std::make_pair(kIceList[now_task.second][0],
                             kIceList[now_task.second][1]));
          kImIceTime = GetTimeStamp();
          printf_s("已在第%d路、第%d列放置了一株模仿寒冰菇\n",
                   kIceList[now_task.second][0], kIceList[now_task.second][1]);
        } else {
          if (kIceTime && GetTimeStamp() - kIceTime < 50010)
            Sleep(50010 - (GetTimeStamp() - kIceTime));

          Card(kIceCard);
          Pnt(std::make_pair(kIceList[now_task.second][0],
                             kIceList[now_task.second][1]));
          kIceTime = GetTimeStamp();
          printf_s("已在第%d路、第%d列放置了一株寒冰菇\n",
                   kIceList[now_task.second][0], kIceList[now_task.second][1]);
        }

        kIsImIce = !kIsImIce;
      } else {
        if (kIceTime && GetTimeStamp() - kIceTime < 50010)
          Sleep(50010 - (GetTimeStamp() - kIceTime));

        Card(kIceCard);
        Pnt(std::make_pair(kIceList[now_task.second][0],
                           kIceList[now_task.second][1]));
        kIceTime = GetTimeStamp();
        printf_s("已在第%d路、第%d列放置了一株寒冰菇\n",
                 kIceList[now_task.second][0], kIceList[now_task.second][1]);
      }
    }
  }
}

// 鼠标点下（前台）
void MouseDown(bool is_right) {
  INPUT inputs({ 0 });
  inputs.type = INPUT_MOUSE;

  // 是否是右键
  if (is_right)
    inputs.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
  else
    inputs.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

  SendInput(1, &inputs, sizeof(inputs));
}

// 移动鼠标（前台）
void MouseMove(double x, double y) {
  double fx(x * (65535.0 / kScreenWidth));
  double fy(y * (65535.0 / kScreenHeight));

  INPUT inputs({ 0 });
  inputs.type = INPUT_MOUSE;
  inputs.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
  inputs.mi.dx = LONG(fx);
  inputs.mi.dy = LONG(fy);

  SendInput(1, &inputs, sizeof(inputs));
}

// 鼠标抬起（前台）
void MouseUp(bool is_right) {
  INPUT inputs({ 0 });
  inputs.type = INPUT_MOUSE;

  // 是否是右键
  if (is_right)
    inputs.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
  else
    inputs.mi.dwFlags = MOUSEEVENTF_LEFTUP;

  SendInput(1, &inputs, sizeof(inputs));
}

// 处理用过的炮
void UsedCannon(std::queue<std::pair<time_t, int> >* used_cannons) {
  if (kIsUsed[kNowCannon])  // 如果该炮已被用过
    used_cannons->pop();  // 将此炮踢出队列，以便更新时间戳
  else
    kIsUsed[kNowCannon] = true;  // 否则将其标记为用过

  // 将用过的炮加入队列，并记录时间戳
  used_cannons->push(std::make_pair(GetTimeStamp(), kNowCannon));
}

// 初始化键控器
bool InitController(char scene, int cannon_num, int cannon_list[][5],
                    std::queue<std::pair<time_t, int> >** used_cannons,
                    int ice_num, int ice_list[][5], int coffee_card,
                    int im_coffee_card, int ice_card, int im_ice_card) {
  // 设置输出编码为UTF-8
  SetConsoleOutputCP(65001);

  // 向大家问个好~
  std::puts("欢迎大家使用本键控工具，本键控工具由him君制作");
  std::puts("本键控工具完美适配 Steam 的《植物大战僵尸年度版》哦~");
  std::puts("如果出错请检查游戏版本，实在不行可以去百度贴吧或者GitHub联系我");

  std::puts("\n======= 开始初始化 =======");

  // 初始化 used_cannon 指针
  *used_cannons = new std::queue<std::pair<time_t, int> >;

  // 获取场景
  kScene = scene;

  // 获取加农炮数量和位置
  kCannonNum = cannon_num;
  std::copy(&cannon_list[0][0], &cannon_list[0][0] + kMaxCannonNum * 5,
            &kCannonList[0][0]);

  // 获取冰的数量和位置
  kIceNum = ice_num;
  std::copy(&ice_list[0][0], &ice_list[0][0] + kMaxIceNum * 5,
            &kIceList[0][0]);

  // 获取咖啡豆、模仿咖啡豆、冰和模仿冰的植物卡位置
  kCoffeeCard = coffee_card;
  kImCoffeeCard = im_coffee_card;
  kIceCard = ice_card;
  kImIceCard = im_ice_card;

  // 获取窗口句柄并判断是否成功
  if (!GetPvzHwnd()) {
    std::puts("======= 初始化异常 =======\n\n初始化出现错误，请按回车键退出");
    std::getchar();
    return false;
  }

  // 获取窗口控件左上角坐标
  RECT win_pos;
  GetClientRect(kPvzHwnd, &win_pos);  // 获取客户端坐标系下的窗口坐标
  kCliPos = *((POINT*)&win_pos);  // 将RECT转化为POINT
  ClientToScreen(kPvzHwnd, &kCliPos);  // 将客户端坐标系转换成屏幕坐标系

  // 获取进程句柄并判断是否成功
  if (!GetProcHandle()) {
    std::puts("======= 初始化异常 =======\n\n初始化出现错误，请按回车键退出");
    std::getchar();
    return false;
  }

  // 获取僵尸刷新时间所对的内存地址
  kMemoryAddress = ReadMemory(kBaseAddress) + 0x868;
  kMemoryAddress = ReadMemory(kMemoryAddress) + 0x55b4;

  // 获取屏幕的长和宽
  kScreenWidth = GetSystemMetrics(SM_CXSCREEN) - 1;
  kScreenHeight = GetSystemMetrics(SM_CYSCREEN) - 1;

  std::puts("======= 初始化完毕 =======");
  std::puts("\n正在准备开始，请在五秒内切换至游戏界面");

  kIsRun = true;

  // 给大家留个五秒的反应时间
  Sleep(5000);

  return true;
}

// 从内存中读取下一波僵尸刷新还有多久
uint32_t CountDown(void) {
  DWORD count_down(0);
  SIZE_T read_size(0);

  // 读内存
  ReadProcessMemory(kProcHandle, LPVOID(kMemoryAddress), &count_down,
                    sizeof(count_down), &read_size);

  while (!count_down)
    ReadProcessMemory(kProcHandle, LPVOID(kMemoryAddress), &count_down,
                      sizeof(count_down), &read_size);

  return count_down;
}

// 点炮（往某个位置发一发炮）
void Cannon(double row, double column,
            std::queue<std::pair<time_t, int> >* used_cannons) {
  int i(10);
  kNowCannon %= kCannonNum;  // 更新目前发到了第几门炮

  // 点击十次防止断触
  while (i--)
    Pnt(std::make_pair(kCannonList[kNowCannon][0],
                       kCannonList[kNowCannon][1]));  // 点炮

  // 发炮
  Pnt(std::make_pair(row, column));
  SafeClick();

  printf_s("位于第%d路、第%d列的加农炮向第%g路、第%g列发射了一发炮\n",
           kCannonList[kNowCannon][0], kCannonList[kNowCannon][1],
           row, column);

  // 善后处理
  UsedCannon(used_cannons);
  ++kNowCannon;
}

// 在战斗界面选择卡片
void Card(int num) {
  Click(50 + 50 * num, 70, false);
}

// 模拟鼠标点击
void Click(double x, double y, bool is_right) {
  // MouseMove(x + kCliPos.x, y + kCliPos.y);  // 移动鼠标

  LPARAM mouse_pos(MAKELPARAM(x, y));
  PostMessage(kPvzHwnd, WM_MOUSEMOVE, 0, mouse_pos); // 移动鼠标

  if (is_right) {  // 右键
    // MouseDown(true);  // 鼠标落下
    // MouseUp(true);  // 鼠标抬起

    PostMessage(kPvzHwnd, WM_RBUTTONDOWN, 0, mouse_pos);  // 鼠标落下
    PostMessage(kPvzHwnd, WM_RBUTTONUP, 0, mouse_pos);  // 鼠标抬起
  } else {  // 左键
    // MouseDown(false);
    // MouseUp(false);

    PostMessage(kPvzHwnd, WM_LBUTTONDOWN, 0, mouse_pos);
    PostMessage(kPvzHwnd, WM_LBUTTONUP, 0, mouse_pos);
  }
}

// 与ClickForgnd()唯一的区别是中间多了等待时间
void ClickButton(double x, double y, bool is_right) {
  MouseMove(x + kCliPos.x, y + kCliPos.y);

  if (is_right) {  // 右键
    MouseDown(true);  // 鼠标落下
    Sleep(100);
    MouseUp(true);  // 鼠标抬起
  } else {  // 左键
    MouseDown(false);
    Sleep(100);
    MouseUp(false);
  }
}

// 前台模拟鼠标点击
void ClickForgnd(double x, double y, bool is_right) {
  MouseMove(x + kCliPos.x, y + kCliPos.y);  // 移动鼠标

  if (is_right) {  // 右键
    MouseDown(true);  // 鼠标落下
    MouseUp(true);  // 鼠标抬起
  } else {  // 左键
    MouseDown(false);
    MouseUp(false);
  }
}

// 在选卡界面选卡
void ChooseCard(int row, int column, bool is_imitater) {
  double x(0), y(0);

  // 判断是否是模仿者
  if (is_imitater) {
    std::puts("正在打开模仿者界面");
    ClickForgnd(500, 543, false);
    Sleep(200);
    x = 190;
    y = 125;
    printf_s("正在模仿者界面选择位于第%d行第%d列的植物卡\n", row, column);
  } else {
    x = 22;
    y = 123;
    printf_s("正在选择位于第%d行第%d列的植物卡\n", row, column);
  }

  // 将行、列转化为窗口坐标，并点击
  x += 25 + (column - 1) * 53;
  y += 35 + (row - 1) * 70;
  ClickForgnd(x, y, false);
}

// 一起摇滚吧！
void LetsRock(void) {
  std::puts("\n正在点击开始游戏按钮······");
  ClickButton(225, 565, false);
}

// 预判下一波
// 会一直等待至设定的时间
void PreJudge(int adva_time, bool is_huge_wave) {
  // 如果未到设定的时间，那就一直循环
  if (is_huge_wave) {
    while (CountDown() > 4) {}
    Sleep((724 - adva_time) * 10);
  } else {
    while (CountDown() > static_cast<uint32_t>(adva_time)) {}
  }
}

// 按一次空格，可用于女仆
void PressSpace(void) {
  /*
  INPUT inputs({ 0 });
  inputs.type = INPUT_KEYBOARD;
  inputs.ki.wVk = VK_SPACE;
  SendInput(1, &inputs, sizeof(inputs));  // 空格键按下
  inputs.ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(1, &inputs, sizeof(inputs));  // 空格键抬起
  */

  PostMessage(kPvzHwnd, WM_KEYDOWN, 0x20, 0);  // 空格键按下
  PostMessage(kPvzHwnd, WM_KEYUP, 0x20, 0);  // 空格键抬起
}

// 点击场地格子
void Pnt(const std::pair<double, double>& pnt) {
  // 要点第几行、第几列
  double row(pnt.first), column(pnt.second);

  // 判断场景
  switch (kScene) {
    case 'P': case 'F':  // 泳池和浓雾
      Click(80 * column, 55 + 85 * row, false);
      break;

    case 'R': case 'M':  // 屋顶和月夜
      Click(
        80 * column,
        column > 5 ? 45 + 85 * row : 45 + 85 * row + 20 * (6 - column),
        false
      );
      break;

    default:  // 其他
      Click(80 * column, 40 + 100 * row, false);
      break;
  }
}

// 退出键控器
void QuitController(std::queue<std::pair<time_t, int> >** used_cannons) {
  std::puts("\n感谢您使用本键控工具");
  delete* used_cannons;
  CloseHandle(kProcHandle);  // 关闭进程句柄
  kIsRun = false;
  std::puts("请按回车退出程序");
  std::getchar();
}

// 发炮，如果没有可用炮就等待至有炮冷却完毕后发
void RecoverCannon(double row, double column,
                   std::queue<std::pair<time_t, int> >* used_cannons) {
  bool is_find(false);

  // 寻找是否还有能用的炮
  for (int i(0); i < kCannonNum; ++i) {
    if (!kIsUsed[i]) {
      is_find = true;
      Cannon(row, column, used_cannons);
    }
  }

  // 如果没有能直接用的炮
  if (!is_find) {
    if (GetTimeStamp() - used_cannons->front().first > 34750) {
      // 如果其实有能用的炮，那就用
      kIsUsed[used_cannons->front().second] = false;
      used_cannons->pop();
    } else {
      // 如果没有，那就等
      Sleep(34750 - static_cast<int>(GetTimeStamp() -
            used_cannons->front().first));
    }

    Cannon(row, column, used_cannons);
  }
}

// 安全点击
void SafeClick(void) {
  Click(60, 50, true);
}

// 启动自动补冰
void StartIceFiller(void) {
  std::thread fill_ice_th(FillIce);
  fill_ice_th.detach();
}

// 点冰
void WakeIce(void) {
  kNowIce %= kIceNum;

  if (kImCoffeeCard) {
    if (kIsImCoffee) {
      Card(kImCoffeeCard);
      Pnt(std::make_pair(kIceList[kNowIce][0], kIceList[kNowIce][1]));
      printf_s("已将模仿咖啡豆种在第%d路、第%d列\n", kIceList[kNowIce][0],
               kIceList[kNowIce][1]);
    } else {
      Card(kCoffeeCard);
      Pnt(std::make_pair(kIceList[kNowIce][0], kIceList[kNowIce][1]));
      printf_s("已将咖啡豆种在第%d路、第%d列\n", kIceList[kNowIce][0],
               kIceList[kNowIce][1]);
    }

    kIsImCoffee = !kIsImCoffee;
  } else {
    Card(kCoffeeCard);
    Pnt(std::make_pair(kIceList[kNowIce][0], kIceList[kNowIce][1]));
    printf_s("已将咖啡豆种在第%d路、第%d列\n", kIceList[kNowIce][0],
             kIceList[kNowIce][1]);
  }

  kUsedIce.push(std::make_pair(GetTimeStamp(), kNowIce++));
}