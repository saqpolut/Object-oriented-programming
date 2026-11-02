#include "singlyLinkedList.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <cstring>
//kонструктор / деструктор
ListF::ListF() : head(nullptr), tail(nullptr) {}

ListF::~ListF() {
    while (head != nullptr) {
        NodeF* tmp = head;
        head = head->next;
        delete tmp;
    }
    tail = nullptr;
}

//проверка
bool ListF::isEmptyF() const {
    return head == nullptr;
}

//размер
int ListF::sizeF() const {
    int count = 0;
    for (NodeF* current = head; current != nullptr; current = current->next)
        ++count;
    return count;
}

//поиск
NodeF* ListF::findNodeF(const std::string& value) const {
    for (NodeF* current = head; current != nullptr; current = current->next)
        if (current->data == value){
            return current;
        }
    return nullptr;
}

NodeF* ListF::getAtF(int index) const {
    if (index < 0) return nullptr;
    NodeF* cur = head;
    for (int i = 0; cur != nullptr && i < index; ++i)
        cur = cur->next;
    return cur;
}

//добавление
void ListF::pushFrontF(const std::string& value) {
    NodeF* node = new NodeF(value);
    node->next = head;
    head = node;
    if (tail == nullptr){
        tail = node;
    }
}

void ListF::pushBackF(const std::string& value) {
    NodeF* node = new NodeF(value);
    if (tail == nullptr) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
}

bool ListF::insertAfterF(const std::string& target, const std::string& value) {
    NodeF* targetNode = findNodeF(target);
    if (targetNode == nullptr) return false;

    NodeF* node = new NodeF(value);
    node->next = targetNode->next;
    targetNode->next = node;

    if (tail == targetNode){
        tail = node;
    }
    return true;
}

bool ListF::insertBeforeF(const std::string& target, const std::string& value) {
    if (head == nullptr){
        return false;
    }

    if (head->data == target) {
        pushFrontF(value);
        return true;
    }

    NodeF* prev = head;
    while (prev->next != nullptr && prev->next->data != target)
        prev = prev->next;

    if (prev->next == nullptr){
        return false;
    }

    NodeF* node = new NodeF(value);
    node->next = prev->next;
    prev->next = node;
    return true;
}

//удаление
bool ListF::popFrontF() {
    if (head == nullptr){
        return false;
    }

    NodeF* tmp = head;
    head = head->next;
    if (head == nullptr){
        tail = nullptr;
    }
    delete tmp;
    return true;
}

bool ListF::popBackF() {
    if (head == nullptr){
        return false;
    }

    if (head == tail) {
        delete head;
        head = tail = nullptr;
        return true;
    }

    NodeF* prev = head;
    while (prev->next != tail){
        prev = prev->next;
    }

    delete tail;
    tail = prev;
    prev->next = nullptr;
    return true;
}

bool ListF::removeAfterF(const std::string& target) {
    NodeF* targetNode = findNodeF(target);
    if (targetNode == nullptr || targetNode->next == nullptr){
        return false;
    }

    NodeF* tmp = targetNode->next;
    targetNode->next = tmp->next;

    if (tail == tmp){
        tail = targetNode;
    }
    delete tmp;
    return true;
}

bool ListF::removeBeforeF(const std::string& target) {
    if (head == nullptr || head->data == target){
        return false;
    }

    if (head->next != nullptr && head->next->data == target){
        return popFrontF();
    }

    NodeF* prev = head;
    while (prev->next != nullptr 
        && prev->next->next != nullptr 
        &&  prev->next->next->data != target){
        prev = prev->next;
        }

    if (prev->next == nullptr || prev->next->next == nullptr){
        return false;
    }

    NodeF* tmp = prev->next;
    prev->next = tmp->next;
    delete tmp;
    return true;
}

bool ListF::removeByValueF(const std::string& value) {
    if (head == nullptr){
        return false;
    }

    if (head->data == value){
        return popFrontF();
    }

    NodeF* prev = head;
    while (prev->next != nullptr && prev->next->data != value){
        prev = prev->next;
    }

    if (prev->next == nullptr){
        return false;
    }

    NodeF* tmp = prev->next;
    prev->next = tmp->next;
    if (tail == tmp){
        tail = prev;
    }
    delete tmp;
    return true;
}

