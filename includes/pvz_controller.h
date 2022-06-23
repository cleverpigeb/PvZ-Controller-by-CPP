#ifndef PVZ_CONTROLLER_BY_CPP_PVZ_CONTROLLER_H_
#define PVZ_CONTROLLER_BY_CPP_PVZ_CONTROLLER_H_

#ifdef PVZCONTROLLERBYCPPDLL_EXPORTS
#define CONTROLLER_API __declspec(dllexport)
#else
#define CONTROLLER_API __declspec(dllimport)
#endif

#include <cinttypes>

#include <queue>
#include <utility>

#include <Windows.h>

// 初始化键控器
extern "C" CONTROLLER_API bool InitController(
  char scene,
  int cannon_num,
  int cannon_list[][5],
  std::queue<std::pair<time_t, int> >** used_cannons
);
// 读取刷怪时间
extern "C" CONTROLLER_API uint32_t CountDown(void);
// 发炮，不会检测场上是否有可用炮
extern "C" CONTROLLER_API void Cannon(
  double /* row */,
  double /* column */,
  std::queue<std::pair<time_t, int> >* /* used_cannons */
);
// 在战斗界面选择卡片
extern "C" CONTROLLER_API void Card(int /* num */);
// 模拟鼠标点击
extern "C" CONTROLLER_API void Click(double /* x */, double /* y */,
                                     bool /* is_right */);
// 模拟鼠标点击（对按钮专用）
extern "C" CONTROLLER_API void ClickButton(double /* x */, double /* y */,
                                           bool /* is_right */);
// 在选卡界面选卡
extern "C" CONTROLLER_API void ChooseCard(int /* row */, int /* column */,
                                          bool /* is_imitater */);
// 点击开始
extern "C" CONTROLLER_API void LetsRock(void);
// 预判下一波
extern "C" CONTROLLER_API void PreJudge(int /* adva_time */,
                                        bool /* is_huge_wave */);
// 按一次空格键
extern "C" CONTROLLER_API void PressSpace(void);
// 点植物
extern "C" CONTROLLER_API void Pnt(const std::pair<double, double>& /* pnt */);
// 退出键控器
extern "C" CONTROLLER_API void QuitController(
  std::queue<std::pair<time_t, int> >** /* used_cannons */
);
// 发炮，如果没有可用炮就等待至有炮冷却完毕后发
extern "C" CONTROLLER_API void RecoverCannon(
  double /* row */,
  double /* column */,
  std::queue<std::pair<time_t, int> >* /* used_cannons */
);
// 安全点击，防止点击太快导致误操作
extern "C" CONTROLLER_API void SafeClick(void);

bool GetProcHandle(void);  // 获取进程句柄
bool GetPvzHwnd(void);  // 获取窗口句柄
time_t GetTimeStamp(void); // 获取时间戳
uint32_t ReadMemory(DWORD /* mem_address */);  // 获取内存静态地址
void MouseDown(bool /* is_right */);  // 鼠标落下
void MouseMove(double /* x */, double /* y */);  // 移动鼠标
void MouseUp(bool /* is_right */);  // 鼠标抬起
void UsedCannon(
  std::queue<std::pair<time_t, int> >* /* used_cannons */
);  // 处理被用过的炮

#endif