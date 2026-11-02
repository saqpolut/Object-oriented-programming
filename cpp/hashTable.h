#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <cstddef>
#include <string>

enum HashMethod {
    LINEAR_PROBING,
    DOUBLE_HASHING
};

enum SlotState {
    SLOT_EMPTY,
    SLOT_OCCUPIED,
    SLOT_DELETED
};

struct HashEntry {
    std::string key;
    void* value;
    SlotState state;
};

class HashTable {
private:
    HashEntry* entries;
    size_t capacity;
    size_t size;
    HashMethod method;
    size_t probes; //счётчик для эмпирического анализа
    size_t collisions; // счётчик для эмпирического анализа

    //статические хелперы
    static size_t hash1(const std::string& key);
    static size_t hash2(const std::string& key);
    static bool isPrime(size_t number);
    static size_t nextPrime(size_t number);
    static size_t getStep(const HashTable& table, const std::string& key);

public:
    //конструктор / деструктор
    HashTable(size_t expectedElements, HashMethod method);
    ~HashTable();

    //запрет копирования - защита от двойного delete
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    //основные операции
    bool hashFind(const std::string& key, void*& value);
    bool hashInsert(const std::string& key, void* value);
    bool hashErase(const std::string& key);

    //обёртки для int
    bool hashFindInt(const std::string& key, int& value);
    bool hashInsertInt(const std::string& key, int value);

    //геттеры
    size_t getSize() const noexcept { return size; }
    size_t getCapacity() const noexcept { return capacity; }
    size_t getProbes() const noexcept { return probes; }
    size_t getCollisions() const noexcept { return collisions; }
    bool isEmpty() const noexcept { return size == 0; }

    bool saveBinary(const std::string& filename) const;
    bool loadBinary(const std::string& filename);

};

#endif //HASH_TABLE_H