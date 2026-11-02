#include "hashTable.h"
#include <cstdint>
#include <fstream>
#include <limits>
#include <utility>
#include <vector>


//статические хелперы
size_t HashTable::hash1(const std::string& key) {
    size_t hash = 5381;
    for (size_t i = 0; i < key.length(); ++i){
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(key[i]);
    }
    return hash;
}

size_t HashTable::hash2(const std::string& key) {
    size_t hash = 0;
    for (size_t i = 0; i < key.length(); ++i){
        hash = static_cast<unsigned char>(key[i]) + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

bool HashTable::isPrime(size_t number) {
    if (number < 2){
        return false;
    }
    for (size_t i = 2; i * i <= number; ++i){
        if (number % i == 0){
            return false;
        }
    }
    return true;
}

size_t HashTable::nextPrime(size_t number) {
    while (!isPrime(number)){
        ++number;
    }
    return number;
}

size_t HashTable::getStep(const HashTable& table, const std::string& key) {
    if (table.method == LINEAR_PROBING){
        return 1;
    }
    return 1 + (hash2(key) % (table.capacity - 1));
}

//конструктор / деструктор
HashTable::HashTable(size_t expectedElements, HashMethod method) {
    capacity = nextPrime(expectedElements * 2 + 11);
    entries = new HashEntry[capacity];
    size = 0;
    this->method = method;
    probes = 0;
    collisions = 0;

    for (size_t i = 0; i < capacity; ++i){
        entries[i].state = SLOT_EMPTY;
        entries[i].value = nullptr;
    }
}

HashTable::~HashTable() {
    delete[] entries;
    entries = nullptr;
    capacity = 0;
    size = 0;
}

//основные операции
bool HashTable::hashFind(const std::string& key, void*& value) {
    if (capacity == 0){
        return false;
    }

    size_t index = hash1(key) % capacity;
    size_t step = getStep(*this, key);

    for (size_t checked = 0; checked < capacity; ++checked){
        ++probes;
        HashEntry& entry = entries[index];

        if (entry.state == SLOT_EMPTY){
            return false;
        }

        if (entry.state == SLOT_OCCUPIED && entry.key == key){
            value = entry.value;
            return true;
        }

        ++collisions;
        index = (index + step) % capacity;
    }
    return false;
}

bool HashTable::hashInsert(const std::string& key, void* value) {
    if (capacity == 0){
        return false;
    }

    size_t index = hash1(key) % capacity;
    size_t step = getStep(*this, key);
    size_t firstDeleted = capacity;

    for (size_t checked = 0; checked < capacity; ++checked){
        ++probes;
        HashEntry& entry = entries[index];

        if (entry.state == SLOT_OCCUPIED && entry.key == key){
            entry.value = value;
            return true;
        }

        if (entry.state == SLOT_DELETED && firstDeleted == capacity){
            firstDeleted = index;
        }

        if (entry.state == SLOT_EMPTY){
            size_t target = (firstDeleted != capacity) ? firstDeleted : index;
            entries[target].key = key;
            entries[target].value = value;
            entries[target].state = SLOT_OCCUPIED;
            ++size;
            return true;
        }

        ++collisions;
        index = (index + step) % capacity;
    }

    if (firstDeleted != capacity){
        entries[firstDeleted].key = key;
        entries[firstDeleted].value = value;
        entries[firstDeleted].state = SLOT_OCCUPIED;
        ++size;
        return true;
    }

    return false;
}

bool HashTable::hashErase(const std::string& key) {
    if (capacity == 0){
        return false;
    }

    size_t index = hash1(key) % capacity;
    size_t step = getStep(*this, key);

    for (size_t checked = 0; checked < capacity; ++checked){
        ++probes;
        HashEntry& entry = entries[index];

        if (entry.state == SLOT_EMPTY){
            return false;
        }

        if (entry.state == SLOT_OCCUPIED && entry.key == key){
            entry.state = SLOT_DELETED;
            entry.value = nullptr;
            --size;
            return true;
        }

        index = (index + step) % capacity;
    }
    return false;
}

//обёртки для int
bool HashTable::hashFindInt(const std::string& key, int& value) {
    void* ptr = nullptr;
    if (hashFind(key, ptr)){
        value = static_cast<int>(reinterpret_cast<intptr_t>(ptr));
        return true;
    }
    return false;
}

bool HashTable::hashInsertInt(const std::string& key, int value) {
    return hashInsert(key, reinterpret_cast<void*>(static_cast<intptr_t>(value)));
}

bool HashTable::saveBinary(const std::string& filename) const {
    static_assert(sizeof(int) == sizeof(std::int32_t),
                  "This serialization expects 32-bit int");

    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        return false;
    }

    // Сигнатура формата: помогает распознать файл при загрузке.
    const char magic[8] = {'H', 'T', 'B', 'I', 'N', '0', '1', '\0'};
    out.write(magic, sizeof(magic));

    const std::uint32_t savedMethod =
        static_cast<std::uint32_t>(method);
    const std::uint64_t savedSize =
        static_cast<std::uint64_t>(size);

    out.write(reinterpret_cast<const char*>(&savedMethod),
              sizeof(savedMethod));
    out.write(reinterpret_cast<const char*>(&savedSize),
              sizeof(savedSize));

    // Сохраняем только занятые ячейки, а не пустые и удалённые.
    for (size_t i = 0; i < capacity; ++i) {
        if (entries[i].state != SLOT_OCCUPIED) {
            continue;
        }

        const std::uint64_t keyLength =
            static_cast<std::uint64_t>(entries[i].key.size());

        const std::int32_t value = static_cast<std::int32_t>(
            reinterpret_cast<std::intptr_t>(entries[i].value));

        out.write(reinterpret_cast<const char*>(&keyLength),
                  sizeof(keyLength));
        out.write(entries[i].key.data(),
                  static_cast<std::streamsize>(entries[i].key.size()));
        out.write(reinterpret_cast<const char*>(&value),
                  sizeof(value));
    }

    out.flush();
    return static_cast<bool>(out);
}

bool HashTable::loadBinary(const std::string& filename) {
    static_assert(sizeof(int) == sizeof(std::int32_t),
                  "This serialization expects 32-bit int");

    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        return false;
    }

    const char expectedMagic[8] = {'H', 'T', 'B', 'I', 'N', '0', '1', '\0'};
    char actualMagic[8]{};

    in.read(actualMagic, sizeof(actualMagic));
    if (!in || !std::equal(
                   std::begin(actualMagic), std::end(actualMagic),
                   std::begin(expectedMagic))) {
        return false;
    }

    std::uint32_t savedMethod = 0;
    std::uint64_t savedSize = 0;

    in.read(reinterpret_cast<char*>(&savedMethod), sizeof(savedMethod));
    in.read(reinterpret_cast<char*>(&savedSize), sizeof(savedSize));

    if (!in || savedMethod > static_cast<std::uint32_t>(DOUBLE_HASHING)) {
        return false;
    }

    // Ограничения защищают от слишком больших значений в повреждённом файле.
    if (savedSize > 1'000'000) {
        return false;
    }

    const HashMethod loadedMethod = static_cast<HashMethod>(savedMethod);
    std::vector<std::pair<std::string, std::int32_t>> records;
    records.reserve(static_cast<size_t>(savedSize));

    for (std::uint64_t i = 0; i < savedSize; ++i) {
        std::uint64_t keyLength = 0;
        in.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));

        if (!in || keyLength > 1'048'576) {
            return false;
        }

        std::string key(static_cast<size_t>(keyLength), '\0');
        in.read(key.data(), static_cast<std::streamsize>(keyLength));

        std::int32_t value = 0;
        in.read(reinterpret_cast<char*>(&value), sizeof(value));

        if (!in) {
            return false;
        }

        records.emplace_back(std::move(key), value);
    }

    // Собираем новую таблицу отдельно: при ошибке старая останется нетронутой.
    HashTable replacement(static_cast<size_t>(savedSize), loadedMethod);

    for (const auto& record : records) {
        if (!replacement.hashInsertInt(record.first,
                                       static_cast<int>(record.second))) {
            return false;
        }
    }

    if (replacement.getSize() != savedSize) {
        return false;
    }

    // Подменяем содержимое только после успешного чтения и восстановления.
    std::swap(entries, replacement.entries);
    std::swap(capacity, replacement.capacity);
    std::swap(size, replacement.size);
    std::swap(method, replacement.method);

    probes = 0;
    collisions = 0;
    return true;
}