//поиск индекса
int ListF::searchF(const std::string& value) const {
    int index = 0;
    for (NodeF* cur = head; cur != nullptr; cur = cur->next, ++index){
        if (cur->data == value){
            return index;
        }
    }
    return -1;
}

//вывод
void ListF::printBackwardRecF(NodeF* node) {
    if (node == nullptr){
        return;
    }
    printBackwardRecF(node->next);
    std::cout << node->data << " ";
}

void ListF::printForwardF() const {
    if (isEmptyF()) {
        std::cout << "Список пуст" << std::endl;
        return;
    }
    for (NodeF* cur = head; cur != nullptr; cur = cur->next){
        std::cout << cur->data << " -> ";
    }
    std::cout << "nullptr" << std::endl;
}

void ListF::printBackwardF() const {
    if (isEmptyF()) {
        std::cout << "Список пуст" << std::endl;
        return;
    }
    printBackwardRecF(head);
    std::cout << std::endl;
}

void ListF::printIndexedF() const {
    if (isEmptyF()) {
        std::cout << "Список пуст" << std::endl;
        return;
    }
    int i = 0;
    for (NodeF* cur = head; cur != nullptr; cur = cur->next, ++i){
        std::cout << "[" << i << "] " << cur->data << std::endl;
    }
}

//сериализация
bool ListF::saveListFToFile(const char* filename) const {
    std::ofstream out(filename);
    if (!out){
        return false;
    }

    for (NodeF* cur = head; cur != nullptr; cur = cur->next){
        out << cur->data << " ";
    }
    out << "\n";
    return true;
}

bool ListF::loadListFFromFile(const char* filename) {
    std::ifstream in(filename);
    if (!in) return false;

    // очищаем старый список
    while (head != nullptr){
        popFrontF();
    }

    std::string val;
    while (in >> val){
        pushBackF(val);
    }
    return true;


}

bool ListF::saveListFBinary(const char* filename) const {
    if (filename == nullptr) {
        return false;
    }

    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        return false;
    }

    // Сигнатура формата файла
    const char magic[8] = {'L', 'I', 'S', 'T', 'F', '0', '1', '\0'};
    out.write(magic, sizeof(magic));

    const std::uint64_t count =
        static_cast<std::uint64_t>(sizeF());

    out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    // Для каждой строки записываем её длину и содержимое.
    // Благодаря этому строки с пробелами сохраняются корректно.
    for (NodeF* current = head; current != nullptr; current = current->next) {
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


bool ListF::loadListFBinary(const char* filename) {
    if (filename == nullptr) {
        return false;
    }

    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        return false;
    }

    const char expectedMagic[8] = {'L', 'I', 'S', 'T', 'F', '0', '1', '\0'};
    char actualMagic[8]{};

    in.read(actualMagic, sizeof(actualMagic));

    if (!in ||
        std::memcmp(actualMagic, expectedMagic, sizeof(expectedMagic)) != 0) {
        return false;
    }

    std::uint64_t count = 0;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));

    // Защита от повреждённого файла с неправдоподобным числом элементов.
    if (!in || count > 1'000'000) {
        return false;
    }

    // Сначала загружаем данные во временный список.
    // Если чтение файла завершится ошибкой, текущий список не изменится.
    ListF temporaryList;

    for (std::uint64_t i = 0; i < count; ++i) {
        std::uint64_t length = 0;

        in.read(reinterpret_cast<char*>(&length), sizeof(length));

        // Ограничиваем длину одной строки 16 МБ.
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

        temporaryList.pushBackF(value);
    }

    // Меняем текущий список местами с временным.
    // При выходе из функции temporaryList уничтожит старые узлы.
    NodeF* oldHead = head;
    head = temporaryList.head;
    temporaryList.head = oldHead;

    NodeF* oldTail = tail;
    tail = temporaryList.tail;
    temporaryList.tail = oldTail;

    return true;
}
