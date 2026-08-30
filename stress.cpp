// stress.cpp - correctness + stability check for lxySort across many types
#include "lxy_sort.hpp"
#include <algorithm>
#include <vector>
#include <string>
#include <random>
#include <climits>
#include <cstdio>
#include <cmath>
#include <utility>
using namespace std;

static mt19937 rng(2024);
static int fails = 0, tests = 0;

static void reportFail(const char* tag) {
    if (fails < 15) printf("FAIL %s\n", tag);
    fails++;
}

template<typename T>
static void checkCorrect(vector<T> v, const char* tag) {
    vector<T> exp = v;
    sort(exp.begin(), exp.end());
    lxySort(v);
    tests++;
    if (v != exp) reportFail(tag);
}

// type whose operator< compares only the value (so sorting is by value, stable)
template<typename T>
struct ValIdx {
    T val; int idx;
    bool operator<(const ValIdx& o) const { return val < o.val; }
    bool operator==(const ValIdx& o) const { return val == o.val && idx == o.idx; }
};

// stability: sort (value,index) by value, ensure equal values keep original index order
template<typename T>
static void checkStable(vector<T> values, const char* tag) {
    vector<ValIdx<T>> v(values.size());
    for (size_t i = 0; i < v.size(); ++i) v[i] = {values[i], (int)i};
    lxyStableSort(v);   // must be stable (sorts by val via operator<)
    tests++;
    for (size_t i = 1; i < v.size(); ++i)
        if (v[i] < v[i-1]) { reportFail(tag); return; }   // not sorted
    for (size_t i = 1; i < v.size(); ++i) {
        if (!(v[i-1] < v[i]) && !(v[i] < v[i-1]))          // equal values
            if (v[i-1].idx > v[i].idx) { reportFail(tag); return; }  // unstable
    }
}

// sort a struct by key with lxySortByKey + stability + custom comparator
struct Rec { int id; int val; int idx; };

