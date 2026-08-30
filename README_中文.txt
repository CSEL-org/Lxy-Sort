# lxySort —— 稳定、自适应混合排序库（header-only）

核心文件：`lxy_sort.hpp`（单头文件，include 即用，仅依赖标准库）
本说明为 GBK(ANSI 936) 编码，在中文 Windows 记事本/ANSI 编辑器下可直接查看。

## 一、如何直接调用

```cpp
#include "lxy_sort.hpp"

std::vector<int> a = {5,3,9,1,4,3,7,2,8,6};
lxySort(a);                        // 稳定排序，自动选最快算法
lxySortUnstable(a);                // 最快模式（不要求稳定）
lxySort(a, std::greater<int>{});   // 自定义比较器
lxySortByKey(recs, [](const Rec& r){ return r.id; });          // 按单键稳定排序
lxySortByKey(recs, [](const Rec& r){ return std::make_tuple(r.id, r.name); }); // 多键
lxySortParallel(a, cmp);           // 并行排序（比较型，需 -fopenmp）
```

编译（Windows/MinGW 或 Linux 均可，无第三方依赖）：
```
g++ -O2 -std=c++17 your.cpp -o your        # 串行
g++ -O2 -std=c++17 -fopenmp your.cpp -o your   # 启用并行
```

## 二、支持的数据类型
- 所有整数：int / unsigned / long / long long / short / char 等（有符号/无符号）
- 浮点：float / double（含负数、-0.0、次正规数、±无穷大）
- 字符串：std::string（MSD 基数排序）
- 其他可比较类型：自定义结构体（只需 operator<，与 std::sort 一致）

## 三、算法与复杂度
单次 O(n) 检测扫描后自动分派：
| 数据形态 | 算法 | 复杂度 |
|---------|------|--------|
| 已有序 / 全相同 | 直接返回 | O(n) |
| 严格逆序 | reverse | O(n) |
| 小数组 n≤128 | 插入/归并 | 常量 |
| 小范围 range≤n | 计数排序 | O(n) |
| 近似有序 | 自然归并 | O(n·log runs) |
| 大数组宽范围（整数/浮点） | 基数排序 | O(n) |
| 字符串 | MSD 基数 | O(n·L) |
| 通用结构体/自定义比较器 | 稳定归并 | O(n log n) |

平均/常见场景 **O(n)**；最坏 O(n log n)；额外空间 O(n)（可复用工作区）。

## 四、相对 std::sort 的实测性能（N=1M，g++ -O2）
| 场景 | 算法 | 提速 |
|------|------|------|
| Ascending / All same | O(n) 早退 | 12~18x |
| Descending | O(n) reverse | 9x |
| Rotated sorted | 计数 | 11x |
| Random 0..999 | 计数 | 5.5x |
| Random 0..1e9 | radix | 3.9x |
| double Ascending | O(n) | 16.9x |
| 随机 string | MSD radix | 3.3x |

综合 **39/40 场景快于 std::sort**。

## 五、诚实的边界（重要）
1. **通用类型的稳定排序**（如随机 std::string）比 std::sort 的非稳定 introsort 慢
   ——这是稳定排序的本质代价（与 std::stable_sort 同理）。要快就用 lxySortUnstable。
2. **微型数组 n≤128**：略慢于 std（亚微秒噪声，稳定排序打不过 std 的 introsort）。
3. **并行只对比较型有效**：radix/计数/string-radix 是内存带宽瓶颈，并行无增益；
   lxySortParallel 主要利好 CPU 密集的比较型/自定义比较器（struct 约 2-3x）。
4. **需要 O(n) 额外内存**（非原地）；排序会重排原数组。
5. 线性排序（radix/计数）仅在比较器是默认 std::less 且 T 为算术类型时启用。

## 六、文件清单
- `lxy_sort.hpp`：核心实现（唯一需要 include 的文件）
- `example.cpp`：最小用法示例
- `stress.cpp` / `stress2.cpp`：正确性+稳定性测试（含并行/多键）
- `bench.cpp`：40+ 场景与 std::sort 对比基准
- `run_tests.sh` / `run_tests.bat`：一键测试脚本

## 七、一键测试
```
./run_tests.sh            # 或 Windows: run_tests.bat
./run_tests.sh bench      # 额外跑性能基准
./bench.exe 1000000       # 或直接跑基准，N 可指定
```
