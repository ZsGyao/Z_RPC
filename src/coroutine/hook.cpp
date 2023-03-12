/**
  ******************************************************************************
  * @file           : hook.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-11
  ******************************************************************************
  */

#include "src/coroutine/hook.h"
#include "src/common/config.h"
#include <dlfcn.h>

#define HOOK_SYS_FUNC(name) name##_fun_ptr_t g_sys_##name##_fun = (name##_fun_ptr_t)dlsym(RTLD_NEXT, #name);

HOOK_SYS_FUNC(accept);
HOOK_SYS_FUNC(read);
HOOK_SYS_FUNC(write);
HOOK_SYS_FUNC(connect);
HOOK_SYS_FUNC(sleep);

namespace zrpc {

    extern std::shared_ptr<Config> zRpcConfig;

    static bool g_hook = true;

    void SetHook(bool) {

    }
}
