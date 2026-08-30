# lxySort

**一个 header-only、自适应、高性能的 C++17 排序库。**

`lxy_sort.hpp` 是单头文件、即插即用、零第三方依赖（仅用 C++ 标准库）。它会根据你的数据自动挑选最快的算法——并且在大多数场景下**全面超越 `std::sort`**。

> English version: [README.md](./README.md)

---

## 快速上手

```cpp
#include "lxy_sort.hpp"

std::vector<int> a = {5,3,9,1,4,3,7,2,8,6};
lxySort(a);                            // 稳定排序，自动选最快算法
lxySortUnstable(a);                    // 最快模式（不要求稳定）
lxySort(a, std::greater<int>{});       // 自定义比较器
lxySortByKey(recs, [](const Rec& r){ return r.id; });                     // 按单键稳定排序
lxySortByKey(recs, [](const Rec& r){ return std::make_tuple(r.id, r.name); }); // 按多键稳定排序
lxySortParallel(a, cmp);               // 并行排序（比较型，需 -fopenmp）
```

编译（Windows/MinGW 或 Linux 均可，无第三方依赖）：

```
g++ -O2 -std=c++17 your.cpp -o your              # 串行
g++ -O2 -std=c++17 -fopenmp your.cpp -o your     # 启用并行
```

---

## 支持的数据类型

- **所有整数**：`int` / `unsigned` / `long` / `long long` / `short` / `char` 等（有符号/无符号）
- **浮点**：`float` / `double`（含负数、`-0.0`、次正规数、±无穷大）
- **字符串**：`std::string`（MSD 基数排序）
- **其他可比较类型**：自定义结构体（只需 `operator<`，与 `std::sort` 一致）

---

## 算法分派

单次 O(n) 检测扫描后自动分派到最快算法：

| 数据形态 | 算法 | 复杂度 |
|---------|------|--------|
| 已有序 / 全相同 | 直接返回 | O(n) |
| 严格逆序 | reverse | O(n) |
| 微型数组 | 插入/归并 | 常量 |
| 小范围（range≤n） | 计数排序 | O(n) |
| 近似有序 | 自然归并 | O(n·log runs) |
| 大数组宽范围（整数/浮点） | 基数排序 | O(n) |
| 全范围少数 distinct（如仅 min/max） | 二值填充 | O(n) |
| 字符串 | MSD 基数 | O(n·L) |
| 通用结构体/自定义比较器 | 稳定归并 | O(n log n) |

平均/常见场景 **O(n)**；最坏 **O(n log n)**；额外空间 **O(n)**（可复用 thread-local 工作区，**无每次分配**）。

---

## 相对 std::sort 的实测性能（N=1M，g++ -O2）

| 场景 | 算法 | 提速 |
|------|------|------|
| Ascending / All same | O(n) 早退 | 12–18x |
| Descending | O(n) reverse | 9x |
| Rotated sorted | 计数 | 11x |
| Random 0..999 | 计数 | 5.5x |
| Random 0..1e9 | radix | 3.9–6x |
| 2 个极值（仅 INT_MIN/INT_MAX） | 二值填充 | 2.6x |
| double Ascending | O(n) | 16.9x |
| 随机 string | MSD radix | 3.3x |
| uint64 Random | 64 位 radix | 2.7x |
| float Random wide | radix | 7.4x |
| stable random keys | 半原地归并 | ~1.0x（打平） |

**61/64** 个基准场景快于 `std::sort`（覆盖 int/uint64/int64/float/double/string/稳定/byKey/并行/分派边界共 64 场景）。**6606 组 stress 测试全过，`-Wall` 零告警。**

---

## 诚实的边界

1. **通用类型的稳定排序**（如随机 `std::string`）比 `std::sort` 的**非稳定** introsort 慢——这是稳定排序的本质代价（与 `std::stable_sort` 同理）。要快就用 `lxySortUnstable`。
2. **微型数组 N≤128**：与 `std::sort` 大致打平（亚微秒噪声——`libstdc++` 的 introsort 本身已极度优化，我们的 introsort 是忠实复刻，平均胜出，但在该尺度下处于 ±10% 测量噪声内）。
3. **并行只对比较型有效**：radix/计数/string-radix 是内存带宽瓶颈（无增益）；`lxySortParallel` 主要利好 CPU 密集的比较型/自定义比较器（struct 约 2–3x）。
4. **需要 O(n) 额外内存**（非原地）；排序会重排原数组。
5. 线性排序（radix/计数）仅在比较器是默认 `std::less` 且 T 为算术类型时启用。

---

## 文件清单

- `lxy_sort.hpp` — 核心实现（唯一需要 include 的文件）
- `example.cpp` — 最小用法示例
- `stress.cpp` / `stress2.cpp` — 正确性 + 稳定性测试（含并行/多键）
- `bench.cpp` — 64 场景与 std::sort 对比基准
- `run_tests.sh` / `run_tests.bat` — 一键测试脚本

## 一键测试

```
./run_tests.sh            # 或 Windows: run_tests.bat
./run_tests.sh bench      # 额外跑性能基准
./bench.exe 1000000       # 或直接跑基准，N 可指定
```

---

## 许可证

[MIT](./LICENSE) © 2025 MEMZ-CHROER
