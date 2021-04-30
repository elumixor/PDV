#include "bfs.h"
#include <set>
#include <queue>
#include <iostream>
#include <map>
#include <unordered_set>
#include <atomic>
#include <mutex>
#include <omp.h>

typedef unsigned long long t_id;
typedef std::shared_ptr<const state> t_ptr;
#define val const auto &

#define id(node) node->get_identifier()
#define not_visited(node) visited.find(id(node)) == end


template<typename T>
class Queue {
private:
    class Node {
    public:
        std::atomic<Node *> next{nullptr};
        T item;
        explicit Node(T item) : item{item} {}
    };

    std::atomic<Node *> head;
    std::atomic<Node *> tail;

    std::atomic<unsigned> _size{0};
public:
    Queue() : head{new Node(T())}, tail{head.load()} {};

    void enqueue(T item) {
        Node *old_tail;
        Node *old_next;

        Node *node = new Node(item);

        bool update_ok = false;
        while (!update_ok) {
            old_tail = tail;
            old_next = old_tail->next;

            if (tail == old_tail) {
                if (old_next == nullptr) {
                    // was tail->next
                    update_ok = old_tail->next.compare_exchange_strong(old_next, node);
                } else {
                    tail.compare_exchange_strong(old_tail, old_next);
                }
            }
        }

        tail.compare_exchange_strong(old_tail, node);
        _size++;
    }

    auto empty() const { return _size == 0; }
    auto size() const { return _size.load(); }

    T dequeue() {
        T result{};

        bool update_ok = false;
        while (!update_ok) {
            Node *old_head = head;
            Node *old_tail = tail;
            Node *old_head_next = old_head->next;

            if (old_head == head) {
                if (old_head == old_tail) {
                    if (old_head_next == nullptr) {
                        return T();
                    }

                    tail.compare_exchange_strong(old_tail, old_head_next);
                } else {
                    result = old_head_next->item;
                    update_ok = head.compare_exchange_strong(old_head, old_head_next);
                }
            }
        }

        _size--;
        return result;
    }

    // TODO: destructor
};

template<typename T>
class Stack {
    class Node {
    public:
        std::atomic<Node *> next{nullptr};
        T item;
        explicit Node(T item) : item{item} {}
    };

    Node *head{new Node(T())};
public:
    void push(T item) {
        Node *node = new Node(item);
        Node *previous;

        do {
            previous = head->next;
            node->next = previous;
        } while (!head->next.compare_exchange_strong(previous, node));
    }

    bool contains(const T &item) const {
        Node *last_head = head;
        Node *current = head;
        Node *end = nullptr;

        while (true) {
            while (current != end) {
                if (current->item == item) return true;

                current = current->next;
            }

            if (last_head == head) return false;

            end = last_head;
            last_head = head;
            current = head;
        }
    }

    // TODO: destructor
};

t_ptr bfs(t_ptr root) {
    Stack<t_id> visited;
    visited.push(id(root));

    Queue<t_ptr> queue;
    queue.enqueue(root);

    t_ptr found = nullptr;

    while (!queue.empty()) {
        auto size = queue.size();

#pragma omp parallel for
        for (auto i = 0u; i < size; ++i) {
            if (found == nullptr) {
                t_ptr node = queue.dequeue();

                if (node->is_goal()) {
                    found = node;
                } else {
                    for (val c : node->next_states()) {
                        if (!visited.contains(c->get_identifier())) {
                            queue.enqueue(c);
                            visited.push(c->get_identifier());
                        }
                    }
                }
            }
        }

        if (found != nullptr) return found;
    }

    return nullptr;
}