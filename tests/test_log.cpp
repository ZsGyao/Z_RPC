/**
  ******************************************************************************
  * @file           : test_log.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-10
  ******************************************************************************
  */

#include "log.h"
#include <iostream>

int main(int argc, char* argv[]) {

    std::cout << " Log begin " << std::endl;

    DebugLog << "Hello log";

    std::cout << " Log end " << std::endl;
    return 0;
}