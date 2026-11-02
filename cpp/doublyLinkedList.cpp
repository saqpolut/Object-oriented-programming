#include "doublyLinkedList.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <cstring>


//конструктор / деструктор
ListD::ListD() : head(nullptr), tail(nullptr) {}

ListD::~ListD() {
    while (head != nullptr) {
        NodeD* tmp = head;
        head = head->next;
        delete tmp;
    }
    tail = nullptr;
}

//проверка
bool ListD::isEmptyD() const {
    return head == nullptr;
}
//размер
int ListD::sizeD() const {
    int count = 0;
    for (NodeD* current = head; current != nullptr; current = current->next){
        ++count;
    }
    return count;
}

//поиск
NodeD* ListD::findNodeD(const std::string& value) const {
    for (NodeD* current = head; current != nullptr; current = current->next){
        if (current->data == value){
            return current;
        }
    }
    return nullptr;
}

NodeD* ListD::getAtD(int index) const {
    if (index < 0){
        return nullptr;
    }
    NodeD* current = head;
    for (int i = 0; current != nullptr && i < index; ++i){
        current = current->next;
    }
    return current;
}

//добавление
void ListD::pushFrontD(const std::string& value) {
    NodeD* node = new NodeD(value);
    node->next = head;
    if (head != nullptr) {
        head->prev = node;
    } else {
        tail = node; //список был пуст
    }
    head = node;
}

void ListD::pushBackD(const std::string& value) {
    NodeD* node = new NodeD(value);
    node->prev = tail;
    if (tail != nullptr) {
        tail->next = node;
    } else {
        head = node; //список был пуст
    }
    tail = node;
}

bool ListD::insertAfterD(const std::string& target, const std::string& value) {
    NodeD* t = findNodeD(target);
    if (t == nullptr){
        return false;
    }

    NodeD* node = new NodeD(value);
    node->next = t->next;
    node->prev = t;

    if (t->next != nullptr) {
        t->next->prev = node;
    } else {
        tail = node; //вставили после последнего
    }
    t->next = node;
    return true;
}

bool ListD::insertBeforeD(const std::string& target, const std::string& value) {
    NodeD* t = findNodeD(target);
    if (t == nullptr){
        return false;
    }

    NodeD* node = new NodeD(value);
    node->prev = t->prev;
    node->next = t;

    if (t->prev != nullptr) {
        t->prev->next = node;
    } else {
        head = node; //вставили перед первым
    }
    t->prev = node;
    return true;
}

//удаление
bool ListD::popFrontD() {
    if (head == nullptr){
        return false;
    }

    NodeD* tmp = head;
    head = tmp->next;

    if (head != nullptr) {
        head->prev = nullptr;
    } else {
        tail = nullptr; //был единственный элемент
    }
    delete tmp;
    return true;
}

bool ListD::popBackD() {
    if (tail == nullptr){
        return false;
    }

    NodeD* tmp = tail;
    tail = tmp->prev;

    if (tail != nullptr) {
        tail->next = nullptr;
    } else {
        head = nullptr; //был единственный элемент
    }
    delete tmp;
    return true;
}

bool ListD::removeAfterD(const std::string& target) {
    NodeD* t = findNodeD(target);
    if (t == nullptr || t->next == nullptr){
        return false;
    }

    NodeD* tmp = t->next;
    t->next = tmp->next;

    if (tmp->next != nullptr) {
        tmp->next->prev = t;
    } else {
        tail = t; //удаляли последний элемент
    }
    delete tmp;
    return true;
}

bool ListD::removeBeforeD(const std::string& target) {
    NodeD* t = findNodeD(target);
    if (t == nullptr || t->prev == nullptr){
        return false;
    }

    NodeD* tmp = t->prev;
    t->prev = tmp->prev;

    if (tmp->prev != nullptr) {
        tmp->prev->next = t;
    } else {
        head = t; //удаляли голову
    }
    delete tmp;
    return true;
}

bool ListD::removeByValueD(const std::string& value) {
    NodeD* t = findNodeD(value);
    if (t == nullptr){
        return false;
    }

    if (t->prev != nullptr) {
        t->prev->next = t->next;
    } else {
        head = t->next;
    }

    if (t->next != nullptr) {
        t->next->prev = t->prev;
    } else {
        tail = t->prev;
    }
    delete t;
    return true;
}

//поиск индекса
int ListD::searchD(const std::string& value) const {
    int index = 0;
    for (NodeD* current = head; current != nullptr; current = current->next, ++index){
        if (current->data == value){
            return index;
        }
    }
    return -1;
}

//вывод
void ListD::printForwardD() const {
    if (isEmptyD()){
        std::cout << "Список пуст" << std::endl;
        return;
    }
    for (NodeD* cur = head; cur != nullptr; cur = cur->next){
        std::cout << cur->data << " <-> ";
    }
    std::cout << "nullptr" << std::endl;
}

void ListD::printBackwardD() const {
    if (isEmptyD()){
        std::cout << "Список пуст" << std::endl;
        return;
    }
    for (NodeD* cur = tail; cur != nullptr; cur = cur->prev){
        std::cout << cur->data << " <-> ";
    }
    std::cout << "nullptr" << std::endl;
}

void ListD::printIndexedD() const {
    if (isEmptyD()){
        std::cout << "Список пуст" << std::endl;
        return;
    }
    int i = 0;
    for (NodeD* cur = head; cur != nullptr; cur = cur->next, ++i){
        std::cout << "[" << i << "] " << cur->data << std::endl;
    }
}

//сериализация
bool ListD::saveListDToFile(const char* filename) const {
    std::ofstream out(filename);
    if (!out){
        return false;
    }

    for (NodeD* cur = head; cur != nullptr; cur = cur->next){
        out << cur->data << " ";
    }
    out << "\n";
    return true;
}

bool ListD::loadListDFromFile(const char* filename) {
    std::ifstream in(filename);
    if (!in){
        return false;
    }

    // очищаем старый список
    while (head != nullptr) popFrontD();

    std::string val;
    while (in >> val){
        pushBackD(val);
    }
    return true;
}

bool ListD::saveListDBinary(const char* filename) const {
    if (filename == nullptr) {
        return false;
    }

    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        return false;
    }

    const char magic[8] = {'L', 'I', 'S', 'T', 'D', '0', '1', '\0'};
    out.write(magic, sizeof(magic));

    const std::uint64_t count =
        static_cast<std::uint64_t>(sizeD());

    out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (NodeD* current = head; current != nullptr; current = current->next) {
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


bool ListD::loadListDBinary(const char* filename) {
    if (filename == nullptr) {
        return false;
    }

    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        return false;
    }

    const char expectedMagic[8] = {'L', 'I', 'S', 'T', 'D', '0', '1', '\0'};
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

    // Читаем в отдельный список, чтобы при ошибке файл
    // не повредил уже существующий список.
    ListD temporaryList;

    for (std::uint64_t i = 0; i < count; ++i) {
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

        temporaryList.pushBackD(value);
    }

    // Обмениваем указатели. При выходе temporaryList
    // освободит узлы старого списка.
    NodeD* oldHead = head;
    head = temporaryList.head;
    temporaryList.head = oldHead;

    NodeD* oldTail = tail;
    tail = temporaryList.tail;
    temporaryList.tail = oldTail;

    return true;
}
