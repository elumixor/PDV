#include "bfs.h"
#include "../domains/maze.h"
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

#define id(node) node->get_identifier()
#define not_visited(node) visited.find(id(node)) == end

// quick prototype
template<template<class, class...> class R1,
        template<class, class...> class R2, class T, class... A1, class... A2>
auto join(R1<R2<T, A2...>, A1...> const &outer) {
    R1<T, A2...> joined;
    joined.reserve(std::accumulate(outer.begin(), outer.end(), std::size_t{}, [](auto size, auto const &inner) {
        return size + inner.size();
    }));
    for (auto const &inner : outer)
        joined.insert(joined.end(), inner.begin(), inner.end());
    return joined;
}

t_ptr bfs(t_ptr root) {
    if (root->is_goal()) return root;

    std::vector<t_ptr> current = std::move(root->next_states());

    std::unordered_set<t_id> visited;

    t_ptr found;

    while (!current.empty()) {
        val size = current.size();

        std::vector<std::vector<t_ptr>> next(size);

#pragma omp parallel for default(none) shared(size, current, found, visited, next) schedule(static, 8)
        for (auto i = 0u; i < size; ++i) {
            val child = current[i];
            if (child->is_goal()) {
                found = child;
                continue;
            }

            t_id id{child->get_identifier()};

            if (visited.find(id) != visited.end()) continue;

            // this may be expensive as well
            visited.insert(id);

            next[i] = std::move(child->next_states());
        }

        if (found != nullptr) return found;

        current = std::move(join(next));
    }

    return nullptr;
}