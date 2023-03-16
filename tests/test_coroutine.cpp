/**
  ******************************************************************************
  * @file           : test_coroutine.cpp
  * @author         : zgys
  * @brief          : 测试协程
  * @attention      : None
  * @date           : 23-3-8
  ******************************************************************************
  */

#include "src/common/log.h"
#include "src/common/config.h"
#include "src/common/start.h"
#include "src/coroutine/coroutine.h"
#include <iostream>
#include <thread>

void run_in_cor2() {

}

void run_in_cor1() {
    std::cout << "**********run_in_cor1 begin**********" << std::endl;

    std::cout << "**********before run_in_cor1 yield**********" << std::endl;
    zrpc::Coroutine::GetCurrentCoroutine()->yield();
    std::cout << "********** run_in_cor1 yield**********" << std::endl;

    std::cout << "**********run_in_cor1 end**********" << std::endl;
}

void test_coroutine() {

}

int main(int argc, char* argv[]) {
    // test run_in_cor1()
    zrpc::StartRpcServer();
    std::cout << "**********init start************" << std::endl;
    zrpc::Coroutine::InitMainCoroutine();
    std::cout << "**********init finish***********" << std::endl;

    std::cout << "**********main begin************" << std::endl;
    // 一旦把此处的stack_size设置为0或者设置的太小，会导致栈溢出Coer dump
    // zrpc::Coroutine::ptr cor(new zrpc::Coroutine(run_in_cor1));
    zrpc::Coroutine::ptr cor(new zrpc::Coroutine(run_in_cor1));
    cor->resume();
    std::cout << "**********main after resume**********" << std::endl;
    cor->resume();
    std::cout << "**********main end**********" << std::endl;
    //  AppDebugLog << " run over";
//*********************************************************************


    return 0;
}