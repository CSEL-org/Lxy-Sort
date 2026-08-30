#include "lxy_sort.hpp"
#include <algorithm>
#include <vector>
#include <random>
#include <cstdio>
using namespace std;
int main(){
    mt19937 rng(9); int fails=0,tests=0;
    auto chk=[&](vector<int> v,const char*tag){vector<int>e=v;sort(e.begin(),e.end());lxySort(v);tests++;if(v!=e){if(fails<15)printf("FAIL %s n=%d\n",tag,(int)v.size());fails++;}};
    // data with a controlled number of descents spanning the 256..2048 bug zone
    for(int n : {2048,10000,100000,1000000}){
        for(int k : {150,300,700,1500,3000}){ // k swaps -> ~2k..4k descents
            vector<int> v(n); for(int i=0;i<n;i++)v[i]=i;
            for(int i=0;i<k;i++){int p=rng()%n,q=rng()%n;swap(v[p],v[q]);}
            chk(v,"moderately-sorted");
        }
    }
    // moderate disorder: shuffle blocks
    for(int n : {50000,200000}){
        vector<int> v(n); for(int i=0;i<n;i++)v[i]=i;
        for(int i=0;i<n;i++) if(rng()%7==0) swap(v[i],v[rng()%n]);
        chk(v,"7%-shuffled");
    }
    // interleaved few big runs
    for(int t=0;t<200;t++){int n=1000+rng()%90000;vector<int>v(n);for(int i=0;i<n;i++)v[i]=(i< n/3)? n-1-i : (i);chk(v,"two-runs");}
    printf("tests=%d fails=%d %s\n",tests,fails,fails?"FAILED":"ALL PASS");
    return fails?1:0;
}