int main() {
    // lxySortUnstable correctness (same checks, unstable variant)
    for (int t = 0; t < 500; t++) {
        int n = 1 + rng() % 80000;
        vector<int> v(n); for (auto& x : v) x = (int)(rng() % 4000000000u) - 2000000000;
        vector<int> exp = v; sort(exp.begin(), exp.end()); lxySortUnstable(v); tests++;
        if (v != exp) reportFail("int-random-unstable");
    }
    { vector<string> s(3000); for(auto&x:s){int L=rng()%6+1;x.reserve(L);for(int j=0;j<L;j++)x+=(char)('a'+rng()%26);}
      vector<string> e=s; sort(e.begin(),e.end()); lxySortUnstable(s); tests++; if(s!=e) reportFail("string-unstable"); }

    // custom comparator (descending)
    for (int t = 0; t < 300; t++) {
        int n = 1 + rng() % 50000;
        vector<int> v(n); for (auto& x : v) x = (int)(rng() % 1000000);
        vector<int> e = v; sort(e.begin(), e.end(), greater<int>());
        lxySort(v, greater<int>()); tests++;
        if (v != e) reportFail("desc-comparator");
    }

    // lxySortByKey: correctness + stability (struct sorted by id, equal ids keep idx order)
    for (int t = 0; t < 200; t++) {
        int n = 2 + rng() % 5000;
        vector<Rec> v(n); for(int i=0;i<n;i++) v[i] = {(int)(rng()%30), (int)(rng()%1000), i};
        lxySortByKey(v, [](const Rec&r){return r.id;});
        tests++;
        // sorted by id
        for (int i = 1; i < n; i++) if (v[i].id < v[i-1].id) { reportFail("byKey-sorted"); break; }
        // stability among equal id
        bool stable = true;
        for (int i = 1; i < n; i++) if (v[i].id == v[i-1].id && v[i].idx < v[i-1].idx) { stable=false; break; }
        if (!stable) reportFail("byKey-stable");
    }

    // multi-key sort: sort by (id, val) using tuple key, verify correctness + stability
    for (int t = 0; t < 150; t++) {
        int n = 2 + rng() % 4000;
        struct MK { int id; int val; int idx; };
        std::vector<MK> v(n);
        for (int i = 0; i < n; i++) v[i] = {(int)(rng()%10), (int)(rng()%10), i};
        lxySortByKey(v, [](const MK& r){ return std::make_tuple(r.id, r.val); });
        tests++;
        bool ok = true;
        for (int i = 1; i < n; i++) {
            if (v[i].id < v[i-1].id || (v[i].id == v[i-1].id && v[i].val < v[i-1].val)) { ok = false; break; }
        }
        if (!ok) reportFail("byKey-multikey-sorted");
        bool st = true;
        for (int i = 1; i < n; i++) {
            if (v[i].id==v[i-1].id && v[i].val==v[i-1].val && v[i].idx < v[i-1].idx) { st = false; break; }
        }
        if (!st) reportFail("byKey-multikey-stable");
    }

    // parallel sort correctness (large arrays)
    for (int t = 0; t < 40; t++) {
        int n = 200000 + rng() % 300000;
        vector<int> v(n); for(auto&x:v)x=(int)(rng()%4000000000u)-2000000000;
        vector<int> e=v; sort(e.begin(),e.end()); lxySortParallel(v); tests++; if(v!=e) reportFail("parallel-int");
        vector<double> d(n); for(auto&x:d)x=(double)((long long)(rng()%4000000000u)-2000000000LL)/3.7;
        vector<double> de=d; sort(de.begin(),de.end()); lxySortParallel(d); tests++; if(d!=de) reportFail("parallel-double");
        vector<string> s(n); for(auto&x:s){int L=rng()%6+1;x.reserve(L);for(int j=0;j<L;j++)x+=(char)('a'+rng()%26);}
        vector<string> se=s; sort(se.begin(),se.end()); lxySortParallel(s); tests++; if(s!=se) reportFail("parallel-string");
    }
    // parallel stability
    for (int t = 0; t < 30; t++) {
        int n = 200000 + rng() % 100000;
        vector<int> v(n); for(auto&x:v)x=rng()%20; checkStable(v,"stable-parallel-int");
    }

    // iterator-range support + auto-byKey
    for (int t = 0; t < 150; t++) {
        int n = 1 + rng() % 5000;
        // iterator on std::vector (default comp)
        vector<int> a(n); for(auto&x:a)x=(int)(rng()%1000000);
        vector<int> ea=a; sort(ea.begin(),ea.end());
        lxySort(a.begin(), a.end()); tests++; if(a!=ea) reportFail("iter-vector");
        // iterator on raw array
        static int raw[5001]; for(int i=0;i<n;i++) raw[i]=(int)(rng()%1000000);
        vector<int> er(raw,raw+n); sort(er.begin(),er.end());
        lxySort(raw, raw+n); tests++;
        for(int i=0;i<n;i++) if(raw[i]!=er[i]){reportFail("iter-rawarray");break;}
        // pointer + count
        lxySort(raw, (size_t)n); tests++; for(int i=1;i<n;i++) if(raw[i-1]>raw[i]){reportFail("ptr-count");break;}
        // iterator with comparator
        vector<int> b(n); for(auto&x:b)x=(int)(rng()%1000000);
        vector<int> eb=b; sort(eb.begin(),eb.end(),greater<int>());
        lxySort(b.begin(), b.end(), greater<int>()); tests++; if(b!=eb) reportFail("iter-comparator");
    }
    // auto-byKey on vector: lxySort(vec, keyLambda) routes to lxySortByKey
    for (int t = 0; t < 100; t++) {
        int n = 2 + rng() % 3000;
        struct R { int id; int v; int idx; };
        vector<R> v(n); for(int i=0;i<n;i++) v[i]={(int)(rng()%10),(int)(rng()%10),i};
        lxySort(v, [](const R& r){ return r.id; });   // auto-byKey
        tests++;
        bool ok=true; for(int i=1;i<n;i++) if(v[i].id<v[i-1].id){ok=false;break;} if(!ok) reportFail("autobykey-sorted");
        bool st=true; for(int i=1;i<n;i++) if(v[i].id==v[i-1].id && v[i].idx<v[i-1].idx){st=false;break;} if(!st) reportFail("autobykey-stable");
    }

    // ---- int correctness across branches ----
    for (int t = 0; t < 1500; t++) {
        int n = 1 + rng() % 120000;
        vector<int> v(n); for (auto& x : v) x = (int)(rng() % 4000000000u) - 2000000000;
        checkCorrect(v, "int-random");
    }
    for (int t = 0; t < 200; t++) {
        int n = 1 + rng() % 100000;
        vector<int> a(n); for (int i=0;i<n;i++) a[i]=i; checkCorrect(a,"int-asc");
        vector<int> b(n); for (int i=0;i<n;i++) b[i]=n-i; checkCorrect(b,"int-desc");
        vector<int> c(n,7); checkCorrect(c,"int-same");
        vector<int> d=a; for(int i=0;i<30;i++){int p=rng()%n,q=rng()%n;swap(d[p],d[q]);} checkCorrect(d,"int-nearly");
    }
    { vector<int> ex={INT_MAX,INT_MIN,0,-1,5,5,-5}; checkCorrect(ex,"int-extreme");
      vector<int> ex2(50000); for(auto&x:ex2)x=(rng()%2)?INT_MIN:INT_MAX; checkCorrect(ex2,"int-ext-pair"); }

    // ---- other arithmetic types ----
    for (int t = 0; t < 300; t++) {
        int n = 1 + rng() % 60000;
        vector<long long> a(n); for(auto&x:a)x=(long long)rng(); checkCorrect(a,"ll-random");
        vector<unsigned> u(n); for(auto&x:u)x=(unsigned)(rng()%4000000000u); checkCorrect(u,"uint-random");
        vector<short> s(n); for(auto&x:s)x=(short)(int)(rng()%20000-10000); checkCorrect(s,"short-random");
        vector<float> f(n); for(auto&x:f)x=(float)((int)(rng()%4000000)-2000000)/7.0f; checkCorrect(f,"float-random");
        vector<double> dd(n); for(auto&x:dd)x=(double)((long long)(rng()%4000000000u)-2000000000LL)/3.7; checkCorrect(dd,"double-random");
    }
    { vector<float> f={-0.0f,0.0f,1.0f,-1.0f,1e30f,-1e30f,3.14f,-3.14f,1e-30f,-1e-30f}; checkCorrect(f,"float-extreme");
      vector<double> dd={-0.0,0.0,1e300,-1e300,2.2,-2.2,1e-300,-1e-300}; checkCorrect(dd,"double-extreme"); }

    // ---- generic string ----
    for (int t = 0; t < 200; t++) {
        int n = 1 + rng() % 20000;
        vector<string> s(n);
        for (auto& x : s) { int L = rng()%8+1; x.reserve(L); for(int j=0;j<L;j++) x += (char)('a'+rng()%26); }
        checkCorrect(s, "string-random");
    }
    { vector<string> s(5000); for(int i=0;i<5000;i++) s[i]="str"+to_string(i%500); checkCorrect(s,"string-dup"); }

    // ---- stability across types ----
    for (int t = 0; t < 150; t++) {
        int n = 2 + rng() % 4000;
        vector<int> v(n); for(auto&x:v)x=rng()%12; checkStable(v,"stable-int");
        vector<float> f(n); for(auto&x:f)x=(float)(rng()%10); checkStable(f,"stable-float");
        vector<string> s(n); for(auto&x:s)x=string(1,'a'+(char)(rng()%4)); checkStable(s,"stable-string");
        vector<unsigned> u(n); for(auto&x:u)x=rng()%8; checkStable(u,"stable-uint");
    }

    printf("tests=%d fails=%d  %s\n", tests, fails, fails ? "FAILED" : "ALL PASS");
    return fails ? 1 : 0;
}
