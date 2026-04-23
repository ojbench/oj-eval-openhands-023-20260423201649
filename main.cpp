#include <iostream>
#include <string>
#include "deque.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    
    int n;
    std::cin >> n;
    
    sjtu::deque<int> dq;
    
    for (int i = 0; i < n; ++i) {
        std::string op;
        std::cin >> op;
        
        try {
            if (op == "push_back") {
                int x;
                std::cin >> x;
                dq.push_back(x);
            } else if (op == "push_front") {
                int x;
                std::cin >> x;
                dq.push_front(x);
            } else if (op == "pop_back") {
                dq.pop_back();
            } else if (op == "pop_front") {
                dq.pop_front();
            } else if (op == "front") {
                std::cout << dq.front() << std::endl;
            } else if (op == "back") {
                std::cout << dq.back() << std::endl;
            } else if (op == "size") {
                std::cout << dq.size() << std::endl;
            } else if (op == "empty") {
                std::cout << (dq.empty() ? "true" : "false") << std::endl;
            } else if (op == "clear") {
                dq.clear();
            } else if (op == "at") {
                int pos;
                std::cin >> pos;
                std::cout << dq.at(pos) << std::endl;
            } else if (op == "[]") {
                int pos;
                std::cin >> pos;
                std::cout << dq[pos] << std::endl;
            }
        } catch (const sjtu::exception &e) {
            std::cout << "error" << std::endl;
        }
    }
    
    return 0;
}
