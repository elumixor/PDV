#include "iddfs.h"
#include <set>
#include <queue>
#include <iostream>
#include <map>
#include <unordered_set>
#include <atomic>
#include <mutex>
#include <functional>

typedef unsigned long long t_id;
typedef std::shared_ptr<const state> t_ptr;
#define val const auto &

#define id(node) node->get_identifier()
#define not_visited(node) visited.find(id(node)) == end

std::shared_ptr<const state> iddfs(std::shared_ptr<const state> root) {
    std::unordered_set<t_id> visited{};
    val end = visited.end();

    std::function<t_ptr(t_ptr, unsigned)> DLS = [&](t_ptr src, unsigned limit) -> t_ptr {
        if (src->is_goal())
            return src;

        if (limit <= 0)
            return nullptr;

        for (val child : src->next_states()) {
            if (!(not_visited(child))) continue;

            visited.insert(id(child));

            val result = DLS(child, limit - 1);
            if (result != nullptr)
                return result;
        }

        return nullptr;
    };

    for (auto i = 0u; i < 100; ++i) {
        visited.clear();

        val result = DLS(root, i);
        if (result != nullptr)
            return result;
    }

    return nullptr;
}

