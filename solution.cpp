#ifndef SJTU_EXCEPTIONS_HPP
#define SJTU_EXCEPTIONS_HPP

#include <cstddef>
#include <cstring>
#include <string>

namespace sjtu {

class exception {
protected:
    const std::string variant = "";
    std::string detail = "";
public:
    exception() {}
    exception(const exception &ec) : variant(ec.variant), detail(ec.detail) {}
    virtual std::string what() {
        return variant + " " + detail;
    }
};

class index_out_of_bound : public exception {
public:
    index_out_of_bound() {
        const_cast<std::string&>(variant) = "index_out_of_bound";
    }
};

class runtime_error : public exception {
public:
    runtime_error() {
        const_cast<std::string&>(variant) = "runtime_error";
    }
};

class invalid_iterator : public exception {
public:
    invalid_iterator() {
        const_cast<std::string&>(variant) = "invalid_iterator";
    }
};

class container_is_empty : public exception {
public:
    container_is_empty() {
        const_cast<std::string&>(variant) = "container_is_empty";
    }
};

}

#endif
#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"
#include <cstddef>

namespace sjtu {

template<class T>
class deque {
private:
    static const size_t BLOCK_SIZE = 512;
    
    struct Block {
        T* data;
        size_t capacity;
        
        Block() : data(nullptr), capacity(BLOCK_SIZE) {
            data = (T*)malloc(sizeof(T) * capacity);
        }
        
        ~Block() {
            if (data) {
                free(data);
            }
        }
        
        Block(const Block& other) : capacity(other.capacity) {
            data = (T*)malloc(sizeof(T) * capacity);
        }
        
        Block& operator=(const Block& other) {
            if (this != &other) {
                if (data) free(data);
                capacity = other.capacity;
                data = (T*)malloc(sizeof(T) * capacity);
            }
            return *this;
        }
    };
    
    Block** blocks;
    size_t blockCount;
    size_t blockCapacity;
    size_t frontBlock;
    size_t frontIndex;
    size_t backBlock;
    size_t backIndex;
    size_t totalSize;
    
    void expandBlocks() {
        size_t newCapacity = blockCapacity * 2;
        Block** newBlocks = new Block*[newCapacity];
        
        size_t offset = (newCapacity - blockCapacity) / 2;
        for (size_t i = 0; i < blockCapacity; ++i) {
            newBlocks[i + offset] = blocks[i];
        }
        for (size_t i = 0; i < offset; ++i) {
            newBlocks[i] = nullptr;
        }
        for (size_t i = offset + blockCapacity; i < newCapacity; ++i) {
            newBlocks[i] = nullptr;
        }
        
        delete[] blocks;
        blocks = newBlocks;
        frontBlock += offset;
        backBlock += offset;
        blockCapacity = newCapacity;
    }
    
    void ensureFrontBlock() {
        if (frontBlock == 0) {
            expandBlocks();
        }
        if (blocks[frontBlock - 1] == nullptr) {
            blocks[frontBlock - 1] = new Block();
        }
    }
    
    void ensureBackBlock() {
        if (backBlock >= blockCapacity - 1) {
            expandBlocks();
        }
        if (blocks[backBlock + 1] == nullptr) {
            blocks[backBlock + 1] = new Block();
        }
    }

public:
    class const_iterator;
    class iterator {
    friend class deque;
    friend class const_iterator;
    private:
        deque* container;
        size_t block;
        size_t index;
        
    public:
        iterator() : container(nullptr), block(0), index(0) {}
        iterator(deque* c, size_t b, size_t i) : container(c), block(b), index(i) {}
        
        iterator operator+(const int &n) const {
            iterator result = *this;
            result += n;
            return result;
        }
        
        iterator operator-(const int &n) const {
            iterator result = *this;
            result -= n;
            return result;
        }
        
        int operator-(const iterator &rhs) const {
            if (container != rhs.container) {
                throw invalid_iterator();
            }
            
            int result = 0;
            if (block == rhs.block) {
                result = (int)index - (int)rhs.index;
            } else if (block > rhs.block) {
                result = (int)index;
                result += (block - rhs.block - 1) * BLOCK_SIZE;
                result += BLOCK_SIZE - (int)rhs.index;
            } else {
                result = -((int)rhs.index);
                result -= (rhs.block - block - 1) * BLOCK_SIZE;
                result -= BLOCK_SIZE - (int)index;
            }
            return result;
        }
        
