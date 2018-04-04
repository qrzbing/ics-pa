/*************************************************************************
	> File Name: mytest_call.c
	> Author: Qin Ruizhe
	> Mail: qrzbing@nuaa.edu.cn
	> Created Time: Wed Apr  4 22:30:02 2018
 ************************************************************************/

#include"trap.h"
void func(){
    int i = 1;
    i += 1;
    return;
}
int main(){
    func();
    return 0;
}

