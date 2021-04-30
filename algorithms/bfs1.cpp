#include "bfs.h"
#include <set>
#include <queue>
#include <iostream>
#include <map>
#include <unordered_set>
#include <atomic>

typedef unsigned long long t_id;
typedef std::shared_ptr<const state> t_ptr;
#define val const auto &

template<typename T>
bool contains(const std::unordered_set<T> &set, const T &value) {
    return set.find(value) != set.end();
}

template<typename T>
class Queue {
public:
    class Node {
    public:
        T value;
        std::atomic<Node *> next{nullptr};

        explicit Node(T value) : value{value} {}
    };

    std::atomic<unsigned long> size{0};
    std::atomic<Node *> start{nullptr};
    std::atomic<Node *> end{nullptr};

    void push(T value) {
        Node *new_node{new Node(value)};

        while (true) {
            Node *current_start = start;
            if (current_start == nullptr) {
                if (start.compare_exchange_strong(current_start, new_node)) {
                    end = new_node;
                    size += 1;
                    return;
                }

                continue;
            } else {
                Node *current_end = end;
                Node *current_end_next = current_end->next;

                if (current_end->next.compare_exchange_strong(current_end_next, new_node)) {
                    end = new_node;
                    size += 1;
                    return;
                }
            }
        }
    }

    bool empty() const { return size == 0; }

    T pop() {
        while (true) {
            Node *current = start;
            T value = current->value;

            if (current == end) {
                if (start.compare_exchange_strong(current, nullptr)) {
                    end = nullptr;
                    size -= 1;
                    delete current;
                    return value;
                }
            } else {
                if (start.compare_exchange_strong(current, current->next)) {
                    size -= 1;
                    delete current;
                    return value;
                }
            }
        }
    }

    ~Queue() {
        while (start != nullptr) {
            Node *s = start;
            Node *c = s->next;

            delete s;

            start = c;
        }
    }
};

t_ptr bfs(t_ptr root) {
    if (root->is_goal()) return root;

    std::unordered_set<t_id> visited{};
    Queue<t_ptr> queue{};
    queue.push(root);

    t_ptr found = nullptr;
//    while (queue.size != 0) {
    while (queue.size > 0) {
        unsigned size = queue.size;
#pragma omp parallel for
        for (auto i = 0u; i < size; ++i) {
            if (found != nullptr) continue;
            t_ptr current;

//#pragma omp critical
            {
                current = queue.pop();
            }

            if (current->is_goal()) {
                found = current;
                continue;
            }

            for (val child : current->next_states()) {
                val id = child->get_identifier();
                if (contains(visited, id)) continue;

                queue.push(child);
                visited.insert(id);
            }
        }

        if (found != nullptr) return found;
    }

    return nullptr;
}