        iterator& operator+=(const int &n) {
            if (n == 0) return *this;
            
            if (n > 0) {
                int remaining = n;
                while (remaining > 0) {
                    int available = BLOCK_SIZE - index;
                    if (remaining < available) {
                        index += remaining;
                        remaining = 0;
                    } else {
                        remaining -= available;
                        ++block;
                        index = 0;
                    }
                }
            } else {
                int remaining = -n;
                while (remaining > 0) {
                    if (remaining <= (int)index) {
                        index -= remaining;
                        remaining = 0;
                    } else {
                        remaining -= index + 1;
                        --block;
                        index = BLOCK_SIZE - 1;
                    }
                }
            }
            return *this;
        }
        
        iterator& operator-=(const int &n) {
            return (*this) += (-n);
        }
        
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        iterator& operator++() {
            ++index;
            if (index >= BLOCK_SIZE) {
                ++block;
                index = 0;
            }
            return *this;
        }
        
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }
        
        iterator& operator--() {
            if (index == 0) {
                --block;
                index = BLOCK_SIZE - 1;
            } else {
                --index;
            }
            return *this;
        }
        
        T& operator*() const {
            if (!container || !container->blocks[block]) {
                throw invalid_iterator();
            }
            return container->blocks[block]->data[index];
        }
        
        T* operator->() const noexcept {
            return &(container->blocks[block]->data[index]);
        }
        
        bool operator==(const iterator &rhs) const {
            return container == rhs.container && block == rhs.block && index == rhs.index;
        }
        
        bool operator==(const const_iterator &rhs) const {
            return container == rhs.container && block == rhs.block && index == rhs.index;
        }
        
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
    };
    
    class const_iterator {
    friend class deque;
    friend class iterator;
    private:
        const deque* container;
        size_t block;
        size_t index;
        
    public:
        const_iterator() : container(nullptr), block(0), index(0) {}
        const_iterator(const deque* c, size_t b, size_t i) : container(c), block(b), index(i) {}
        const_iterator(const iterator& other) : container(other.container), block(other.block), index(other.index) {}
        
        const_iterator operator+(const int &n) const {
            const_iterator result = *this;
            result += n;
            return result;
        }
        
        const_iterator operator-(const int &n) const {
            const_iterator result = *this;
            result -= n;
            return result;
        }
        
        int operator-(const const_iterator &rhs) const {
            if (container != rhs.container) {
                throw invalid_iterator();
            }
            
            int result = 0;
            if (block == rhs.block) {
                result = (int)index - (int)rhs.index;
            } else if (block > rhs.block) {
                result = (int)index;
                result += (block - rhs.block - 1) * BLOCK_SIZE;
                result += BLOCK_SIZE - (int)rhs.index;
            } else {
                result = -((int)rhs.index);
                result -= (rhs.block - block - 1) * BLOCK_SIZE;
                result -= BLOCK_SIZE - (int)index;
            }
            return result;
        }
        
        const_iterator& operator+=(const int &n) {
            if (n == 0) return *this;
            
            if (n > 0) {
                int remaining = n;
                while (remaining > 0) {
                    int available = BLOCK_SIZE - index;
                    if (remaining < available) {
                        index += remaining;
                        remaining = 0;
                    } else {
                        remaining -= available;
                        ++block;
                        index = 0;
                    }
                }
            } else {
                int remaining = -n;
                while (remaining > 0) {
                    if (remaining <= (int)index) {
                        index -= remaining;
                        remaining = 0;
                    } else {
                        remaining -= index + 1;
                        --block;
                        index = BLOCK_SIZE - 1;
                    }
                }
            }
            return *this;
        }
        
        const_iterator& operator-=(const int &n) {
            return (*this) += (-n);
        }
        
        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        const_iterator& operator++() {
            ++index;
            if (index >= BLOCK_SIZE) {
                ++block;
                index = 0;
            }
            return *this;
        }
        
        const_iterator operator--(int) {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }
        
        const_iterator& operator--() {
            if (index == 0) {
                --block;
                index = BLOCK_SIZE - 1;
            } else {
                --index;
            }
            return *this;
        }
        
        const T& operator*() const {
            if (!container || !container->blocks[block]) {
                throw invalid_iterator();
            }
            return container->blocks[block]->data[index];
        }
        
