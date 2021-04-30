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
        Node *next{nullptr};

        explicit Node(T value) : value{value} {}
    };

    std::atomic<unsigned long> size{0};
    Node *start{nullptr};
    Node *end{nullptr};

    std::mutex mtx{};

    void push(T value) {
        Node *new_node{new Node(value)};

        mtx.lock();

        if (start == nullptr) {
            start = new_node;
            end = new_node;
        } else {
            end->next = new_node;
            end = new_node;
        }

        mtx.unlock();

        size += 1;
    }

    bool empty() const { return size == 0; }

    T pop() {
        mtx.lock();
        T value = start->value;

        if (start == end) {
            delete start;
            start = nullptr;
            end = nullptr;
        } else {
            Node *next = start->next;
            delete start;
            start = next;
        }

        size -= 1;

        mtx.unlock();

        return value;
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

template<typename T>
class MySet {
    std::unordered_set<T> set{};
    std::mutex mtx{};
public:
    bool contains(const T &value) {
        mtx.lock();
        bool res = set.find(value) != set.end();
        mtx.unlock();
        return res;
    }

    bool insert(const T &value) {
        mtx.lock();
        set.insert(value);
        mtx.unlock();
    }
};

t_ptr bfs(t_ptr root) {
    if (root->is_goal()) return root;

//    std::unordered_set<t_id> visited{};
    MySet<t_id> visited{};
    Queue<t_ptr> queue{};
    queue.push(root);

    t_ptr found = nullptr;
//    while (queue.size != 0) {
    while (queue.size > 0) {
        unsigned size = queue.size;
//#pragma omp parallel for schedule(dynamic) num_threads(2)
#pragma omp parallel
        {
#pragma omp single
            {
                for (auto i = 0u; i < size; ++i) {
                    if (found != nullptr) continue;
                    t_ptr current = queue.pop();

                    if (current->is_goal()) {
                        found = current;
                        continue;
                    }

                    val children = current->next_states();
                    val children_size = children.size();

//#pragma omp parallel for schedule(dynamic) num_threads(2)
#pragma omp task if (children_size > 20)
                    {
                        for (auto j = 0u; j < children_size; ++j) {
                            val child = children[j];
                            val id = child->get_identifier();

                            if (visited.contains(id)) continue;

                            queue.push(child);
                            visited.insert(id);
                        }
                    }
                }
            }
        }

        if (found != nullptr) return found;
    }

    return nullptr;
}