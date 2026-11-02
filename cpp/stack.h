#ifndef STACK_H
#define STACK_H

#include <string>
#include <fstream>

//Структура узла стека
struct NodeS {
    std::string data;
    NodeS* next;
};

class Stack {
private:
    NodeS* head;

    //функция для сохранения в обратном порядке
    static void saveStackHelper(NodeS* node, std::ofstream& out);

public:
    Stack();
    ~Stack();

    //запрет копирования - защита от двойного delete
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    //проверка
    bool isStackEmpty() const;

    //основные операции
    void pushStack(const std::string& value);
    bool popStack(std::string& outValue);
    bool peekStack(std::string& value) const; //вершина без удаления

    //вывод
    void printStack() const;

    //сериализация
    bool saveStackToFile(const char* filename) const;
    bool loadStackFromFile(const char* filename);

    bool saveStackBinary(const char* filename) const;
    bool loadStackBinary(const char* filename);

};

#endif //STACK_H