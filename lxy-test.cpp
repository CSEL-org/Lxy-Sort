// lxy-test.cpp - Benchmark for lxySort (stable, adaptive hybrid sort)
// The sort implementation lives in lxy_sort.hpp.

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
struct Clock {
    static double ns() {
        static LARGE_INTEGER f; static bool init = false;
        if (!init) { QueryPerformanceFrequency(&f); init = true; }
        LARGE_INTEGER c; QueryPerformanceCounter(&c);
        return (double)c.QuadPart * 1e9 / (double)f.QuadPart;
    }
};
#else
#include <chrono>
struct Clock {
    static double ns() { return (double)duration_cast<nanoseconds>(chrono::steady_clock::now().time_since_epoch()).count(); }
};
#endif

#include <algorithm>
#include <vector>
#include <string>
#include <random>
#include <iomanip>
#include <cstdio>
#include <functional>
using namespace std;

#include "lxy_sort.hpp"

static string padNum(double ms) {
    char b[32];
    snprintf(b, sizeof b, "%8.3fms", ms);
    return string(b);
}

template<typename T>
static void runGroup(const string& gname, mt19937& rng, vector<int> sizes,
                     vector<pair<string,function<vector<T>(int)>>> gens) {
    auto best = [&](vector<T>& v, bool useLxy, const char** alg) {
        double bestT = 1e300; bool ok = true;
        int reps = (int)v.size() < 1000 ? 100 : 12;
        for (int rep = 0; rep < reps; ++rep) {
            vector<T> c = v; const char* a = nullptr;
            double st = Clock::ns();
            if (useLxy) lxySortTrace(c, alg ? &a : nullptr); else sort(c.begin(), c.end());
            double en = Clock::ns(); if (en - st < bestT) bestT = en - st;
            if (alg) *alg = a;
            ok = ok && is_sorted(c.begin(), c.end());
        }
        return make_pair(bestT, ok);
    };

    printf("\n=== %s ===\n", gname.c_str());
    printf("%-24s %8s %11s %11s %9s %4s %s\n", "Scenario","N","lxySort","std::sort","ratio","win","algorithm");
    printf("%s\n", string(80,'-').c_str());
    double lt = 0, st_ = 0; int wins = 0, total = 0;
    for (auto& [name, gen] : gens) {
        for (int n : sizes) {
            auto v = gen(n);
            const char* alg = nullptr;
            auto [tl, ok1] = best(v, true, &alg);
            auto [ts, ok2] = best(v, false, nullptr);
            double ratio = (ts > 0 && tl > 0) ? (double)ts / tl : 0;
            lt += tl; st_ += ts; ++total; bool win = tl < ts; if (win) ++wins;
            printf("%-24s %8d %11s %11s %9.2f %4s %s\n",
                name.c_str(), n, padNum(tl/1e6).c_str(), padNum(ts/1e6).c_str(),
                ratio, win ? "LXY":"std", alg ? alg : "-");
        }
        printf("\n");
    }
    printf("%s\n", string(80,'-').c_str());
    printf("%-24s wins %d/%d   lxy total %.1fms  std total %.1fms\n\n", gname.c_str(), wins, total, lt/1e6, st_/1e6);
}

