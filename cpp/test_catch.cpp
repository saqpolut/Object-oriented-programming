#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "massiv.h"
#include "singlyLinkedList.h"
#include "doublyLinkedList.h"
#include "queue.h"
#include "stack.h"
#include "AVL_Tree.h"
#include "hashTable.h"

// Временно перенаправляет std::cin для тестирования readMassiv().
class CinRedirect {
public:
    explicit CinRedirect(std::istringstream& input)
        : oldBuffer_(std::cin.rdbuf(input.rdbuf())) {
        std::cin.clear();
    }

    ~CinRedirect() {
        std::cin.rdbuf(oldBuffer_);
        std::cin.clear();
    }

    CinRedirect(const CinRedirect&) = delete;
    CinRedirect& operator=(const CinRedirect&) = delete;

private:
    std::streambuf* oldBuffer_;
};

// ============================================================
// Массив Massiv
// ============================================================

TEST_CASE("Massiv: вставка, получение, замена и удаление", "[massiv]") {
    Massiv array;
    std::string value;

    CHECK(array.isEmpty());
    CHECK(array.getLength() == 0);
    CHECK_FALSE(array.getElement(0, value));
    CHECK_FALSE(array.removeAtIndex(0));
    CHECK_FALSE(array.replaceAtIndex(0, "x"));
    CHECK_FALSE(array.addAtIndex(1, "x"));

    REQUIRE(array.addToEnd("a"));
    REQUIRE(array.addToEnd("b"));
    REQUIRE(array.addAtIndex(1, "middle"));
    REQUIRE(array.getLength() == 3);

    REQUIRE(array.getElement(0, value));
    CHECK(value == "a");
    REQUIRE(array.getElement(1, value));
    CHECK(value == "middle");
    REQUIRE(array.getElement(2, value));
    CHECK(value == "b");

    CHECK(array.replaceAtIndex(0, "changed"));
    CHECK_FALSE(array.replaceAtIndex(10, "bad"));
    CHECK_FALSE(array.getElement(10, value));

    CHECK(array.removeAtIndex(1));
    CHECK_FALSE(array.removeAtIndex(10));
    CHECK(array.getLength() == 2);

    CHECK(array.removeAtIndex(1));
    CHECK(array.removeAtIndex(0));
    CHECK(array.isEmpty());

    CHECK(array.addAtIndex(0, "first"));
    REQUIRE(array.getElement(0, value));
    CHECK(value == "first");
}

TEST_CASE("Massiv: копирование и присваивание", "[massiv]") {
    Massiv source;
    REQUIRE(source.addToEnd("one"));
    REQUIRE(source.addToEnd("two"));

    Massiv copy(source);
    std::string value;

    REQUIRE(copy.getElement(0, value));
    CHECK(value == "one");
    REQUIRE(copy.getElement(1, value));
    CHECK(value == "two");

    Massiv assigned;
    assigned = source;

    REQUIRE(assigned.getElement(0, value));
    CHECK(value == "one");

    assigned = assigned;
    REQUIRE(assigned.getElement(1, value));
    CHECK(value == "two");
}

TEST_CASE("Massiv: чтение корректного и некорректного ввода", "[massiv]") {
    Massiv array;
    std::string value;

    SECTION("корректные данные") {
        std::istringstream input("3 alpha beta gamma");
        CinRedirect redirect(input);

        REQUIRE(array.readMassiv());
        REQUIRE(array.getLength() == 3);
        REQUIRE(array.getElement(0, value));
        CHECK(value == "alpha");
        REQUIRE(array.getElement(2, value));
        CHECK(value == "gamma");
    }

    SECTION("некорректное количество") {
        std::istringstream input("not_a_number");
        CinRedirect redirect(input);
        CHECK_FALSE(array.readMassiv());
    }

    SECTION("не хватает элементов") {
        std::istringstream input("3 only_one");
        CinRedirect redirect(input);
        CHECK_FALSE(array.readMassiv());
    }

    SECTION("пустой массив") {
        std::istringstream input("0");
        CinRedirect redirect(input);
        CHECK(array.readMassiv());
        CHECK(array.isEmpty());
    }
}

