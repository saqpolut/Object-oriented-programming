#ifndef QUEUE_H
#define QUEUE_H

#include <string>

//Структура узла очереди
struct NodeQ {
    std::string data;
    NodeQ* next;
    NodeQ(const std::string& d) { data = d; next = nullptr; }
};

class Queue {
private:
    NodeQ* head;  //голова: откуда извлекаем (pop)
    NodeQ* tail;  //хвост: куда добавляем (push)
    int count;    //количество элементов для быстрого получения размера

public:
    Queue();
    ~Queue();

    //запрет копирования — защита от двойного delete
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    //проверка и размер
    bool isQueueEmpty() const;
    int sizeQueue() const;

    //основные операции
    void pushQueue(const std::string& value); //добавить в хвост
    bool popQueue(std::string& outValue); //извлечь из головы
    bool peekQueue(std::string& outValue) const; //посмотреть первый, не извлекая

    //вывод
    void printQueue() const;

    //сериализация
    bool saveQueueToFile(const char* filename) const;
    bool loadQueueFromFile(const char* filename);

    bool saveQueueBinary(const char* filename) const;
    bool loadQueueBinary(const char* filename);

};

#endif // QUEUE_H