        const T* operator->() const noexcept {
            return &(container->blocks[block]->data[index]);
        }
        
        bool operator==(const iterator &rhs) const {
            return container == rhs.container && block == rhs.block && index == rhs.index;
        }
        
        bool operator==(const const_iterator &rhs) const {
            return container == rhs.container && block == rhs.block && index == rhs.index;
        }
        
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
    };
    
    deque() {
        blockCapacity = 8;
        blocks = new Block*[blockCapacity];
        for (size_t i = 0; i < blockCapacity; ++i) {
            blocks[i] = nullptr;
        }
        
        frontBlock = backBlock = blockCapacity / 2;
        blocks[frontBlock] = new Block();
        frontIndex = backIndex = BLOCK_SIZE / 2;
        totalSize = 0;
        blockCount = 1;
    }
    
    deque(const deque &other) {
        blockCapacity = other.blockCapacity;
        blocks = new Block*[blockCapacity];
        
        for (size_t i = 0; i < blockCapacity; ++i) {
            if (other.blocks[i] != nullptr) {
                blocks[i] = new Block(*other.blocks[i]);
                for (size_t j = 0; j < BLOCK_SIZE; ++j) {
                    if (i > other.frontBlock || (i == other.frontBlock && j >= other.frontIndex)) {
                        if (i < other.backBlock || (i == other.backBlock && j < other.backIndex)) {
                            new (&blocks[i]->data[j]) T(other.blocks[i]->data[j]);
                        }
                    }
                }
            } else {
                blocks[i] = nullptr;
            }
        }
        
        frontBlock = other.frontBlock;
        frontIndex = other.frontIndex;
        backBlock = other.backBlock;
        backIndex = other.backIndex;
        totalSize = other.totalSize;
        blockCount = other.blockCount;
    }
    
    ~deque() {
        clear();
        for (size_t i = 0; i < blockCapacity; ++i) {
            if (blocks[i] != nullptr) {
                delete blocks[i];
            }
        }
        delete[] blocks;
    }
    
    deque &operator=(const deque &other) {
        if (this == &other) return *this;
        
        clear();
        for (size_t i = 0; i < blockCapacity; ++i) {
            if (blocks[i] != nullptr) {
                delete blocks[i];
            }
        }
        delete[] blocks;
        
        blockCapacity = other.blockCapacity;
        blocks = new Block*[blockCapacity];
        
        for (size_t i = 0; i < blockCapacity; ++i) {
            if (other.blocks[i] != nullptr) {
                blocks[i] = new Block(*other.blocks[i]);
                for (size_t j = 0; j < BLOCK_SIZE; ++j) {
                    if (i > other.frontBlock || (i == other.frontBlock && j >= other.frontIndex)) {
                        if (i < other.backBlock || (i == other.backBlock && j < other.backIndex)) {
                            new (&blocks[i]->data[j]) T(other.blocks[i]->data[j]);
                        }
                    }
                }
            } else {
                blocks[i] = nullptr;
            }
        }
        
        frontBlock = other.frontBlock;
        frontIndex = other.frontIndex;
        backBlock = other.backBlock;
        backIndex = other.backIndex;
        totalSize = other.totalSize;
        blockCount = other.blockCount;
        
        return *this;
    }
    
    T & at(const size_t &pos) {
        if (pos >= totalSize) {
            throw index_out_of_bound();
        }
        
        size_t currentPos = pos + frontIndex;
        size_t blockIdx = frontBlock + currentPos / BLOCK_SIZE;
        size_t idx = currentPos % BLOCK_SIZE;
        
        return blocks[blockIdx]->data[idx];
    }
    
    const T & at(const size_t &pos) const {
        if (pos >= totalSize) {
            throw index_out_of_bound();
        }
        
        size_t currentPos = pos + frontIndex;
        size_t blockIdx = frontBlock + currentPos / BLOCK_SIZE;
        size_t idx = currentPos % BLOCK_SIZE;
        
        return blocks[blockIdx]->data[idx];
    }
    
    T & operator[](const size_t &pos) {
        return at(pos);
    }
    
    const T & operator[](const size_t &pos) const {
        return at(pos);
    }
    
    const T & front() const {
        if (totalSize == 0) {
            throw container_is_empty();
        }
        return blocks[frontBlock]->data[frontIndex];
    }
    