TEST_CASE("Massiv: бинарная сериализация и ошибки файлов", "[massiv]") {
    const char* filename = "massiv_catch_test.bin";

    Massiv original;
    REQUIRE(original.addToEnd("alpha"));
    REQUIRE(original.addToEnd("beta"));
    REQUIRE(original.saveBinary(filename));

    Massiv loaded;
    REQUIRE(loaded.loadBinary(filename));
    REQUIRE(loaded.getLength() == 2);

    std::string value;
    REQUIRE(loaded.getElement(0, value));
    CHECK(value == "alpha");
    REQUIRE(loaded.getElement(1, value));
    CHECK(value == "beta");

    CHECK_FALSE(loaded.loadBinary("missing_massiv_catch.bin"));
    CHECK_FALSE(original.saveBinary(
        "/directory_that_does_not_exist_12345/massiv.bin"));

    std::remove(filename);
}

TEST_CASE("Massiv: бинарная сериализация пустой строки и усечённого файла",
          "[massiv]") {
    const char* filename = "massiv_catch_empty.bin";

    Massiv original;
    REQUIRE(original.addToEnd(""));
    REQUIRE(original.addToEnd("data"));
    REQUIRE(original.saveBinary(filename));

    Massiv loaded;
    REQUIRE(loaded.loadBinary(filename));

    std::string value;
    REQUIRE(loaded.getElement(0, value));
    CHECK(value.empty());
    REQUIRE(loaded.getElement(1, value));
    CHECK(value == "data");

    std::remove(filename);

    const char* truncated = "massiv_catch_truncated.bin";
    {
        std::ofstream file(truncated, std::ios::binary);
        // Файл без поля количества элементов.
    }

    CHECK_FALSE(loaded.loadBinary(truncated));
    std::remove(truncated);
}

// ============================================================
// Односвязный список ListF
// ============================================================

TEST_CASE("ListF: операции и граничные случаи", "[listf]") {
    ListF list;

    CHECK(list.isEmptyF());
    CHECK(list.sizeF() == 0);
    CHECK(list.searchF("missing") == -1);
    CHECK(list.getAtF(-1) == nullptr);
    CHECK(list.getAtF(0) == nullptr);
    CHECK(list.findNodeF("missing") == nullptr);

    CHECK_FALSE(list.popFrontF());
    CHECK_FALSE(list.popBackF());
    CHECK_FALSE(list.removeAfterF("missing"));
    CHECK_FALSE(list.removeBeforeF("missing"));
    CHECK_FALSE(list.removeByValueF("missing"));

    list.pushFrontF("b");
    list.pushFrontF("a");
    list.pushBackF("d");

    REQUIRE(list.sizeF() == 3);
    CHECK(list.searchF("a") == 0);
    CHECK(list.searchF("d") == 2);
    CHECK(list.searchF("missing") == -1);

    REQUIRE(list.getAtF(1) != nullptr);
    CHECK(list.getAtF(1)->data == "b");
    CHECK(list.getAtF(3) == nullptr);
    CHECK(list.findNodeF("d") != nullptr);

    CHECK(list.insertAfterF("b", "c"));
    CHECK(list.insertBeforeF("a", "first"));
    CHECK_FALSE(list.insertAfterF("missing", "x"));
    CHECK_FALSE(list.insertBeforeF("missing", "x"));

    CHECK(list.removeBeforeF("a"));
    CHECK(list.removeBeforeF("c"));
    CHECK_FALSE(list.removeBeforeF("a"));

    CHECK(list.removeAfterF("a"));
    CHECK_FALSE(list.removeAfterF("d"));

    CHECK(list.removeByValueF("a"));
    CHECK_FALSE(list.removeByValueF("missing"));

    while (list.popFrontF()) {
    }
    CHECK(list.isEmptyF());
    CHECK_FALSE(list.popBackF());
}

