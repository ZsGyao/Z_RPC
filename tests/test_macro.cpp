/**
  ******************************************************************************
  * @file           : test_macrp.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-14
  ******************************************************************************
  */
#include "src/common/macro.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "***" << std::endl;
    ZRPC_ASSERT2(false, "test ZRPC_ASSERT2");
    std::cout << "---" << std::endl;
    return 0;
}