    const T & back() const {
        if (totalSize == 0) {
            throw container_is_empty();
        }
        size_t idx = (backIndex == 0) ? BLOCK_SIZE - 1 : backIndex - 1;
        size_t blk = (backIndex == 0) ? backBlock - 1 : backBlock;
        return blocks[blk]->data[idx];
    }
    
    iterator begin() {
        return iterator(this, frontBlock, frontIndex);
    }
    
    const_iterator cbegin() const {
        return const_iterator(this, frontBlock, frontIndex);
    }
    
    iterator end() {
        return iterator(this, backBlock, backIndex);
    }
    
    const_iterator cend() const {
        return const_iterator(this, backBlock, backIndex);
    }
    
    bool empty() const {
        return totalSize == 0;
    }
    
    size_t size() const {
        return totalSize;
    }
    
    void clear() {
        if (totalSize == 0) return;
        
        for (size_t b = frontBlock; b <= backBlock; ++b) {
            if (blocks[b] == nullptr) continue;
            
            size_t startIdx = (b == frontBlock) ? frontIndex : 0;
            size_t endIdx = (b == backBlock) ? backIndex : BLOCK_SIZE;
            
            for (size_t i = startIdx; i < endIdx; ++i) {
                blocks[b]->data[i].~T();
            }
        }
        
        frontBlock = backBlock = blockCapacity / 2;
        frontIndex = backIndex = BLOCK_SIZE / 2;
        totalSize = 0;
    }
    
    iterator insert(iterator pos, const T &value) {
        if (pos.container != this) {
            throw invalid_iterator();
        }
        
        if (pos == end()) {
            push_back(value);
            return iterator(this, backBlock, backIndex - 1);
        }
        
        push_back(value);
        
        iterator it = end();
        --it;
        while (it != pos) {
            iterator prev = it;
            --prev;
            *it = *prev;
            it = prev;
        }
        
        *pos = value;
        return pos;
    }
    
    iterator erase(iterator pos) {
        if (pos.container != this || pos == end()) {
            throw invalid_iterator();
        }
        
        iterator it = pos;
        iterator next = it;
        ++next;
        
        while (next != end()) {
            *it = *next;
            ++it;
            ++next;
        }
        
        pop_back();
        return pos;
    }
    
    void push_back(const T &value) {
        if (totalSize == 0) {
            new (&blocks[frontBlock]->data[frontIndex]) T(value);
            ++totalSize;
            ++backIndex;
            return;
        }
        
        if (backIndex >= BLOCK_SIZE) {
            ensureBackBlock();
            ++backBlock;
            backIndex = 0;
        }
        
        new (&blocks[backBlock]->data[backIndex]) T(value);
        ++backIndex;
        ++totalSize;
    }
    
    void pop_back() {
        if (totalSize == 0) {
            throw container_is_empty();
        }
        
        if (backIndex == 0) {
            --backBlock;
            backIndex = BLOCK_SIZE - 1;
        } else {
            --backIndex;
        }
        
        blocks[backBlock]->data[backIndex].~T();
        --totalSize;
        
        if (totalSize == 0) {
            frontBlock = backBlock = blockCapacity / 2;
            frontIndex = backIndex = BLOCK_SIZE / 2;
        }
    }
    
    void push_front(const T &value) {
        if (totalSize == 0) {
            new (&blocks[frontBlock]->data[frontIndex]) T(value);
            ++totalSize;
            ++backIndex;
            return;
        }
        
        if (frontIndex == 0) {
            ensureFrontBlock();
            --frontBlock;
            frontIndex = BLOCK_SIZE;
        }
        
        --frontIndex;
        new (&blocks[frontBlock]->data[frontIndex]) T(value);
        ++totalSize;
    }
    
    void pop_front() {
        if (totalSize == 0) {
            throw container_is_empty();
        }
        
        blocks[frontBlock]->data[frontIndex].~T();
        ++frontIndex;
        --totalSize;
        
        if (frontIndex >= BLOCK_SIZE) {
            ++frontBlock;
            frontIndex = 0;
        }
        
        if (totalSize == 0) {
            frontBlock = backBlock = blockCapacity / 2;
            frontIndex = backIndex = BLOCK_SIZE / 2;
        }
    }
};

}

#endif
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