TEST_CASE("ListF: текстовая и бинарная сериализация", "[listf]") {
    const char* textFile = "listf_catch_test.txt";
    const char* binaryFile = "listf_catch_test.bin";

    ListF original;
    original.pushBackF("first");
    original.pushBackF("second");

    REQUIRE(original.saveListFToFile(textFile));
    REQUIRE(original.saveListFBinary(binaryFile));

    ListF fromText;
    REQUIRE(fromText.loadListFFromFile(textFile));
    REQUIRE(fromText.sizeF() == 2);
    CHECK(fromText.getAtF(0)->data == "first");
    CHECK(fromText.getAtF(1)->data == "second");

    ListF fromBinary;
    REQUIRE(fromBinary.loadListFBinary(binaryFile));
    REQUIRE(fromBinary.sizeF() == 2);
    CHECK(fromBinary.getAtF(0)->data == "first");
    CHECK(fromBinary.getAtF(1)->data == "second");

    CHECK_FALSE(fromText.loadListFFromFile("missing_listf_catch.txt"));
    CHECK_FALSE(fromBinary.loadListFBinary("missing_listf_catch.bin"));
    CHECK_FALSE(fromBinary.saveListFBinary(nullptr));
    CHECK_FALSE(fromBinary.loadListFBinary(nullptr));

    std::remove(textFile);
    std::remove(binaryFile);
}

// ============================================================
// Двусвязный список ListD
// ============================================================

TEST_CASE("ListD: операции и граничные случаи", "[listd]") {
    ListD list;

    CHECK(list.isEmptyD());
    CHECK(list.sizeD() == 0);
    CHECK(list.searchD("missing") == -1);
    CHECK(list.getAtD(-1) == nullptr);
    CHECK(list.getAtD(0) == nullptr);
    CHECK(list.findNodeD("missing") == nullptr);

    CHECK_FALSE(list.popFrontD());
    CHECK_FALSE(list.popBackD());
    CHECK_FALSE(list.removeAfterD("missing"));
    CHECK_FALSE(list.removeBeforeD("missing"));
    CHECK_FALSE(list.removeByValueD("missing"));

    list.pushFrontD("b");
    list.pushFrontD("a");
    list.pushBackD("d");

    REQUIRE(list.sizeD() == 3);
    CHECK(list.searchD("a") == 0);
    CHECK(list.searchD("d") == 2);
    CHECK(list.searchD("missing") == -1);

    REQUIRE(list.getAtD(1) != nullptr);
    CHECK(list.getAtD(1)->data == "b");
    CHECK(list.getAtD(3) == nullptr);
    CHECK(list.findNodeD("d") != nullptr);

    CHECK(list.insertAfterD("b", "c"));
    CHECK(list.insertBeforeD("a", "first"));
    CHECK_FALSE(list.insertAfterD("missing", "x"));
    CHECK_FALSE(list.insertBeforeD("missing", "x"));

    CHECK(list.removeBeforeD("a"));
    CHECK(list.removeBeforeD("c"));
    CHECK_FALSE(list.removeBeforeD("first"));

    CHECK(list.removeAfterD("a"));
    CHECK_FALSE(list.removeAfterD("d"));

    CHECK(list.removeByValueD("d"));
    CHECK_FALSE(list.removeByValueD("missing"));

    while (list.popFrontD()) {
    }
    CHECK(list.isEmptyD());
}

TEST_CASE("ListD: текстовая и бинарная сериализация", "[listd]") {
    const char* textFile = "listd_catch_test.txt";
    const char* binaryFile = "listd_catch_test.bin";

    ListD original;
    original.pushBackD("first");
    original.pushBackD("second");

    REQUIRE(original.saveListDToFile(textFile));
    REQUIRE(original.saveListDBinary(binaryFile));

    ListD fromText;
    REQUIRE(fromText.loadListDFromFile(textFile));
    REQUIRE(fromText.sizeD() == 2);
    CHECK(fromText.getAtD(0)->data == "first");
    CHECK(fromText.getAtD(1)->data == "second");

    ListD fromBinary;
    REQUIRE(fromBinary.loadListDBinary(binaryFile));
    REQUIRE(fromBinary.sizeD() == 2);
    CHECK(fromBinary.getAtD(0)->data == "first");
    CHECK(fromBinary.getAtD(1)->data == "second");

    CHECK_FALSE(fromText.loadListDFromFile("missing_listd_catch.txt"));
    CHECK_FALSE(fromBinary.loadListDBinary("missing_listd_catch.bin"));
    CHECK_FALSE(fromBinary.saveListDBinary(nullptr));
    CHECK_FALSE(fromBinary.loadListDBinary(nullptr));

    std::remove(textFile);
    std::remove(binaryFile);
}

