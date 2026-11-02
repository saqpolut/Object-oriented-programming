#include "queue.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <cstring>

//конструктор / деструктор
Queue::Queue() : head(nullptr), tail(nullptr), count(0) {}

Queue::~Queue() {
    while (head != nullptr){
        NodeQ* tmp = head;
        head = head->next;
        delete tmp;
    }
    tail = nullptr;
    count = 0;
}

//проверка
bool Queue::isQueueEmpty() const {
    return head == nullptr;
}
//размер
int Queue::sizeQueue() const {
    return count;
}

//основные операции
void Queue::pushQueue(const std::string& value) {
    NodeQ* node = new NodeQ(value);
    if (tail == nullptr){
        head = tail = node; //очередь была пуста
    } else{
        tail->next = node;
        tail = node;
    }
    ++count;
}

bool Queue::popQueue(std::string& outValue) {
    if (isQueueEmpty()){
        return false;
    }

    NodeQ* old_head = head;
    outValue = old_head->data;

    head = old_head->next;
    if (head == nullptr){
        tail = nullptr; //убрали единственный элемент
    }

    delete old_head;
    --count;
    return true;
}

bool Queue::peekQueue(std::string& outValue) const {
    if (isQueueEmpty()){
        return false;
    }
    outValue = head->data;
    return true;
}

//вывод
void Queue::printQueue() const {
    if (isQueueEmpty()){
        std::cout << "очередь пуста" << std::endl;
        return;
    }
    for (NodeQ* cur = head; cur != nullptr; cur = cur->next){
        std::cout << cur->data << " ";
    }
    std::cout << std::endl;
}

//сериализация
bool Queue::saveQueueToFile(const char* filename) const {
    std::ofstream out(filename);
    if (!out){
        return false;
    }

    for (NodeQ* cur = head; cur != nullptr; cur = cur->next){
        out << cur->data << "\n";
    }
    return true;
}

bool Queue::loadQueueFromFile(const char* filename) {
    std::ifstream in(filename);
    if (!in){
        return false;
    }

    // очищаем текущую очередь перед загрузкой
    while (head != nullptr) {
        NodeQ* tmp = head;
        head = head->next;
        delete tmp;
    }
    tail = nullptr;
    count = 0;

    std::string val;
    // читаем в том же порядке, в каком записали, и добавляем в хвост:
    // первый прочитанный снова станет головой
    while (in >> val) {
        pushQueue(val);
    }
    return true;
}

bool Queue::saveQueueBinary(const char* filename) const {
    if (filename == nullptr) {
        return false;
    }

    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        return false;
    }

    const char magic[8] = {'Q', 'U', 'E', 'U', 'E', '0', '1', '\0'};
    out.write(magic, sizeof(magic));

    const std::uint64_t elementCount =
        static_cast<std::uint64_t>(count);

    out.write(
        reinterpret_cast<const char*>(&elementCount),
        sizeof(elementCount)
    );

    for (NodeQ* current = head; current != nullptr; current = current->next) {
        const std::uint64_t length =
            static_cast<std::uint64_t>(current->data.size());

        out.write(
            reinterpret_cast<const char*>(&length),
            sizeof(length)
        );

        if (length > 0) {
            out.write(
                current->data.data(),
                static_cast<std::streamsize>(length)
            );
        }

        if (!out) {
            return false;
        }
    }

    out.flush();
    return static_cast<bool>(out);
}


bool Queue::loadQueueBinary(const char* filename) {
    if (filename == nullptr) {
        return false;
    }

    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        return false;
    }

    const char expectedMagic[8] = {'Q', 'U', 'E', 'U', 'E', '0', '1', '\0'};
    char actualMagic[8]{};

    in.read(actualMagic, sizeof(actualMagic));

    if (!in ||
        std::memcmp(actualMagic, expectedMagic, sizeof(expectedMagic)) != 0) {
        return false;
    }

    std::uint64_t elementCount = 0;
    in.read(
        reinterpret_cast<char*>(&elementCount),
        sizeof(elementCount)
    );

    if (!in || elementCount > 1'000'000) {
        return false;
    }

    // Загружаем элементы во временную очередь.
    // При ошибке исходная очередь останется без изменений.
    Queue temporaryQueue;

    for (std::uint64_t i = 0; i < elementCount; ++i) {
        std::uint64_t length = 0;
        in.read(reinterpret_cast<char*>(&length), sizeof(length));

        if (!in || length > 16 * 1024 * 1024) {
            return false;
        }

        std::string value(
            static_cast<std::size_t>(length),
            '\0'
        );

        if (length > 0) {
            in.read(
                &value[0],
                static_cast<std::streamsize>(length)
            );
        }

        if (!in) {
            return false;
        }

        temporaryQueue.pushQueue(value);
    }

    // Меняем указатели и счётчики местами.
    // При выходе temporaryQueue удалит прежние элементы этой очереди.
    NodeQ* oldHead = head;
    head = temporaryQueue.head;
    temporaryQueue.head = oldHead;

    NodeQ* oldTail = tail;
    tail = temporaryQueue.tail;
    temporaryQueue.tail = oldTail;

    int oldCount = count;
    count = temporaryQueue.count;
    temporaryQueue.count = oldCount;

    return true;
}
