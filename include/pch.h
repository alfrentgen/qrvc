#include <stdint.h>

#include <iostream>
#include <istream>
#include <fstream>

#include <map>
#include <vector>
#include <queue>
#include <algorithm>

#include <pthread.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <random>

#include <functional>
#include <cstdlib>
#include <string>
#include <regex>
#include <exception>
#include <utilities.h>

#define OK 0
#define FAIL -1

#define CHECK_FAIL(val)\
if(val == FAIL){\
    return FAIL;\
}\
