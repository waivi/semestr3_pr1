#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "vector.h"
#include <string>

template<typename K, typename V>  
struct HashNode {
    std::string key;  
    V value;
    HashNode* next;
    
    HashNode(const std::string& k, const V& v) : key(k), value(v), next(nullptr) {}
};

struct HashFunction {
    size_t operator()(const std::string& key, size_t capacity) const {
        size_t hash = 5381;
        for (size_t i = 0; i < key.size(); ++i) {
            hash = ((hash << 5) + hash) + key[i];
        }
        return hash % capacity;
    }
};

template<typename K, typename V, typename Hash = HashFunction>  
class HashMap {
private:
    Vector<HashNode<K, V>*> table; //массив указателей на цепочки
    size_t size_;
    size_t capacity_;
    Hash hash_func;
    const double LOAD_FACTOR_THRESHOLD = 0.75;

    void rehash() {
        size_t new_capacity = capacity_ * 2;
        Vector<HashNode<K, V>*> new_table(new_capacity, nullptr);
        
        for (size_t i = 0; i < capacity_; ++i) {
            HashNode<K, V>* current = table[i];
            while (current != nullptr) {
                HashNode<K, V>* next = current->next;
                size_t new_index = hash_func(current->key, new_capacity);
                
                current->next = new_table[new_index];
                new_table[new_index] = current;
                
                current = next;
            }
        }
        
        table = new_table;
        capacity_ = new_capacity;
    }

public:
    HashMap(size_t capacity = 16) : size_(0), capacity_(capacity) {
        table.resize(capacity_, nullptr);
    }

    ~HashMap() {
        clear();
    }

    void put(const K& key, const V& value) {  
        if (static_cast<double>(size_) / capacity_ > LOAD_FACTOR_THRESHOLD) {
            rehash();
        }
        
        size_t index = hash_func(key, capacity_);
        HashNode<K, V>* current = table[index]; //указатель на начало цепочки
        
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }
        HashNode<K, V>* new_node = new HashNode<K, V>(key, value);
        new_node->next = table[index];
        table[index] = new_node;
        size_++;
    }

    bool get(const K& key, V& value) const {  
        size_t index = hash_func(key, capacity_);
        HashNode<K, V>* current = table[index];
        
        while (current != nullptr) {
            if (current->key == key) {
                value = current->value;
                return true;
            }
            current = current->next;
        }
        return false;
    }

    bool remove(const K& key) {  
        size_t index = hash_func(key, capacity_);
        HashNode<K, V>* current = table[index];
        HashNode<K, V>* prev = nullptr;
        
        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    table[index] = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                size_--;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    void clear() {
        for (size_t i = 0; i < capacity_; ++i) {
            HashNode<K, V>* current = table[i];
            while (current != nullptr) {
                HashNode<K, V>* next = current->next;
                delete current;
                current = next;
            }
            table[i] = nullptr;
        }
        size_ = 0;
    }

    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    double load_factor() const { return static_cast<double>(size_) / capacity_; }

    Vector<std::string> keys() const {
        Vector<std::string> result;
        for (size_t i = 0; i < capacity_; ++i) {
            HashNode<K, V>* current = table[i];
            while (current != nullptr) {
                result.push_back(current->key);
                current = current->next;
            }
        }
        return result;
    }

    Vector<V> values() const {
        Vector<V> result;
        for (size_t i = 0; i < capacity_; ++i) {
            HashNode<K, V>* current = table[i];
            while (current != nullptr) {
                result.push_back(current->value);
                current = current->next;
            }
        }
        return result;
    }
    
};

#endif