// ============================================================
// Очередь Queue
// ============================================================

TEST_CASE("Queue: основные операции и текстовая сериализация", "[queue]") {
    const char* filename = "queue_catch_test.txt";

    Queue queue;
    std::string value;

    CHECK(queue.isQueueEmpty());
    CHECK(queue.sizeQueue() == 0);
    CHECK_FALSE(queue.peekQueue(value));
    CHECK_FALSE(queue.popQueue(value));

    queue.pushQueue("first");
    queue.pushQueue("second");

    CHECK_FALSE(queue.isQueueEmpty());
    CHECK(queue.sizeQueue() == 2);

    REQUIRE(queue.peekQueue(value));
    CHECK(value == "first");

    REQUIRE(queue.saveQueueToFile(filename));

    Queue loaded;
    REQUIRE(loaded.loadQueueFromFile(filename));
    CHECK(loaded.sizeQueue() == 2);

    REQUIRE(loaded.popQueue(value));
    CHECK(value == "first");
    REQUIRE(loaded.popQueue(value));
    CHECK(value == "second");
    CHECK(loaded.isQueueEmpty());
    CHECK_FALSE(loaded.popQueue(value));
    CHECK_FALSE(loaded.loadQueueFromFile("missing_queue_catch.txt"));

    std::remove(filename);
}

TEST_CASE("Queue: бинарная сериализация и некорректный файл", "[queue]") {
    const char* filename = "queue_catch_test.bin";

    Queue original;
    original.pushQueue("first");
    original.pushQueue("second");
    REQUIRE(original.saveQueueBinary(filename));

    Queue loaded;
    REQUIRE(loaded.loadQueueBinary(filename));
    REQUIRE(loaded.sizeQueue() == 2);

    std::string value;
    REQUIRE(loaded.popQueue(value));
    CHECK(value == "first");
    REQUIRE(loaded.popQueue(value));
    CHECK(value == "second");
    CHECK(loaded.isQueueEmpty());

    CHECK_FALSE(loaded.saveQueueBinary(nullptr));
    CHECK_FALSE(loaded.loadQueueBinary(nullptr));
    CHECK_FALSE(loaded.loadQueueBinary("missing_queue_catch.bin"));

    const char* badFile = "queue_catch_bad_magic.bin";
    {
        std::ofstream file(badFile, std::ios::binary);
        file.write("BADMAGIC", 8);
    }
    CHECK_FALSE(loaded.loadQueueBinary(badFile));

    std::remove(filename);
    std::remove(badFile);
}

// ============================================================
// Стек Stack
// ============================================================

TEST_CASE("Stack: основные операции и текстовая сериализация", "[stack]") {
    const char* filename = "stack_catch_test.txt";

    Stack stack;
    std::string value;

    CHECK(stack.isStackEmpty());
    CHECK_FALSE(stack.peekStack(value));
    CHECK_FALSE(stack.popStack(value));

    stack.pushStack("bottom");
    stack.pushStack("top");

    REQUIRE(stack.peekStack(value));
    CHECK(value == "top");
    REQUIRE(stack.saveStackToFile(filename));

    Stack loaded;
    REQUIRE(loaded.loadStackFromFile(filename));

    REQUIRE(loaded.popStack(value));
    CHECK(value == "top");
    REQUIRE(loaded.popStack(value));
    CHECK(value == "bottom");
    CHECK(loaded.isStackEmpty());
    CHECK_FALSE(loaded.popStack(value));
    CHECK_FALSE(loaded.loadStackFromFile("missing_stack_catch.txt"));

    std::remove(filename);
}

TEST_CASE("Stack: бинарная сериализация и некорректный файл", "[stack]") {
    const char* filename = "stack_catch_test.bin";

    Stack original;
    original.pushStack("bottom");
    original.pushStack("top");
    REQUIRE(original.saveStackBinary(filename));

    Stack loaded;
    REQUIRE(loaded.loadStackBinary(filename));

    std::string value;
    REQUIRE(loaded.popStack(value));
    CHECK(value == "top");
    REQUIRE(loaded.popStack(value));
    CHECK(value == "bottom");
    CHECK(loaded.isStackEmpty());

    CHECK_FALSE(loaded.saveStackBinary(nullptr));
    CHECK_FALSE(loaded.loadStackBinary(nullptr));
    CHECK_FALSE(loaded.loadStackBinary("missing_stack_catch.bin"));

    const char* badFile = "stack_catch_bad_magic.bin";
    {
        std::ofstream file(badFile, std::ios::binary);
        file.write("BADMAGIC", 8);
    }
    CHECK_FALSE(loaded.loadStackBinary(badFile));

    std::remove(filename);
    std::remove(badFile);
}

