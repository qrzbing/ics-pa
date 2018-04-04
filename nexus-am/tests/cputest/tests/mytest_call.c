/*************************************************************************
	> File Name: mytest_call.c
	> Author: Qin Ruizhe
	> Mail: qrzbing@nuaa.edu.cn
	> Created Time: Wed Apr  4 22:30:02 2018
 ************************************************************************/

#include"trap.h"
int func(){
    int i = 1;
    i += 1;
    return i;
}
int main(){
    int t;
    t = func();
    t = t - 1;
    return 0;
}

