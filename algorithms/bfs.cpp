#include "bfs.h"
#include <set>
#include <queue>
#include <iostream>
#include <map>
#include <unordered_set>
#include <atomic>
#include <mutex>

typedef unsigned long long t_id;
typedef std::shared_ptr<const state> t_ptr;
#define val const auto &


t_ptr bfs(t_ptr root) {
    std::cout << root->to_string() << std::endl;
    std::cerr << root->to_string() << std::endl;

    return nullptr;
}