// ============================================================
// AVL-дерево
// ============================================================

TEST_CASE("AVLTree: четыре типа поворота при вставке", "[avl]") {
    SECTION("LL") {
        AVLTree tree;
        tree.insertT(30);
        tree.insertT(20);
        tree.insertT(10);

        REQUIRE(tree.findT(20) != nullptr);
        CHECK(tree.findT(20)->key == 20);
        CHECK(tree.getHeightT() == 2);
    }

    SECTION("LR") {
        AVLTree tree;
        tree.insertT(30);
        tree.insertT(10);
        tree.insertT(20);

        REQUIRE(tree.findT(20) != nullptr);
        CHECK(tree.getHeightT() == 2);
    }

    SECTION("RR") {
        AVLTree tree;
        tree.insertT(10);
        tree.insertT(20);
        tree.insertT(30);

        REQUIRE(tree.findT(20) != nullptr);
        CHECK(tree.getHeightT() == 2);
    }

    SECTION("RL") {
        AVLTree tree;
        tree.insertT(10);
        tree.insertT(30);
        tree.insertT(20);

        REQUIRE(tree.findT(20) != nullptr);
        CHECK(tree.getHeightT() == 2);
    }
}

TEST_CASE("AVLTree: поиск, удаление и текстовая сериализация", "[avl]") {
    const char* filename = "avl_catch_test.txt";

    AVLTree tree;
    CHECK(tree.isEmpty());
    CHECK(tree.countNodesT() == 0);
    CHECK(tree.getHeightT() == 0);
    CHECK(tree.findT(10) == nullptr);
    CHECK(tree.findMinT() == nullptr);

    REQUIRE(tree.saveTreeToFile(filename));
    REQUIRE(tree.loadTreeFromFile(filename));
    CHECK_FALSE(tree.loadTreeFromFile("missing_avl_catch.txt"));

    for (int value : {40, 20, 60, 10, 30, 50, 70, 25, 35}) {
        tree.insertT(value);
    }

    tree.insertT(30);
    CHECK(tree.countNodesT() == 9);
    CHECK_FALSE(tree.isEmpty());

    REQUIRE(tree.findT(30) != nullptr);
    CHECK(tree.findT(30)->key == 30);
    CHECK(tree.findT(999) == nullptr);

    REQUIRE(tree.findMinT() != nullptr);
    CHECK(tree.findMinT()->key == 10);

    REQUIRE(tree.saveTreeToFile(filename));

    AVLTree loaded;
    REQUIRE(loaded.loadTreeFromFile(filename));
    CHECK(loaded.countNodesT() == tree.countNodesT());
    CHECK(loaded.findT(25) != nullptr);

    tree.removeT(999);
    tree.removeT(10);
    tree.removeT(20);
    tree.removeT(40);

    CHECK(tree.findT(10) == nullptr);
    CHECK(tree.findT(20) == nullptr);
    CHECK(tree.findT(40) == nullptr);

    for (int value : {25, 30, 35, 50, 60, 70}) {
        tree.removeT(value);
    }

    CHECK(tree.isEmpty());
    CHECK(tree.countNodesT() == 0);

    std::remove(filename);
}

TEST_CASE("AVLTree: удаление узла с одним потомком и неверный токен",
          "[avl]") {
    AVLTree tree;
    tree.insertT(20);
    tree.insertT(10);
    tree.removeT(20);

    CHECK(tree.findT(20) == nullptr);
    REQUIRE(tree.findT(10) != nullptr);
    CHECK(tree.countNodesT() == 1);

    const char* filename = "avl_catch_invalid.txt";
    {
        std::ofstream file(filename);
        file << "not_an_integer";
    }

    AVLTree invalid;
    CHECK(invalid.loadTreeFromFile(filename));
    CHECK(invalid.isEmpty());

    std::remove(filename);
}

