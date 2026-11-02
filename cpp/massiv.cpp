#include "massiv.h"
#include <iostream>
#include <new>

Massiv::Massiv() : data(nullptr), size(0), capacity(0) {}

Massiv::Massiv(size_t initialCapacity)
    : data(initialCapacity ? new std::string[initialCapacity] : nullptr),
      size(0), capacity(initialCapacity) {}

Massiv::Massiv(const Massiv& other)
    : data(other.capacity ? new std::string[other.capacity] : nullptr),
      size(other.size), capacity(other.capacity) {
    for (size_t i = 0; i < size; ++i) data[i] = other.data[i];
}

Massiv& Massiv::operator=(const Massiv& other){
    if (this != &other){
        std::string* newData = other.capacity ? new std::string[other.capacity] : nullptr;
        for (size_t i = 0; i < other.size; ++i){
            newData[i] = other.data[i];
        }
        delete[] data;
        data = newData;
        size = other.size;
        capacity = other.capacity;
    }
    return *this;
}

Massiv::~Massiv(){
    delete[] data;
}

bool Massiv::reserve(size_t newCapacity) {
    if (newCapacity <= capacity){
        return true;
    }
    std::string* newData = new (std::nothrow) std::string[newCapacity];
    if (!newData){
        return false;
    }
    for (size_t i = 0; i < size; ++i){
        newData[i] = data[i];
    }
    delete[] data;
    data = newData;
    capacity = newCapacity;
    return true;
}

bool Massiv::addToEnd(const std::string& value) {
    if (size == capacity) {
        size_t newCap = (capacity == 0) ? 1 : capacity * 2;
        if (!reserve(newCap)) return false;
    }
    data[size++] = value;
    return true;
}

bool Massiv::addAtIndex(size_t index, const std::string& value) {
    if (index > size) return false;
    if (size == capacity) {
        size_t newCap = (capacity == 0) ? 1 : capacity * 2;
        if (!reserve(newCap)) return false;
    }
    for (size_t i = size; i > index; --i) data[i] = data[i - 1];
    data[index] = value;
    ++size;
    return true;
}

bool Massiv::getElement(size_t index, std::string& value) const {
    if (index >= size) return false;
    value = data[index];
    return true;
}

bool Massiv::removeAtIndex(size_t index) {
    if (index >= size) return false;
    for (size_t i = index; i + 1 < size; ++i) data[i] = data[i + 1];
    --size;
    return true;
}

bool Massiv::replaceAtIndex(size_t index, const std::string& value) {
    if (index >= size) return false;
    data[index] = value;
    return true;
}

void Massiv::printMassiv() const {
    if (size == 0){
        std::cout << "Массив пуст" << std::endl;
        return;
    }
    for (size_t i = 0; i < size; ++i){
        std::cout << data[i] << ' ';
    }
    std::cout << '\n';
}

bool Massiv::readMassiv() {
    size_t count;
    std::cout << "Введите количество элементов: ";
    if (!(std::cin >> count)) return false;
    Massiv tmp;
    for (size_t i = 0; i < count; ++i) {
        std::string v;
        if (!(std::cin >> v)) return false;
        tmp.addToEnd(v);
    }
    *this = tmp;
    return true;
}

// бинарная сериализация
bool Massiv::saveBinary(const std::string& filename) const {
    std::ofstream out(filename, std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<const char*>(&size), sizeof(size));
    for (size_t i = 0; i < size; ++i) {
        size_t len = data[i].size();
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        out.write(data[i].data(), len);
    }
    return out.good();
}

bool Massiv::loadBinary(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) return false;
    size_t count;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!in) return false;
    Massiv tmp;
    for (size_t i = 0; i < count; ++i) {
        size_t len;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (!in) return false;
        std::string s(len, '\0');
        in.read(&s[0], len);
        if (!in) return false;
        tmp.addToEnd(s);
    }
    *this = tmp;
    return true;
}