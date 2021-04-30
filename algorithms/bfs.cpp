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

#define id(node) node->get_identifier()
#define not_visited(node) visited.find(id(node)) == end

t_ptr bfs(t_ptr root) {
    std::unordered_set<t_id> visited{id(root)};
    val end = visited.end();

    std::queue<t_ptr> queue{{root}};

    while (!queue.empty()) {
        auto node = queue.front();
        queue.pop();

        if (node->is_goal()) return node;

        for (val c : node->next_states())
            if (not_visited(c)) {
                queue.push(c);
                visited.insert(c->get_identifier());
            }
    }

    return nullptr;
}