int main() {
    mt19937 rng(12345);
    vector<int> sizes = {100, 1000, 10000, 100000, 1000000};

    // ---------- int ----------
    vector<pair<string,function<vector<int>(int)>>> ints = {
        {"Random full-range", [&](int n){ vector<int> v(n); for(auto&x:v)x=(int)(rng()%2000000000u-1000000000); return v; }},
        {"Random 0..999",     [&](int n){ vector<int> v(n); for(auto&x:v)x=rng()%1000; return v; }},
        {"Duplicates 0..9",   [&](int n){ vector<int> v(n); for(auto&x:v)x=rng()%10; return v; }},
        {"Ascending",         [](int n){ vector<int> v(n); for(int i=0;i<n;i++)v[i]=i; return v; }},
        {"Descending",        [](int n){ vector<int> v(n); for(int i=0;i<n;i++)v[i]=n-i; return v; }},
        {"All same",          [](int n){ return vector<int>(n,42); }},
        {"Nearly sorted(16)", [&](int n){ vector<int> v(n); for(int i=0;i<n;i++)v[i]=i; for(int i=0;i<16;i++){int p=rng()%n,q=rng()%n; swap(v[p],v[q]);} return v; }},
        {"Negative range",    [&](int n){ vector<int> v(n); for(auto&x:v)x=(int)(rng()%200000u-100000); return v; }},
        {"Snake pattern",     [](int n){ vector<int> v(n); for(int i=0;i<n;i++)v[i]=(i%2==0)?(i/2+1):(n-i/2); return v; }},
    };
    runGroup("int", rng, sizes, ints);

    // ---------- double ----------
    vector<pair<string,function<vector<double>(int)>>> dbls = {
        {"Random full-range", [&](int n){ vector<double> v(n); for(auto&x:v)x=(double)((long long)(rng()%2000000000u)-1000000000)/3.7; return v; }},
        {"Small range",       [&](int n){ vector<double> v(n); for(auto&x:v)x=(double)(rng()%1000); return v; }},
        {"Ascending",         [](int n){ vector<double> v(n); for(int i=0;i<n;i++)v[i]=i; return v; }},
        {"Descending",        [](int n){ vector<double> v(n); for(int i=0;i<n;i++)v[i]=n-i; return v; }},
        {"Nearly sorted",     [&](int n){ vector<double> v(n); for(int i=0;i<n;i++)v[i]=i; for(int i=0;i<16;i++){int p=rng()%n,q=rng()%n; swap(v[p],v[q]);} return v; }},
        {"Mixed signs",       [&](int n){ vector<double> v(n); for(auto&x:v)x=(double)((int)(rng()%2000000)-1000000)/2.5; return v; }},
    };
    runGroup("double", rng, sizes, dbls);

    // ---------- string ----------
    auto genStr = [&](int n){ vector<string> v(n); for(auto&x:v){int L=rng()%8+1;x.reserve(L);for(int j=0;j<L;j++)x+=(char)('a'+rng()%26);} return v; };
    vector<pair<string,function<vector<string>(int)>>> strs = {
        {"Random strings",    genStr},
        {"Ascending",         [&](int n){ vector<string> v(n); for(int i=0;i<n;i++){char b[16]; snprintf(b,16,"%012d",i); v[i]=b;} return v; }},
        {"Descending",        [&](int n){ vector<string> v(n); for(int i=0;i<n;i++){char b[16]; snprintf(b,16,"%012d",n-i); v[i]=b;} return v; }},
        {"Few distinct",      [&](int n){ vector<string> v(n); for(int i=0;i<n;i++)v[i]="s"+to_string(rng()%64); return v; }},
    };
    runGroup("string", rng, sizes, strs);

    // ---------- unstable mode: random strings vs std::sort ----------
    printf("=== string (lxySortUnstable, no stability guarantee) ===\n");
    {
        auto best = [&](vector<string>& v, bool mode) {
            double bestT = 1e300; bool ok = true;
            int reps = v.size() < 1000 ? 50 : 8;
            for (int r = 0; r < reps; ++r) {
                vector<string> c = v; double st = Clock::ns();
                if (mode == 0) lxySortUnstable(c);
                else if (mode == 1) lxySort(c);
                else sort(c.begin(), c.end());
                double en = Clock::ns(); if (en - st < bestT) bestT = en - st;
                ok = ok && is_sorted(c.begin(), c.end());
            }
            return make_pair(bestT, ok);
        };
        printf("%-16s %8s %11s %11s %11s %9s %9s\n", "Scenario","N","std::sort","lxySort","lxyUnstable","std/stable","std/unstb");
        for (int n : {10000, 100000, 1000000}) {
            vector<string> v(n); for(auto&x:v){int L=rng()%8+1;x.reserve(L);for(int j=0;j<L;j++)x+=(char)('a'+rng()%26);}
            auto [ts,ok0] = best(v,2); auto [tst,ok1] = best(v,1); auto [tu,ok2] = best(v,0);
            printf("%-16s %8d %11s %11s %11s %9.2f %9.2f\n", "Random strings", n,
                padNum(ts/1e6).c_str(), padNum(tst/1e6).c_str(), padNum(tu/1e6).c_str(),
                (double)ts/tst, (double)ts/tu);
        }
        printf("\n");
    }

    // ---------- custom comparator + sort by key ----------
    printf("=== custom comparator & lxySortByKey (struct) ===\n");
    {
        struct Rec { int id; int val; };
        mt19937 r2(9);
        const int n = 200000;
        vector<Rec> recs(n); for(int i=0;i<n;i++) recs[i] = {(int)(r2()%1000), (int)(r2()%1000000)};
        // sort by id using key extraction
        auto byId = recs;
        double st = Clock::ns(); lxySortByKey(byId, [](const Rec&r){return r.id;}); double e = Clock::ns();
        bool okId = is_sorted(byId.begin(), byId.end(), [](const Rec&x,const Rec&y){return x.id<y.id;});
        printf("lxySortByKey(id): %8.3fms  sorted-by-id=%s\n", (e-st)/1e6, okId?"OK":"FAIL");
        // custom comparator (descending by val)
        auto byValDesc = recs;
        st = Clock::ns(); lxySort(byValDesc, [](const Rec&x,const Rec&y){return x.val>y.val;}); e = Clock::ns();
        bool okDesc = is_sorted(byValDesc.begin(), byValDesc.end(), [](const Rec&x,const Rec&y){return x.val>y.val;});
        printf("lxySort(desc comparator): %8.3fms  sorted-desc=%s\n", (e-st)/1e6, okDesc?"OK":"FAIL");
        // std::sort baseline
        auto stRef = recs;
        st = Clock::ns(); sort(stRef.begin(), stRef.end(), [](const Rec&x,const Rec&y){return x.val>y.val;}); e = Clock::ns();
        printf("std::sort(desc comparator): %8.3fms\n\n", (e-st)/1e6);
    }

    // ---------- stability verification ----------
    printf("=== Stability check ===\n");
    {
        struct VI { int val; int idx; bool operator<(const VI&o)const{return val<o.val;} };
        mt19937 r2(7);
        bool okStable = true;
        for (int t = 0; t < 200; t++) {
            int n = 2 + r2()%5000;
            vector<VI> v(n); for(int i=0;i<n;i++) v[i]={(int)(r2()%20), i};
            lxySort(v);
            for(int i=1;i<n;i++){
                if (v[i].val < v[i-1].val) { okStable=false; break; }
                if (!(v[i-1].val<v[i].val) && v[i-1].idx > v[i].idx) { okStable=false; break; }
            }
        }
        printf("200 random stability tests: %s\n", okStable ? "STABLE (all equal values preserved order)" : "UNSTABLE!");
    }
    return 0;
}
