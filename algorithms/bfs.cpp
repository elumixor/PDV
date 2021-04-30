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
    if (root->is_goal()) return root;

    std::unordered_set<t_id> visited{};
    val end = visited.end();
    std::queue<t_ptr> queue{};
    queue.push(root);

    t_ptr found = nullptr;
//    while (queue.size != 0) {
    while (!queue.empty()) {
        unsigned size = queue.size();
//#pragma omp parallel for schedule(dynamic) num_threads(2)
        for (auto i = 0u; i < size; ++i) {
            if (found != nullptr) continue;
            t_ptr current = queue.front();
            queue.pop();

            if (current->is_goal()) {
                found = current;
                continue;
            }

            val children = current->next_states();
            val children_size = children.size();

//#pragma omp parallel for
            for (auto j = 0u; j < children_size; ++j) {
                val child = children[j];
                val id = child->get_identifier();

                if (visited.find(id) != end) continue;

                queue.push(child);
                visited.insert(id);
            }
        }

        if (found != nullptr) return found;
    }

    return nullptr;
}