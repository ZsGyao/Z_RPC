/**
  ******************************************************************************
  * @file           : test_coroutine.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-8
  ******************************************************************************
  */

#include "src/common/log.h"
#include "src/coroutine/coroutine.h"
#include <iostream>

void run_in_cor2() {

}

void run_in_cor1() {
    InfoLog << "run in cor1 begin";
    
    InfoLog << "run in cor1 yield";

}

int main(int argc, char* argv[]) {
    std::cout << "main begin" << std::endl;

    zrpc::Coroutine::InitMainCoroutine();
    zrpc::Coroutine::ptr cor(new zrpc::Coroutine(run_in_cor1, 0));


    std::cout << "main end" << std::endl;
}