// ============================================================
// Хеш-таблица HashTable
// ============================================================

TEST_CASE("HashTable: вставка, поиск, обновление и удаление", "[hash]") {
    HashTable table(1, LINEAR_PROBING);
    int value = -1;

    CHECK(table.isEmpty());
    CHECK_FALSE(table.hashFindInt("missing", value));
    CHECK_FALSE(table.hashErase("missing"));

    REQUIRE(table.hashInsertInt("key", 10));
    CHECK_FALSE(table.isEmpty());
    CHECK(table.getSize() == 1);

    REQUIRE(table.hashFindInt("key", value));
    CHECK(value == 10);

    REQUIRE(table.hashInsertInt("key", 20));
    CHECK(table.getSize() == 1);
    REQUIRE(table.hashFindInt("key", value));
    CHECK(value == 20);

    CHECK(table.hashErase("key"));
    CHECK(table.isEmpty());
    CHECK_FALSE(table.hashFindInt("key", value));
    CHECK_FALSE(table.hashErase("key"));
}

TEST_CASE("HashTable: коллизии, удалённые ячейки и переполнение", "[hash]") {
    HashTable table(1, LINEAR_PROBING);
    std::vector<std::string> keys;

    for (int i = 0; i < 13; ++i) {
        keys.push_back("key_" + std::to_string(i));
        REQUIRE(table.hashInsertInt(keys.back(), i));
    }

    CHECK(table.getSize() == 13);
    CHECK(table.getCollisions() > 0);

    int value = -1;
    CHECK_FALSE(table.hashFindInt("not_present", value));
    CHECK_FALSE(table.hashInsertInt("overflow", 100));
    CHECK_FALSE(table.hashErase("not_present"));

    REQUIRE(table.hashErase(keys[0]));
    REQUIRE(table.hashInsertInt("replacement", 200));
    REQUIRE(table.hashFindInt("replacement", value));
    CHECK(value == 200);
}

TEST_CASE("HashTable: двойное хеширование и значения-указатели", "[hash]") {
    HashTable table(2, DOUBLE_HASHING);

    REQUIRE(table.hashInsertInt("first", 1));
    REQUIRE(table.hashInsertInt("second", 2));

    int value = -1;
    REQUIRE(table.hashFindInt("first", value));
    CHECK(value == 1);

    int stored = 42;
    REQUIRE(table.hashInsert("pointer", &stored));

    void* result = nullptr;
    REQUIRE(table.hashFind("pointer", result));
    REQUIRE(result != nullptr);
    CHECK(*static_cast<int*>(result) == 42);
}

TEST_CASE("HashTable: бинарная сериализация целочисленных значений", "[hash]") {
    const std::string filename = "hash_catch_test.bin";

    HashTable original(4, DOUBLE_HASHING);
    REQUIRE(original.hashInsertInt("first", 123));
    REQUIRE(original.hashInsertInt("key with space", -7));
    REQUIRE(original.hashInsertInt("zero", 0));
    REQUIRE(original.saveBinary(filename));

    HashTable loaded(1, LINEAR_PROBING);
    REQUIRE(loaded.loadBinary(filename));

    int value = 0;
    REQUIRE(loaded.hashFindInt("first", value));
    CHECK(value == 123);

    REQUIRE(loaded.hashFindInt("key with space", value));
    CHECK(value == -7);

    REQUIRE(loaded.hashFindInt("zero", value));
    CHECK(value == 0);

    CHECK_FALSE(loaded.hashFindInt("missing", value));

    std::remove(filename.c_str());
}

// ============================================================
// Ошибки открытия файлов
// ============================================================

TEST_CASE("Структуры возвращают ошибку для недоступного пути", "[files]") {
    const char* badPath =
        "/directory_that_does_not_exist_12345/test.txt";

    ListF listF;
    CHECK_FALSE(listF.saveListFToFile(badPath));

    ListD listD;
    CHECK_FALSE(listD.saveListDToFile(badPath));

    Queue queue;
    CHECK_FALSE(queue.saveQueueToFile(badPath));

    Stack stack;
    CHECK_FALSE(stack.saveStackToFile(badPath));

    AVLTree tree;
    CHECK_FALSE(tree.saveTreeToFile(badPath));
}
