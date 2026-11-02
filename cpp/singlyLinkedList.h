#ifndef SINGLYLINKEDLIST_H
#define SINGLYLINKEDLIST_H

#include <string>

// Структура узла односвязного списка
struct NodeF {
    std::string data;
    NodeF* next;
    NodeF(const std::string& d) { data = d; next = nullptr; }
};

class ListF {
private:
    NodeF* head;
    NodeF* tail;

    //функция для обратного вывода
    static void printBackwardRecF(NodeF* node);

public:
    ListF();
    ~ListF();

    //запрет копирования (чтобы не было двойного delete)
    ListF(const ListF&) = delete;
    ListF& operator=(const ListF&) = delete;

    //проверка и размер
    bool isEmptyF() const;
    int sizeF() const;

    //поиск
    NodeF* findNodeF(const std::string& value) const;
    NodeF* getAtF(int index) const;

    //добавление (4 способа)
    void pushFrontF(const std::string& value);
    void pushBackF(const std::string& value);
    bool insertAfterF(const std::string& target, const std::string& value);
    bool insertBeforeF(const std::string& target, const std::string& value);

    //удаление (4 способа + по значению)
    bool popFrontF();
    bool popBackF();
    bool removeAfterF(const std::string& target);
    bool removeBeforeF(const std::string& target);
    bool removeByValueF(const std::string& value);

    //поиск по значению (возвращает индекс или -1)
    int searchF(const std::string& value) const;

    //вывод (3 способа)
    void printForwardF() const;
    void printBackwardF() const;
    void printIndexedF() const;

    // сериализация
    bool saveListFToFile(const char* filename) const;
    bool loadListFFromFile(const char* filename);

    bool saveListFBinary(const char* filename) const;
    bool loadListFBinary(const char* filename);

};

#endif // SINGLYLINKEDLIST_H