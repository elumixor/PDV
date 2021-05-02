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

    ~Queue() {
        Node *current = head;
        while (current != nullptr) {
            Node *next = current->next;
            delete current;
            current = next;
        }
    }
};

template<typename T>
class Stack {
    class Node {
    public:
        std::atomic<Node *> next{nullptr};
        T item;
        explicit Node(T item) : item{std::move(item)} {}
    };

    std::atomic<t_id> size{0ull};

    Node *head{new Node(T())};
public:
    void push(T item) {
        Node *node = new Node(std::move(item));
        Node *previous;

        do {
            previous = head->next;
            node->next = previous;
        } while (!head->next.compare_exchange_strong(previous, node));

        size++;
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

    ~Stack() {
        Node *current = head;
        while (current != nullptr) {
            Node *next = current->next;
            delete current;
            current = next;
        }
    }
};

class Set {
    Stack<t_id> *buckets;
    t_id num_buckets;
    t_id bitmap{1 << 20};

public:
    explicit Set(unsigned num_buckets) : num_buckets{num_buckets} {
        buckets = new Stack<t_id>[num_buckets];
    }

    void push(t_id item) {
        buckets[(item % num_buckets)].push(item);
    }

    bool contains(t_id item) {
        return buckets[item % num_buckets].contains(item);
    }

    ~Set() {
        delete[] buckets;
    }
};

t_ptr bfs(t_ptr root) {
    Set visited(100);
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