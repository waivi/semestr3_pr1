#include <iostream>
#include <vector> //!!!
#include <string>
#include <random>
#include <chrono>
#include <iomanip>
#include <climits>
using namespace std;

// Узел для метода цепочек
struct Node {
    int key;
    int value;
    Node* next;
    Node(int k, int v) : key(k), value(v), next(nullptr) {}
};

// Метод цепочек
class ChainingHashTable {
private:
    int capacity; //размер таблицы
    int size;//количество элементов
    vector<Node*> table;//вектор указателей на узлы

    int hash(int key) {
        return key % capacity;
    }

public:
    ChainingHashTable(int capacity) : capacity(capacity), size(0) {
        table.resize(capacity, nullptr);
    }

    ~ChainingHashTable() {
        for (int i = 0; i < capacity; i++) {
            Node* current = table[i];
            while (current != nullptr) {
                Node* temp = current;
                current = current->next;
                delete temp;
            }
        }
    }

    void add(int key, int value) {
        if (contains(key) != -1) return; //проверка на дубликаты

        int h = hash(key); //вычисляем хэш значение (key%capasity)
        Node* newNode = new Node(key, value); //создаём новый узел с ключом и значением

        if (table[h] == nullptr) { //вставка в таблицу если это первый элемент в цепочке будет
            table[h] = newNode;
        }
        else { //коллизия
            Node* current = table[h]; //указывает на узел первый в цепочке, на саму ячейку
            while (current->next != nullptr) {
                current = current->next;//проходим до конца цепочки
            }
            current->next = newNode; //добавляем в конец цепочки узел новый
        }
        size++;
    }

    void remove(int key) { //удалить 
        int h = hash(key);
        Node* current = table[h];
        Node* prev = nullptr; //для отслеживания предыдущего узла

        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    table[h] = current->next; //удаляем первый элемент цепочки
                }
                else {
                    prev->next = current->next; //из середины или конца
                }
                delete current;
                size--;
                return;
            }
            prev = current;
            current = current->next;
        }
    }

    int contains(int key) {//поиск - возвращает значение
        int h = hash(key);
        Node* current = table[h];//начало цепочки

        while (current != nullptr) {//проходим по цепочке
            if (current->key == key) {
                return current->value;//нашли - возвращаем значение
            }
            current = current->next;
        }
        return -1; // не найдено
    }

    string toString() {
        string result;
        for (int i = 0; i < capacity; i++) {
            result += "[" + std::to_string(i) + "]: ";
            Node* current = table[i];
            while (current != nullptr) {
                result += "(" + std::to_string(current->key) + ":" + 
                         std::to_string(current->value) + ") -> ";
                current = current->next;
            }
            result += "null\n";
        }
        return result;
    }

    // Для анализа длины цепочек
    void getChainLengths(int& minLength, int& maxLength, double& avgLength) {
        minLength = INT_MAX;
        maxLength = 0;
        int totalLength = 0;
        int nonEmptyChains = 0;

        for (int i = 0; i < capacity; i++) {
            int length = 0;
            Node* current = table[i];
            while (current != nullptr) {
                length++;
                current = current->next;
            }

            if (length > 0) {
                minLength = std::min(minLength, length);
                maxLength = std::max(maxLength, length);
                totalLength += length;
                nonEmptyChains++;
            }
        }

        if (nonEmptyChains == 0) {
            minLength = 0;
            avgLength = 0;
        }
        else {
            avgLength = static_cast<double>(totalLength) / nonEmptyChains;
        }
    }

    // Геттеры
    int getSize() const { return size; }
    int getCapacity() const { return capacity; }
    double getLoadFactor() const { return static_cast<double>(size) / capacity; }
};

// Генератор случайных чисел
class RandomGenerator {
private:
    mt19937 gen;//генератор
    uniform_int_distribution<int> dist; //равномерное распределение

public:
    RandomGenerator(int min = 0, int max = 10000)
        : gen(std::chrono::steady_clock::now().time_since_epoch().count()), dist(min, max) {
    }

    int generate() {
        return dist(gen);
    }

    vector<int> generateSequence(int n) {
        vector<int> sequence;
        for (int i = 0; i < n; i++) {
            sequence.push_back(generate());
        }
        return sequence;
    }

    // Генерация отдельно ключей и значений
    void generateKeysAndValues(int n, vector<int>& keys, vector<int>& values) {
        keys.clear();
        values.clear();
        for (int i = 0; i < n; i++) {
            keys.push_back(generate());
            values.push_back(generate());
        }
    }
};