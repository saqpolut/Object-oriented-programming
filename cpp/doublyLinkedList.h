#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

#include <string>

// Структура узла двусвязного списка
struct NodeD {
    std::string data;
    NodeD* prev;
    NodeD* next;

    NodeD(const std::string& d) { data = d; prev = nullptr; next = nullptr; }
};

class ListD {
private:
    NodeD* head;
    NodeD* tail;

public:
    ListD();
    ~ListD();

    // запрет копирования - защитa от двойного delete
    ListD(const ListD&) = delete;
    ListD& operator=(const ListD&) = delete;

    // проверка и размер
    bool isEmptyD() const;
    int sizeD() const;

    // поиск
    NodeD* findNodeD(const std::string& value) const;
    NodeD* getAtD(int index) const;

    // добавление (4 способа)
    void pushFrontD(const std::string& value);
    void pushBackD(const std::string& value);
    bool insertAfterD(const std::string& target, const std::string& value);
    bool insertBeforeD(const std::string& target, const std::string& value);

    // удаление (4 способа + по значению)
    bool popFrontD();
    bool popBackD();
    bool removeAfterD(const std::string& target);
    bool removeBeforeD(const std::string& target);
    bool removeByValueD(const std::string& value);

    // поиск индекса
    int searchD(const std::string& value) const;

    // вывод (3 способа)
    void printForwardD() const;
    void printBackwardD() const;
    void printIndexedD() const;

    // сериализация
    bool saveListDToFile(const char* filename) const;
    bool loadListDFromFile(const char* filename);

    bool saveListDBinary(const char* filename) const;
    bool loadListDBinary(const char* filename);

};

#endif //DOUBLYLINKEDLIST_H