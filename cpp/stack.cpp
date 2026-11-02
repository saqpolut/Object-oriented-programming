#include "stack.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <cstring>


//конструктор / деструктор
Stack::Stack() : head(nullptr) {}

Stack::~Stack() {
    while (head != nullptr){
        NodeS* tmp = head;
        head = head->next;
        delete tmp;
    }
}

//проверка на пустоту
bool Stack::isStackEmpty() const {
    return head == nullptr;
}

//основные операции
void Stack::pushStack(const std::string& value) {
    NodeS* new_node = new NodeS{value, head};
    head = new_node;
}

bool Stack::popStack(std::string& outValue) {
    if (isStackEmpty()){
        return false;
    }
    NodeS* old_head = head;
    outValue = old_head->data; //сохраняем до delete

    head = old_head->next;
    delete old_head;
    return true;
}

bool Stack::peekStack(std::string& value) const {
    if (isStackEmpty()){
        return false;
    }
    value = head->data;
    return true;
}

//вывод
void Stack::printStack() const {
    if (isStackEmpty()){
        std::cout << "стек пуст" << std::endl;
        return;
    }
    for (NodeS* current = head; current != nullptr; current = current->next){
        std::cout << current->data << " ";
    }
    std::cout << std::endl;
}

//сериализация
void Stack::saveStackHelper(NodeS* node, std::ofstream& out) {
    if (node == nullptr){
        return;
    }
    saveStackHelper(node->next, out);
    out << node->data << "\n";
}

bool Stack::saveStackToFile(const char* filename) const {
    std::ofstream out(filename);
    if (!out){
        return false;
    }
    saveStackHelper(head, out);
    return true;
}

bool Stack::loadStackFromFile(const char* filename) {
    std::ifstream in(filename);
    if (!in){
        return false;
    }

    // очищаем текущий стек перед загрузкой
    while (head != nullptr){
        NodeS* tmp = head;
        head = head->next;
        delete tmp;
    }

    std::string val;
    while (in >> val){
        pushStack(val);
    }
    return true;
}

bool Stack::saveStackBinary(const char* filename) const {
    if (filename == nullptr) {
        return false;
    }

    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        return false;
    }

    const char magic[8] = {'S', 'T', 'A', 'C', 'K', '0', '1', '\0'};
    out.write(magic, sizeof(magic));

    std::uint64_t count = 0;
    for (NodeS* current = head; current != nullptr; current = current->next) {
        ++count;
    }

    out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    // Сохраняем от вершины стека к его основанию.
    for (NodeS* current = head; current != nullptr; current = current->next) {
        const std::uint64_t length =
            static_cast<std::uint64_t>(current->data.size());

        out.write(reinterpret_cast<const char*>(&length), sizeof(length));

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


bool Stack::loadStackBinary(const char* filename) {
    if (filename == nullptr) {
        return false;
    }

    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        return false;
    }

    const char expectedMagic[8] = {'S', 'T', 'A', 'C', 'K', '0', '1', '\0'};
    char actualMagic[8]{};

    in.read(actualMagic, sizeof(actualMagic));

    if (!in ||
        std::memcmp(actualMagic, expectedMagic, sizeof(expectedMagic)) != 0) {
        return false;
    }

    std::uint64_t count = 0;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));

    if (!in || count > 1'000'000) {
        return false;
    }

    // Временный стек нужен, чтобы не менять текущий,
    // если файл окажется повреждённым.
    Stack temporaryStack;
    NodeS* loadedTail = nullptr;

    for (std::uint64_t i = 0; i < count; ++i) {
        std::uint64_t length = 0;
        in.read(reinterpret_cast<char*>(&length), sizeof(length));

        if (!in || length > 16 * 1024 * 1024) {
            return false;
        }

        std::string value(static_cast<std::size_t>(length), '\0');

        if (length > 0) {
            in.read(&value[0], static_cast<std::streamsize>(length));
        }

        if (!in) {
            return false;
        }

        NodeS* node = new NodeS;
        node->data = value;
        node->next = nullptr;

        if (temporaryStack.head == nullptr) {
            temporaryStack.head = node;
        } else {
            loadedTail->next = node;
        }

        loadedTail = node;
    }

    // Передаём старый список временному объекту,
    // чтобы он освободил его при выходе из функции.
    NodeS* oldHead = head;
    head = temporaryStack.head;
    temporaryStack.head = oldHead;

    return true;
}
