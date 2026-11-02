#ifndef MASSIV_H
#define MASSIV_H

#include <string>
#include <fstream>
#include <cstddef>

class Massiv
{
private:
    std::string* data;
    size_t size;
    size_t capacity;

    bool reserve(size_t newCapacity);

public:
    Massiv();
    explicit Massiv(size_t initalCapacity);
    Massiv(const Massiv& other);
    Massiv& operator=(const Massiv& other);
    ~Massiv();

    bool addToEnd(const std::string& value);
    bool addAtIndex(size_t index, const std::string& value);
    bool removeAtIndex(size_t index);
    bool replaceAtIndex(size_t index, const std::string& value);
    bool getElement(size_t index, std::string& value) const;

    size_t getLength() const noexcept {return size;}
    size_t getCapacity() const noexcept {return capacity;}

    bool isEmpty() const noexcept {return size == 0;}

    void printMassiv() const;
    bool readMassiv();

    bool saveBinary(const std::string& filename) const;
    bool loadBinary(const std::string& filename);

    friend struct MassivTestAccess;

};


#endif //Massiv_H