#include <gtest/gtest.h>

#include <cstdint>
#include <cstdio>
#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

#include "massiv.h"
#include "singlyLinkedList.h"
#include "doublyLinkedList.h"
#include "queue.h"
#include "stack.h"
#include "AVL_Tree.h"
#include "hashTable.h"

// ------------------------------------------------------------
// 1. МАССИВ (Massiv)
// ------------------------------------------------------------

//вспомогательная структура
//проверить, что при вызове метода с меньшим размером
//память не пререраспределится и данные не теряются
struct MassivTestAccess {
    static bool reserve(Massiv& massiv, size_t capacity) {
        return massiv.reserve(capacity);
    }
};

//Проверка расмерности
//если ёмкость 4, то reserve(2) не уменьшить её
TEST(MassivTest, ReserveDoesNotShrinkCapacity) {
    Massiv massiv(4);

    ASSERT_TRUE(massiv.addToEnd("first"));
    ASSERT_TRUE(massiv.addToEnd("second"));

    EXPECT_TRUE(MassivTestAccess::reserve(massiv, 4));
    EXPECT_TRUE(MassivTestAccess::reserve(massiv, 2));

    std::string value;
    ASSERT_TRUE(massiv.getElement(0, value));
    EXPECT_EQ(value, "first");
    ASSERT_TRUE(massiv.getElement(1, value));
    EXPECT_EQ(value, "second");
}

//проверка логики:
TEST(MassivTest, OperationsAndCopying) {
    Massiv array;
    std::string value;

    //пустой масси (нельзя получить/удалить элемент)
    EXPECT_TRUE(array.isEmpty());
    EXPECT_EQ(array.getLength(), 0u); //u - unsigned
    EXPECT_FALSE(array.getElement(0, value));
    EXPECT_FALSE(array.removeAtIndex(0));
    EXPECT_FALSE(array.replaceAtIndex(0, "x"));
    //попытка вставить 1 в пустой массив
    EXPECT_FALSE(array.addAtIndex(1, "x"));

    //добавление в конец, вставка по индексу
    EXPECT_TRUE(array.addToEnd("a"));
    EXPECT_TRUE(array.addToEnd("b"));
    EXPECT_TRUE(array.addAtIndex(1, "middle"));
    EXPECT_EQ(array.getLength(), 3u);

    ASSERT_TRUE(array.getElement(0, value));
    EXPECT_EQ(value, "a");
    ASSERT_TRUE(array.getElement(1, value));
    EXPECT_EQ(value, "middle");
    ASSERT_TRUE(array.getElement(2, value));
    EXPECT_EQ(value, "b");

    EXPECT_TRUE(array.replaceAtIndex(0, "changed"));
    EXPECT_FALSE(array.replaceAtIndex(10, "bad"));
    EXPECT_FALSE(array.getElement(10, value));

    //копирование и присваивание 
    Massiv copy(array);
    ASSERT_TRUE(copy.getElement(0, value));
    EXPECT_EQ(value, "changed");

    Massiv assigned;
    assigned = copy;
    ASSERT_TRUE(assigned.getElement(1, value));
    EXPECT_EQ(value, "middle");

    assigned = assigned;
    EXPECT_EQ(assigned.getLength(), 3u);

    EXPECT_TRUE(array.removeAtIndex(1));
    EXPECT_FALSE(array.removeAtIndex(10));
    EXPECT_EQ(array.getLength(), 2u);

    EXPECT_TRUE(array.removeAtIndex(1));
    EXPECT_TRUE(array.removeAtIndex(0));
    EXPECT_TRUE(array.isEmpty());

    EXPECT_TRUE(array.addAtIndex(0, "first"));
    ASSERT_TRUE(array.getElement(0, value));
    EXPECT_EQ(value, "first");
}

//проверяет копирование аустого массива
TEST(MassivTest, CopyConstructorAndAssignment) {
    Massiv source(3);
    ASSERT_TRUE(source.addToEnd("one"));
    ASSERT_TRUE(source.addToEnd("two"));

    Massiv copy(source);

    std::string value;
    ASSERT_TRUE(copy.getElement(0, value));
    EXPECT_EQ(value, "one");
    ASSERT_TRUE(copy.getElement(1, value));
    EXPECT_EQ(value, "two");

    Massiv empty(0);
    Massiv emptyCopy(empty);
    EXPECT_TRUE(emptyCopy.isEmpty());

    Massiv assigned(0);
    assigned = source;

    ASSERT_TRUE(assigned.getElement(0, value));
    EXPECT_EQ(value, "one");
    ASSERT_TRUE(assigned.getElement(1, value));
    EXPECT_EQ(value, "two");

    assigned = assigned;
    ASSERT_TRUE(assigned.getElement(0, value));
    EXPECT_EQ(value, "one");
}

//сохраняет массив в файл, загружает обратно, проверяет - совпали ли данные
TEST(MassivTest, BinarySerialization) {
    const char* filename = "massiv_test.bin";

    //записывает строки в файл
    Massiv original;
    EXPECT_TRUE(original.addToEnd("alpha"));
    EXPECT_TRUE(original.addToEnd("beta"));

    ASSERT_TRUE(original.saveBinary(filename));

    //читает их обратно в новый файл
    Massiv loaded;
    ASSERT_TRUE(loaded.loadBinary(filename));
    ASSERT_EQ(loaded.getLength(), 2u);

    //проверяет что данные не исказились
    std::string value;
    ASSERT_TRUE(loaded.getElement(0, value));
    EXPECT_EQ(value, "alpha");
    ASSERT_TRUE(loaded.getElement(1, value));
    EXPECT_EQ(value, "beta");

    //попытка загрузить несуществующий файл
    EXPECT_FALSE(loaded.loadBinary("file_that_does_not_exist.bin"));
    std::remove(filename);
}

TEST(MassivTest, InsertIntoEmptyArray) {
    Massiv array(0);
    std::string value;

    EXPECT_TRUE(array.isEmpty());
    ASSERT_TRUE(array.addAtIndex(0, "first"));

    EXPECT_EQ(array.getLength(), 1u);
    ASSERT_TRUE(array.getElement(0, value));
    EXPECT_EQ(value, "first");
}

//проверка корректного ввода
TEST(MassivTest, ReadInputAndFileErrors) {
    Massiv array;
    std::string value;

    {
        std::istringstream input("3 alpha beta gamma"); //симуляция ввода пользователя
        std::streambuf* oldCin = std::cin.rdbuf(input.rdbuf());
        const bool result = array.readMassiv();
        std::cin.rdbuf(oldCin);

        ASSERT_TRUE(result);
        ASSERT_EQ(array.getLength(), 3u);
        ASSERT_TRUE(array.getElement(0, value));
        EXPECT_EQ(value, "alpha");
        ASSERT_TRUE(array.getElement(2, value));
        EXPECT_EQ(value, "gamma");
    }

    {
        std::istringstream input("not_a_number");
        std::streambuf* oldCin = std::cin.rdbuf(input.rdbuf());
        const bool result = array.readMassiv();
        std::cin.rdbuf(oldCin);
        EXPECT_FALSE(result);
    }

    {
        std::istringstream input("2 only_one_value");
        std::streambuf* oldCin = std::cin.rdbuf(input.rdbuf());
        const bool result = array.readMassiv();
        std::cin.rdbuf(oldCin);
        EXPECT_FALSE(result);
    }

    EXPECT_FALSE(array.saveBinary("missing_directory/file.bin"));

    {
        std::ofstream out("truncated_count.bin", std::ios::binary);
        const size_t count = 1;
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    }

    Massiv loaded;
    EXPECT_FALSE(loaded.loadBinary("truncated_count.bin"));
    std::remove("truncated_count.bin");

    {
        std::ofstream out("truncated_string.bin", std::ios::binary);
        const size_t count = 1;
        const size_t length = 5;
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        out.write(reinterpret_cast<const char*>(&length), sizeof(length));
        out.write("ab", 2);
    }

    EXPECT_FALSE(loaded.loadBinary("truncated_string.bin"));
    std::remove("truncated_string.bin");
}

//проерка ветки, когда файл невозможно открыть для записи
TEST(MassivTest, SaveBinaryFailsWhenWriteFails) {
    Massiv massiv;
    const std::string largeValue(1024 * 1024, 'x');

    ASSERT_TRUE(massiv.addToEnd(largeValue));
    EXPECT_FALSE(massiv.saveBinary("/dev/full")); //диск заполнен
}

TEST(MassivTest, BranchCoverageExhaustive) {
    {
        Massiv zeroCap(0);
        Massiv nonZeroCap(5);

        Massiv copyZero(zeroCap);
        Massiv copyNonZero(nonZeroCap);

        Massiv a(0), b(4);
        a = b;
        b = zeroCap;
        a = a;
    }

    {
        Massiv m;
        EXPECT_FALSE(m.addAtIndex(5, "out_of_bounds"));
        EXPECT_TRUE(m.addAtIndex(0, "first"));
        EXPECT_TRUE(m.addAtIndex(1, "third"));
        EXPECT_TRUE(m.addAtIndex(1, "second"));
        std::string val;
        EXPECT_TRUE(m.getElement(1, val));
        EXPECT_EQ(val, "second");
    }

    {
        //rdbuf - позволяет управлять буфером
        //Без аргументов - возвращает укащатель на текущий буфер
        //С аргументами - оключает старый буфер и подключает новый
        auto* oldCin = std::cin.rdbuf();

        std::istringstream badCount("abc\n");
        std::cin.rdbuf(badCount.rdbuf());
        Massiv m1;
        EXPECT_FALSE(m1.readMassiv());

        std::istringstream incompleteData("3\nfirst\n");
        std::cin.rdbuf(incompleteData.rdbuf());
        Massiv m2;
        EXPECT_FALSE(m2.readMassiv());

        std::istringstream goodData("2\nhello\nworld\n");
        std::cin.rdbuf(goodData.rdbuf());
        Massiv m3;
        EXPECT_TRUE(m3.readMassiv());
        std::string val;
        EXPECT_TRUE(m3.getElement(0, val));
        EXPECT_EQ(val, "hello");

        std::cin.rdbuf(oldCin);
        std::cin.clear();
    }

    {
        Massiv m;
        m.addToEnd("");
        m.addToEnd("data");

        EXPECT_TRUE(m.saveBinary("massiv_branch_test.bin"));
        EXPECT_FALSE(m.saveBinary("/non_existent_directory_123/out.bin"));
    }

    {
        Massiv m;

        EXPECT_FALSE(m.loadBinary("non_existing_file_for_test.bin"));

        {
            std::ofstream f("empty.bin", std::ios::binary);
        }
        EXPECT_FALSE(m.loadBinary("empty.bin"));

        {
            std::ofstream f("trunc_len.bin", std::ios::binary);
            size_t count = 2;
            f.write(reinterpret_cast<char*>(&count), sizeof(count));
        }
        EXPECT_FALSE(m.loadBinary("trunc_len.bin"));

        {
            std::ofstream f("trunc_str.bin", std::ios::binary);
            size_t count = 1;
            size_t len = 50;
            f.write(reinterpret_cast<char*>(&count), sizeof(count));
            f.write(reinterpret_cast<char*>(&len), sizeof(len));
            f.write("abc", 3);
        }
        EXPECT_FALSE(m.loadBinary("trunc_str.bin"));

        EXPECT_TRUE(m.loadBinary("massiv_branch_test.bin"));
        std::string s;
        EXPECT_TRUE(m.getElement(0, s));
        EXPECT_EQ(s, "");
        EXPECT_TRUE(m.getElement(1, s));
        EXPECT_EQ(s, "data");

        std::remove("empty.bin");
        std::remove("trunc_len.bin");
        std::remove("trunc_str.bin");
        std::remove("massiv_branch_test.bin");
    }
}

TEST(MassivTest, ZeroCapacityAndEmptyCopies) {
    Massiv empty(0);
    EXPECT_TRUE(empty.isEmpty());

    Massiv copy(empty);
    EXPECT_TRUE(copy.isEmpty());

    Massiv assigned(2);
    assigned = empty;
    EXPECT_TRUE(assigned.isEmpty());
}

TEST(MassivTest, RejectsTruncatedBinaryFiles) {
    const std::string file = "massiv_truncated_test.bin";
    Massiv massiv;

    {
        std::ofstream out(file, std::ios::binary);
    }
    EXPECT_FALSE(massiv.loadBinary(file));

    {
        std::ofstream out(file, std::ios::binary);
        const size_t count = 1;
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    }
    EXPECT_FALSE(massiv.loadBinary(file));

    {
        std::ofstream out(file, std::ios::binary);
        const size_t count = 1;
        const size_t length = 3;
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        out.write(reinterpret_cast<const char*>(&length), sizeof(length));
        out.write("x", 1);
    }
    EXPECT_FALSE(massiv.loadBinary(file));

    std::remove(file.c_str());
}

TEST(MassivTest, ReadAndPrintBranches) {
    Massiv array(4);
    EXPECT_TRUE(array.isEmpty());

    array.printMassiv();
    ASSERT_TRUE(array.addToEnd("one"));
    array.printMassiv();

    auto readInput = [&array](const std::string& text) {
        std::istringstream input(text);
        std::streambuf* oldBuffer = std::cin.rdbuf(input.rdbuf());

        const bool result = array.readMassiv();

        std::cin.rdbuf(oldBuffer);
        std::cin.clear();
        return result;
    };

    EXPECT_TRUE(readInput("2 alpha beta"));
    EXPECT_EQ(array.getLength(), 2u);
    EXPECT_FALSE(readInput("not_a_number"));
    EXPECT_FALSE(readInput("2 only_one"));
}

// ------------------------------------------------------------
// 2. ОДНОСВЯЗНЫЙ СПИСОК (ListF)
// ------------------------------------------------------------

TEST(ListFTest, OperationsAndBoundaries) {
    ListF list;

    EXPECT_TRUE(list.isEmptyF());
    EXPECT_EQ(list.sizeF(), 0);
    EXPECT_EQ(list.searchF("missing"), -1);
    EXPECT_EQ(list.getAtF(-1), nullptr);
    EXPECT_EQ(list.getAtF(0), nullptr);
    EXPECT_EQ(list.findNodeF("missing"), nullptr);

    EXPECT_FALSE(list.popFrontF());
    EXPECT_FALSE(list.popBackF());
    EXPECT_FALSE(list.removeAfterF("missing"));
    EXPECT_FALSE(list.removeBeforeF("missing"));
    EXPECT_FALSE(list.removeByValueF("missing"));

    list.pushFrontF("b");
    list.pushFrontF("a");
    list.pushBackF("d");

    ASSERT_EQ(list.sizeF(), 3);
    EXPECT_EQ(list.searchF("a"), 0);
    EXPECT_EQ(list.searchF("d"), 2);
    EXPECT_EQ(list.searchF("missing"), -1);
    ASSERT_NE(list.getAtF(1), nullptr);
    EXPECT_EQ(list.getAtF(1)->data, "b");
    EXPECT_EQ(list.getAtF(3), nullptr);
    ASSERT_NE(list.findNodeF("d"), nullptr);

    EXPECT_TRUE(list.insertAfterF("b", "c"));
    EXPECT_TRUE(list.insertBeforeF("a", "first"));
    EXPECT_TRUE(list.insertBeforeF("c", "between"));
    EXPECT_FALSE(list.insertAfterF("missing", "x"));
    EXPECT_FALSE(list.insertBeforeF("missing", "x"));

    EXPECT_TRUE(list.removeBeforeF("a"));
    EXPECT_TRUE(list.removeBeforeF("c"));
    EXPECT_FALSE(list.removeBeforeF("a"));

    EXPECT_TRUE(list.removeAfterF("b"));
    list.pushBackF("tail");
    EXPECT_TRUE(list.removeAfterF("d"));
    EXPECT_FALSE(list.removeAfterF("d"));

    EXPECT_TRUE(list.removeByValueF("a"));
    EXPECT_TRUE(list.removeByValueF("d"));
    EXPECT_FALSE(list.removeByValueF("missing"));

    while (list.popFrontF()) {}
    EXPECT_TRUE(list.isEmptyF());
    EXPECT_FALSE(list.popBackF());
}

TEST(ListFTest, AdditionalDeleteBoundaryCoverage) {
    ListF beforeLast;
    beforeLast.pushBackF("a");
    beforeLast.pushBackF("b");
    beforeLast.pushBackF("c");
    beforeLast.pushBackF("d");

    EXPECT_TRUE(beforeLast.removeBeforeF("d"));
    EXPECT_FALSE(beforeLast.removeBeforeF("missing"));

    ListF tail;
    tail.pushBackF("a");
    tail.pushBackF("b");
    tail.pushBackF("c");

    EXPECT_TRUE(tail.removeByValueF("c"));
    tail.pushBackF("d");
    EXPECT_TRUE(tail.removeByValueF("d"));
    EXPECT_FALSE(tail.removeByValueF("missing"));

    ListF single;
    single.pushBackF("only");
    single.popBackF();
}

TEST(ListFTest, RemoveHeadByValue) {
    ListF list;
    list.pushBackF("first");
    list.pushBackF("second");

    EXPECT_TRUE(list.removeByValueF("first"));
    EXPECT_FALSE(list.removeByValueF("first"));
    EXPECT_TRUE(list.removeByValueF("second"));
    EXPECT_FALSE(list.removeByValueF("second"));
}

TEST(ListFTest, SerializationAndPrintMethods) {
    const char* filename = "listf_test.txt";

    ListF list;
    EXPECT_TRUE(list.saveListFToFile(filename));
    EXPECT_TRUE(list.loadListFFromFile(filename));

    list.pushBackF("one");
    list.pushBackF("two");
    list.printForwardF();
    list.printBackwardF();
    list.printIndexedF();

    ASSERT_TRUE(list.saveListFToFile(filename));

    ListF loaded;
    ASSERT_TRUE(loaded.loadListFFromFile(filename));
    ASSERT_EQ(loaded.sizeF(), 2);
    EXPECT_EQ(loaded.getAtF(0)->data, "one");
    EXPECT_EQ(loaded.getAtF(1)->data, "two");

    EXPECT_FALSE(loaded.loadListFFromFile("missing_listf_file.txt"));
    std::remove(filename);
}

TEST(ListFTest, BinarySerialization) {
    const char* filename = "listf_binary_test.bin";

    ListF original;
    original.pushBackF("first value");
    original.pushBackF("second");

    ASSERT_TRUE(original.saveListFBinary(filename));

    ListF loaded;
    ASSERT_TRUE(loaded.loadListFBinary(filename));

    ASSERT_EQ(loaded.sizeF(), 2);
    ASSERT_NE(loaded.getAtF(0), nullptr);
    ASSERT_NE(loaded.getAtF(1), nullptr);

    EXPECT_EQ(loaded.getAtF(0)->data, "first value");
    EXPECT_EQ(loaded.getAtF(1)->data, "second");
    std::remove(filename);
}

TEST(ListFTest, CoversRemoveBeforeAndTailRemovalBranches) {
    ListF list;

    EXPECT_FALSE(list.removeBeforeF("missing"));

    list.pushBackF("a");
    EXPECT_FALSE(list.removeBeforeF("a"));
    EXPECT_FALSE(list.removeBeforeF("missing"));

    list.pushBackF("b");
    list.pushBackF("c");
    list.pushBackF("d");
    list.pushBackF("e");

    EXPECT_TRUE(list.removeBeforeF("b"));
    EXPECT_TRUE(list.removeBeforeF("e"));
    EXPECT_FALSE(list.removeBeforeF("missing"));

    EXPECT_TRUE(list.removeByValueF("e"));
    EXPECT_TRUE(list.removeByValueF("b"));
    EXPECT_FALSE(list.removeByValueF("missing"));
}

TEST(ListFTest, CoversEmptyStringsAndNullBinaryFilenames) {
    ListF list;

    EXPECT_FALSE(list.saveListFBinary(nullptr));
    EXPECT_FALSE(list.loadListFBinary(nullptr));

    list.pushBackF("");
    list.pushBackF("value");
    ASSERT_TRUE(list.saveListFBinary("list_empty_string.bin"));

    ListF loaded;
    ASSERT_TRUE(loaded.loadListFBinary("list_empty_string.bin"));
    ASSERT_EQ(loaded.sizeF(), 2);
    ASSERT_NE(loaded.getAtF(0), nullptr);
    EXPECT_EQ(loaded.getAtF(0)->data, "");

    std::remove("list_empty_string.bin");
}

TEST(ListFTest, PopBackSingleAndMultipleElements) {
    ListF list;

    list.pushBackF("only");
    EXPECT_TRUE(list.popBackF());
    EXPECT_TRUE(list.isEmptyF());

    list.pushBackF("first");
    list.pushBackF("middle");
    list.pushBackF("last");

    EXPECT_TRUE(list.popBackF());
    ASSERT_EQ(list.sizeF(), 2);
    ASSERT_NE(list.getAtF(1), nullptr);
    EXPECT_EQ(list.getAtF(1)->data, "middle");

    EXPECT_TRUE(list.popBackF());
    EXPECT_TRUE(list.popBackF());
    EXPECT_FALSE(list.popBackF());
}

TEST(ListFTest, CoverMissingBoundaryBranches) {
    ListF empty;

    EXPECT_FALSE(empty.insertBeforeF("missing", "x"));
    empty.printForwardF();
    empty.printBackwardF();
    empty.printIndexedF();

    ListF list;
    list.pushBackF("a");
    list.pushBackF("b");
    list.pushBackF("c");

    EXPECT_TRUE(list.insertAfterF("c", "d"));
    EXPECT_TRUE(list.removeBeforeF("c"));
    EXPECT_TRUE(list.removeAfterF("c"));
    EXPECT_TRUE(list.removeByValueF("c"));

    const char* file = "listf_reload_test.txt";
    {
        std::ofstream out(file);
        out << "new1 new2";
    }

    list.pushBackF("old");
    ASSERT_TRUE(list.loadListFFromFile(file));
    EXPECT_EQ(list.sizeF(), 2);
    EXPECT_EQ(list.getAtF(0)->data, "new1");

    std::remove(file);
}

// ------------------------------------------------------------
// 3. ДВУСВЯЗНЫЙ СПИСОК (ListD)
// ------------------------------------------------------------

TEST(ListDTest, OperationsAndBoundaries) {
    ListD list;

    EXPECT_TRUE(list.isEmptyD());
    EXPECT_EQ(list.sizeD(), 0);
    EXPECT_EQ(list.searchD("missing"), -1);
    EXPECT_EQ(list.getAtD(-1), nullptr);
    EXPECT_EQ(list.getAtD(0), nullptr);
    EXPECT_EQ(list.findNodeD("missing"), nullptr);

    EXPECT_FALSE(list.popFrontD());
    EXPECT_FALSE(list.popBackD());
    EXPECT_FALSE(list.removeAfterD("missing"));
    EXPECT_FALSE(list.removeBeforeD("missing"));
    EXPECT_FALSE(list.removeByValueD("missing"));

    list.pushFrontD("b");
    list.pushFrontD("a");
    list.pushBackD("d");

    ASSERT_EQ(list.sizeD(), 3);
    EXPECT_EQ(list.searchD("a"), 0);
    EXPECT_EQ(list.searchD("d"), 2);
    EXPECT_EQ(list.searchD("missing"), -1);
    ASSERT_NE(list.getAtD(1), nullptr);
    EXPECT_EQ(list.getAtD(1)->data, "b");
    EXPECT_EQ(list.getAtD(3), nullptr);
    ASSERT_NE(list.findNodeD("d"), nullptr);

    EXPECT_TRUE(list.insertAfterD("b", "c"));
    EXPECT_TRUE(list.insertBeforeD("a", "first"));
    EXPECT_FALSE(list.insertAfterD("missing", "x"));
    EXPECT_FALSE(list.insertBeforeD("missing", "x"));

    EXPECT_TRUE(list.removeBeforeD("a"));
    EXPECT_TRUE(list.removeBeforeD("c"));
    EXPECT_FALSE(list.removeBeforeD("first"));

    EXPECT_TRUE(list.removeAfterD("a"));
    EXPECT_FALSE(list.removeAfterD("d"));

    list.pushFrontD("head");
    list.pushBackD("tail");
    EXPECT_TRUE(list.removeByValueD("head"));
    EXPECT_TRUE(list.removeByValueD("tail"));
    EXPECT_FALSE(list.removeByValueD("missing"));

    EXPECT_TRUE(list.popFrontD());
    EXPECT_TRUE(list.popBackD());
    EXPECT_TRUE(list.isEmptyD());
}

TEST(ListDTest, AdditionalDeleteBoundaryCoverage) {
    ListD beforeHead;
    beforeHead.pushBackD("a");
    beforeHead.pushBackD("b");

    EXPECT_FALSE(beforeHead.removeBeforeD("a"));
    EXPECT_FALSE(beforeHead.removeBeforeD("missing"));

    ListD multiple;
    multiple.pushBackD("a");
    multiple.pushBackD("b");
    multiple.pushBackD("c");
    multiple.popBackD();

    ListD single;
    single.pushBackD("only");
    single.popBackD();
}

TEST(ListDTest, SerializationAndPrintMethods) {
    const char* filename = "listd_test.txt";

    ListD list;
    EXPECT_TRUE(list.saveListDToFile(filename));
    EXPECT_TRUE(list.loadListDFromFile(filename));

    list.pushBackD("one");
    list.pushBackD("two");
    list.printForwardD();
    list.printBackwardD();
    list.printIndexedD();

    ASSERT_TRUE(list.saveListDToFile(filename));

    ListD loaded;
    ASSERT_TRUE(loaded.loadListDFromFile(filename));
    ASSERT_EQ(loaded.sizeD(), 2);
    EXPECT_EQ(loaded.getAtD(0)->data, "one");
    EXPECT_EQ(loaded.getAtD(1)->data, "two");

    EXPECT_FALSE(loaded.loadListDFromFile("missing_listd_file.txt"));
    std::remove(filename);
}

TEST(ListDTest, BinarySerialization) {
    const char* filename = "listd_binary_test.bin";

    ListD original;
    original.pushBackD("first value");
    original.pushBackD("second");

    ASSERT_TRUE(original.saveListDBinary(filename));

    ListD loaded;
    ASSERT_TRUE(loaded.loadListDBinary(filename));

    ASSERT_EQ(loaded.sizeD(), 2);
    ASSERT_NE(loaded.getAtD(0), nullptr);
    ASSERT_NE(loaded.getAtD(1), nullptr);

    EXPECT_EQ(loaded.getAtD(0)->data, "first value");
    EXPECT_EQ(loaded.getAtD(1)->data, "second");
    std::remove(filename);
}

TEST(ListDTest, CoversNullFilenameEmptyStringAndBadMagic) {
    ListD list;

    EXPECT_FALSE(list.saveListDBinary(nullptr));
    EXPECT_FALSE(list.loadListDBinary(nullptr));

    list.pushBackD("");
    ASSERT_TRUE(list.saveListDBinary("listd_empty_string.bin"));

    ListD loaded;
    ASSERT_TRUE(loaded.loadListDBinary("listd_empty_string.bin"));
    ASSERT_EQ(loaded.sizeD(), 1);

    NodeD* item = loaded.getAtD(0);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->data, "");

    {
        std::ofstream out("listd_bad_magic.bin", std::ios::binary);
        ASSERT_TRUE(out.is_open());
        const char badMagic[8] = {'B', 'A', 'D', 'M', 'A', 'G', 'I', 'C'};
        out.write(badMagic, sizeof(badMagic));
    }

    ListD invalid;
    EXPECT_FALSE(invalid.loadListDBinary("listd_bad_magic.bin"));

    std::remove("listd_empty_string.bin");
    std::remove("listd_bad_magic.bin");
}

TEST(ListDTest, CoverMissingBoundaryBranches) {
    ListD empty;

    empty.printForwardD();
    empty.printBackwardD();
    empty.printIndexedD();

    ListD list;
    list.pushBackD("a");
    list.pushBackD("b");
    list.pushBackD("c");

    EXPECT_TRUE(list.insertAfterD("c", "d"));
    EXPECT_TRUE(list.insertBeforeD("c", "middle"));
    EXPECT_TRUE(list.removeAfterD("c"));
    EXPECT_TRUE(list.removeBeforeD("c"));

    ListD oneForFront;
    oneForFront.pushBackD("one");
    EXPECT_TRUE(oneForFront.popFrontD());
    EXPECT_TRUE(oneForFront.isEmptyD());

    ListD oneForBack;
    oneForBack.pushBackD("one");
    EXPECT_TRUE(oneForBack.popBackD());
    EXPECT_TRUE(oneForBack.isEmptyD());

    const char* file = "listd_reload_test.txt";
    {
        std::ofstream out(file);
        out << "new1 new2";
    }

    list.pushBackD("old");
    ASSERT_TRUE(list.loadListDFromFile(file));
    EXPECT_EQ(list.sizeD(), 2);

    std::remove(file);
}

// ------------------------------------------------------------
// 4. ОЧЕРЕДЬ (Queue)
// ------------------------------------------------------------

TEST(QueueTest, OperationsAndSerialization) {
    const char* filename = "queue_test.txt";

    Queue queue;
    std::string value;

    EXPECT_TRUE(queue.isQueueEmpty());
    EXPECT_EQ(queue.sizeQueue(), 0);
    EXPECT_FALSE(queue.peekQueue(value));
    EXPECT_FALSE(queue.popQueue(value));

    queue.printQueue();
    EXPECT_TRUE(queue.saveQueueToFile(filename));

    queue.pushQueue("first");
    queue.pushQueue("second");
    EXPECT_FALSE(queue.isQueueEmpty());
    EXPECT_EQ(queue.sizeQueue(), 2);
    ASSERT_TRUE(queue.peekQueue(value));
    EXPECT_EQ(value, "first");

    ASSERT_TRUE(queue.saveQueueToFile(filename));

    Queue loaded;
    ASSERT_TRUE(loaded.loadQueueFromFile(filename));
    EXPECT_EQ(loaded.sizeQueue(), 2);

    ASSERT_TRUE(loaded.popQueue(value));
    EXPECT_EQ(value, "first");
    ASSERT_TRUE(loaded.popQueue(value));
    EXPECT_EQ(value, "second");
    EXPECT_TRUE(loaded.isQueueEmpty());

    EXPECT_FALSE(loaded.popQueue(value));
    EXPECT_FALSE(loaded.loadQueueFromFile("missing_queue_file.txt"));

    std::remove(filename);
}

TEST(QueueTest, BinarySerialization) {
    const char* filename = "queue_binary_test.bin";

    Queue original;
    original.pushQueue("first value");
    original.pushQueue("second");

    ASSERT_TRUE(original.saveQueueBinary(filename));

    Queue loaded;
    ASSERT_TRUE(loaded.loadQueueBinary(filename));

    ASSERT_EQ(loaded.sizeQueue(), 2);

    std::string value;
    ASSERT_TRUE(loaded.peekQueue(value));
    EXPECT_EQ(value, "first value");

    ASSERT_TRUE(loaded.popQueue(value));
    EXPECT_EQ(value, "first value");

    ASSERT_TRUE(loaded.popQueue(value));
    EXPECT_EQ(value, "second");

    EXPECT_TRUE(loaded.isQueueEmpty());
    std::remove(filename);
}

TEST(QueueTest, BranchCoverageExhaustive) {
    Queue q;

    EXPECT_FALSE(q.loadQueueBinary(nullptr));
    EXPECT_FALSE(q.loadQueueBinary("no_such_file_q.bin"));

    {
        std::ofstream f("bad_magic_q.bin", std::ios::binary);
        f.write("BADMAGIC", 8);
    }
    EXPECT_FALSE(q.loadQueueBinary("bad_magic_q.bin"));

    {
        std::ofstream f("huge_count_q.bin", std::ios::binary);
        const char magic[8] = {'Q', 'U', 'E', 'U', 'E', '0', '1', '\0'};
        f.write(magic, 8);
        uint64_t hugeCount = 2'000'000;
        f.write(reinterpret_cast<char*>(&hugeCount), sizeof(hugeCount));
    }
    EXPECT_FALSE(q.loadQueueBinary("huge_count_q.bin"));

    {
        std::ofstream f("huge_len_q.bin", std::ios::binary);
        const char magic[8] = {'Q', 'U', 'E', 'U', 'E', '0', '1', '\0'};
        f.write(magic, 8);
        uint64_t count = 1;
        f.write(reinterpret_cast<char*>(&count), sizeof(count));
        uint64_t hugeLen = 20 * 1024 * 1024;
        f.write(reinterpret_cast<char*>(&hugeLen), sizeof(hugeLen));
    }
    EXPECT_FALSE(q.loadQueueBinary("huge_len_q.bin"));

    {
        std::ofstream f("trunc_q.bin", std::ios::binary);
        const char magic[8] = {'Q', 'U', 'E', 'U', 'E', '0', '1', '\0'};
        f.write(magic, 8);
        uint64_t count = 1;
        f.write(reinterpret_cast<char*>(&count), sizeof(count));
        uint64_t len = 50;
        f.write(reinterpret_cast<char*>(&len), sizeof(len));
        f.write("abc", 3);
    }
    EXPECT_FALSE(q.loadQueueBinary("trunc_q.bin"));

    {
        Queue qEmptyStr;
        qEmptyStr.pushQueue("");
        qEmptyStr.pushQueue("non-empty");
        ASSERT_TRUE(qEmptyStr.saveQueueBinary("empty_str_q.bin"));

        Queue loaded;
        ASSERT_TRUE(loaded.loadQueueBinary("empty_str_q.bin"));

        std::string value;
        ASSERT_TRUE(loaded.popQueue(value));
        EXPECT_EQ(value, "");
        ASSERT_TRUE(loaded.popQueue(value));
        EXPECT_EQ(value, "non-empty");

        std::remove("empty_str_q.bin");
    }

    EXPECT_FALSE(q.saveQueueBinary(nullptr));

    std::remove("bad_magic_q.bin");
    std::remove("huge_count_q.bin");
    std::remove("huge_len_q.bin");
    std::remove("trunc_q.bin");
}

TEST(QueueTest, PrintNonEmptyQueue) {
    Queue queue;
    queue.pushQueue("first");
    queue.pushQueue("second");
    queue.printQueue();
}

TEST(QueueTest, LoadOverNonEmptyQueue) {
    const char* file = "queue_reload_test.txt";
    {
        std::ofstream out(file);
        out << "new1\nnew2\n";
    }

    Queue queue;
    queue.pushQueue("old1");
    queue.pushQueue("old2");

    ASSERT_TRUE(queue.loadQueueFromFile(file));
    EXPECT_EQ(queue.sizeQueue(), 2);

    std::string value;
    ASSERT_TRUE(queue.popQueue(value));
    EXPECT_EQ(value, "new1");

    std::remove(file);
}

// ------------------------------------------------------------
// 5. СТЕК (Stack)
// ------------------------------------------------------------

TEST(StackTest, OperationsAndSerialization) {
    const char* filename = "stack_test.txt";

    Stack stack;
    std::string value;

    EXPECT_TRUE(stack.isStackEmpty());
    EXPECT_FALSE(stack.peekStack(value));
    EXPECT_FALSE(stack.popStack(value));

    stack.printStack();
    EXPECT_TRUE(stack.saveStackToFile(filename));

    stack.pushStack("bottom");
    stack.pushStack("top");

    ASSERT_TRUE(stack.peekStack(value));
    EXPECT_EQ(value, "top");

    ASSERT_TRUE(stack.saveStackToFile(filename));

    Stack loaded;
    ASSERT_TRUE(loaded.loadStackFromFile(filename));

    ASSERT_TRUE(loaded.popStack(value));
    EXPECT_EQ(value, "top");
    ASSERT_TRUE(loaded.popStack(value));
    EXPECT_EQ(value, "bottom");
    EXPECT_TRUE(loaded.isStackEmpty());

    EXPECT_FALSE(loaded.popStack(value));
    EXPECT_FALSE(loaded.loadStackFromFile("missing_stack_file.txt"));

    std::remove(filename);
}

TEST(StackTest, BinarySerialization) {
    const char* filename = "stack_binary_test.bin";

    Stack original;
    original.pushStack("bottom value");
    original.pushStack("top value");

    ASSERT_TRUE(original.saveStackBinary(filename));

    Stack loaded;
    ASSERT_TRUE(loaded.loadStackBinary(filename));

    std::string value;
    ASSERT_TRUE(loaded.popStack(value));
    EXPECT_EQ(value, "top value");
    ASSERT_TRUE(loaded.popStack(value));
    EXPECT_EQ(value, "bottom value");

    EXPECT_TRUE(loaded.isStackEmpty());
    std::remove(filename);
}

TEST(StackTest, BranchCoverageExhaustive) {
    Stack s;

    EXPECT_FALSE(s.loadStackBinary(nullptr));
    EXPECT_FALSE(s.loadStackBinary("no_such_file_s.bin"));

    {
        std::ofstream f("bad_magic_s.bin", std::ios::binary);
        f.write("BADMAGIC", 8);
    }
    EXPECT_FALSE(s.loadStackBinary("bad_magic_s.bin"));

    {
        std::ofstream f("huge_count_s.bin", std::ios::binary);
        const char magic[8] = {'S', 'T', 'A', 'C', 'K', '0', '1', '\0'};
        f.write(magic, 8);
        uint64_t hugeCount = 2'000'000;
        f.write(reinterpret_cast<char*>(&hugeCount), sizeof(hugeCount));
    }
    EXPECT_FALSE(s.loadStackBinary("huge_count_s.bin"));

    {
        std::ofstream f("huge_len_s.bin", std::ios::binary);
        const char magic[8] = {'S', 'T', 'A', 'C', 'K', '0', '1', '\0'};
        f.write(magic, 8);
        uint64_t count = 1;
        f.write(reinterpret_cast<char*>(&count), sizeof(count));
        uint64_t hugeLen = 20 * 1024 * 1024;
        f.write(reinterpret_cast<char*>(&hugeLen), sizeof(hugeLen));
    }
    EXPECT_FALSE(s.loadStackBinary("huge_len_s.bin"));

    {
        std::ofstream f("trunc_s.bin", std::ios::binary);
        const char magic[8] = {'S', 'T', 'A', 'C', 'K', '0', '1', '\0'};
        f.write(magic, 8);
        uint64_t count = 1;
        f.write(reinterpret_cast<char*>(&count), sizeof(count));
        uint64_t len = 50;
        f.write(reinterpret_cast<char*>(&len), sizeof(len));
        f.write("abc", 3);
    }
    EXPECT_FALSE(s.loadStackBinary("trunc_s.bin"));

    {
        Stack sEmptyStr;
        sEmptyStr.pushStack("");
        sEmptyStr.pushStack("top");
        ASSERT_TRUE(sEmptyStr.saveStackBinary("empty_str_s.bin"));

        Stack loaded;
        ASSERT_TRUE(loaded.loadStackBinary("empty_str_s.bin"));

        std::string value;
        ASSERT_TRUE(loaded.popStack(value));
        EXPECT_EQ(value, "top");
        ASSERT_TRUE(loaded.popStack(value));
        EXPECT_EQ(value, "");

        std::remove("empty_str_s.bin");
    }

    EXPECT_FALSE(s.saveStackBinary(nullptr));

    std::remove("bad_magic_s.bin");
    std::remove("huge_count_s.bin");
    std::remove("huge_len_s.bin");
    std::remove("trunc_s.bin");
}

TEST(StackTest, LoadOverNonEmptyStack) {
    const char* file = "stack_reload_test.txt";
    {
        std::ofstream out(file);
        out << "new1\nnew2\n";
    }

    Stack stack;
    stack.pushStack("old1");
    stack.pushStack("old2");

    ASSERT_TRUE(stack.loadStackFromFile(file));
    std::string value;
    ASSERT_TRUE(stack.popStack(value));
    EXPECT_EQ(value, "new2");

    std::remove(file);
}

TEST(StackTest, PrintNonEmptyStack) {
    Stack stack;
    stack.pushStack("first");
    stack.pushStack("second");
    stack.printStack();
}

// ------------------------------------------------------------
// 6. AVL-ДЕРЕВО (AVLTree)
// ------------------------------------------------------------

TEST(AVLTreeTest, FourInsertionRotations) {
    // LL
    {
        AVLTree tree;
        tree.insertT(30);
        tree.insertT(20);
        tree.insertT(10);
        ASSERT_NE(tree.findT(20), nullptr);
        EXPECT_EQ(tree.findT(20)->key, 20);
        EXPECT_EQ(tree.getHeightT(), 2);
    }

    // LR
    {
        AVLTree tree;
        tree.insertT(30);
        tree.insertT(10);
        tree.insertT(20);
        ASSERT_NE(tree.findT(20), nullptr);
        EXPECT_EQ(tree.getHeightT(), 2);
    }

    // RR
    {
        AVLTree tree;
        tree.insertT(10);
        tree.insertT(20);
        tree.insertT(30);
        ASSERT_NE(tree.findT(20), nullptr);
        EXPECT_EQ(tree.getHeightT(), 2);
    }

    // RL
    {
        AVLTree tree;
        tree.insertT(10);
        tree.insertT(30);
        tree.insertT(20);
        ASSERT_NE(tree.findT(20), nullptr);
        EXPECT_EQ(tree.getHeightT(), 2);
    }
}

TEST(AVLTreeTest, SearchRemovalTraversalAndSerialization) {
    const char* filename = "avl_test.txt";

    AVLTree tree;
    EXPECT_TRUE(tree.isEmpty());
    EXPECT_EQ(tree.countNodesT(), 0);
    EXPECT_EQ(tree.getHeightT(), 0);

    int steps = -1;
    EXPECT_EQ(tree.findWithStepsT(10, steps), nullptr);
    EXPECT_EQ(steps, 0);
    EXPECT_EQ(tree.findMinT(), nullptr);

    tree.printInOrderT();
    tree.printPreOrderT();
    tree.printPostOrderT();
    tree.printLevelOrderT();
    tree.printTreeT();
    tree.printTreeInfoT("Пустое дерево");

    EXPECT_TRUE(tree.saveTreeToFile(filename));
    EXPECT_TRUE(tree.loadTreeFromFile(filename));
    EXPECT_FALSE(tree.loadTreeFromFile("missing_avl_file.txt"));

    const int values[] = {40, 20, 60, 10, 30, 50, 70, 25, 35};
    for (int value : values) {
        tree.insertT(value);
    }

    tree.insertT(30);
    EXPECT_EQ(tree.countNodesT(), 9);
    EXPECT_FALSE(tree.isEmpty());

    ASSERT_NE(tree.findT(30), nullptr);
    EXPECT_EQ(tree.findT(30)->key, 30);
    EXPECT_EQ(tree.findT(999), nullptr);

    ASSERT_NE(tree.findWithStepsT(25, steps), nullptr);
    EXPECT_GT(steps, 0);
    EXPECT_EQ(tree.findWithStepsT(999, steps), nullptr);
    EXPECT_GT(steps, 0);

    ASSERT_NE(tree.findMinT(), nullptr);
    EXPECT_EQ(tree.findMinT()->key, 10);

    tree.printInOrderT();
    tree.printPreOrderT();
    tree.printPostOrderT();
    tree.printLevelOrderT();
    tree.printTreeT();
    tree.printTreeInfoT("Непустое дерево");

    ASSERT_TRUE(tree.saveTreeToFile(filename));

    AVLTree loaded;
    ASSERT_TRUE(loaded.loadTreeFromFile(filename));
    EXPECT_EQ(loaded.countNodesT(), tree.countNodesT());
    EXPECT_NE(loaded.findT(25), nullptr);

    tree.removeT(999);
    tree.removeT(10);
    tree.removeT(20);
    tree.removeT(40);
    EXPECT_EQ(tree.findT(10), nullptr);
    EXPECT_EQ(tree.findT(20), nullptr);
    EXPECT_EQ(tree.findT(40), nullptr);

    for (int value : {25, 30, 35, 50, 60, 70}) {
        tree.removeT(value);
    }
    EXPECT_TRUE(tree.isEmpty());
    EXPECT_EQ(tree.countNodesT(), 0);

    std::remove(filename);
}

TEST(AVLTreeTest, RemoveNodeWithOnlyLeftChild) {
    AVLTree tree;
    tree.insertT(20);
    tree.insertT(10);

    ASSERT_NE(tree.findT(20), nullptr);
    tree.removeT(20);

    EXPECT_EQ(tree.findT(20), nullptr);
    ASSERT_NE(tree.findT(10), nullptr);
    EXPECT_EQ(tree.countNodesT(), 1);
}

TEST(AVLTreeTest, DeletionRebalancingCases) {
    {
        AVLTree tree;
        for (int value : {4, 2, 6, 1, 3}) {
            tree.insertT(value);
        }
        tree.removeT(6);

        EXPECT_EQ(tree.countNodesT(), 4);
        EXPECT_NE(tree.findT(1), nullptr);
        EXPECT_NE(tree.findT(2), nullptr);
        EXPECT_NE(tree.findT(3), nullptr);
        EXPECT_NE(tree.findT(4), nullptr);
    }

    {
        AVLTree tree;
        for (int value : {4, 2, 6, 3}) {
            tree.insertT(value);
        }
        tree.removeT(6);

        EXPECT_EQ(tree.countNodesT(), 3);
        EXPECT_NE(tree.findT(2), nullptr);
        EXPECT_NE(tree.findT(3), nullptr);
        EXPECT_NE(tree.findT(4), nullptr);
    }

    {
        AVLTree tree;
        for (int value : {4, 2, 6, 5, 7}) {
            tree.insertT(value);
        }
        tree.removeT(2);

        EXPECT_EQ(tree.countNodesT(), 4);
        EXPECT_NE(tree.findT(4), nullptr);
        EXPECT_NE(tree.findT(5), nullptr);
        EXPECT_NE(tree.findT(6), nullptr);
        EXPECT_NE(tree.findT(7), nullptr);
    }

    {
        AVLTree tree;
        for (int value : {4, 2, 6, 5}) {
            tree.insertT(value);
        }
        tree.removeT(2);

        EXPECT_EQ(tree.countNodesT(), 3);
        EXPECT_NE(tree.findT(4), nullptr);
        EXPECT_NE(tree.findT(5), nullptr);
        EXPECT_NE(tree.findT(6), nullptr);
    }
}

TEST(AVLTreeTest, LoadInvalidTreeToken) {
    const char* validFilename = "avl_valid_test.txt";
    const char* invalidFilename = "invalid_avl_test.txt";

    AVLTree original;
    original.insertT(42);

    ASSERT_TRUE(original.saveTreeToFile(validFilename));

    AVLTree restored;
    ASSERT_TRUE(restored.loadTreeFromFile(validFilename));
    EXPECT_FALSE(restored.isEmpty());
    EXPECT_NE(restored.findT(42), nullptr);

    {
        std::ofstream out(invalidFilename);
        ASSERT_TRUE(out.is_open());
        out << "not_an_integer";
    }

    AVLTree tree;
    EXPECT_TRUE(tree.loadTreeFromFile(invalidFilename));
    EXPECT_TRUE(tree.isEmpty());

    std::remove(validFilename);
    std::remove(invalidFilename);
}


TEST(AVLTreeTest, BinarySerialization) {
    const char* filename = "avl_binary_test.bin";

    AVLTree original;
    original.insertT(42);
    original.insertT(21);
    original.insertT(63);

    ASSERT_TRUE(original.saveTreeBinary(filename));

    AVLTree restored;
    ASSERT_TRUE(restored.loadTreeBinary(filename));

    EXPECT_NE(restored.findT(42), nullptr);
    EXPECT_NE(restored.findT(21), nullptr);
    EXPECT_NE(restored.findT(63), nullptr);

    std::remove(filename);
}

// ------------------------------------------------------------
// 7. ХЕШ-ТАБЛИЦА (HashTable)
// ------------------------------------------------------------

TEST(HashTableTest, BasicOperationsAndTombstones) {
    HashTable table(1, LINEAR_PROBING);
    int value = -1;

    EXPECT_EQ(table.getCapacity(), 13u);
    EXPECT_TRUE(table.isEmpty());
    EXPECT_FALSE(table.hashFindInt("missing", value));
    EXPECT_FALSE(table.hashErase("missing"));

    EXPECT_TRUE(table.hashInsertInt("key", 10));
    EXPECT_FALSE(table.isEmpty());
    EXPECT_EQ(table.getSize(), 1u);

    ASSERT_TRUE(table.hashFindInt("key", value));
    EXPECT_EQ(value, 10);

    EXPECT_TRUE(table.hashInsertInt("key", 20));
    EXPECT_EQ(table.getSize(), 1u);
    ASSERT_TRUE(table.hashFindInt("key", value));
    EXPECT_EQ(value, 20);

    EXPECT_TRUE(table.hashErase("key"));
    EXPECT_TRUE(table.isEmpty());
    EXPECT_FALSE(table.hashFindInt("key", value));
    EXPECT_FALSE(table.hashErase("key"));

    EXPECT_TRUE(table.hashInsertInt("another", 30));
    ASSERT_TRUE(table.hashFindInt("another", value));
    EXPECT_EQ(value, 30);

    EXPECT_GT(table.getProbes(), 0u);
}

TEST(HashTableTest, ReusesDeletedSlotBeforeEmptySlot) {
    HashTable table(1, LINEAR_PROBING);

    int value = 42;
    ASSERT_TRUE(table.hashInsert("reuse-slot", &value));
    ASSERT_TRUE(table.hashErase("reuse-slot"));

    ASSERT_TRUE(table.hashInsert("reuse-slot", &value));

    void* found = nullptr;
    ASSERT_TRUE(table.hashFind("reuse-slot", found));
    EXPECT_EQ(found, &value);
}

TEST(HashTableTest, BinarySerialization) {
    const std::string filename = "hash_table_test.bin";

    HashTable original(4, DOUBLE_HASHING);
    ASSERT_TRUE(original.hashInsertInt("first", 123));
    ASSERT_TRUE(original.hashInsertInt("key with space", -7));
    ASSERT_TRUE(original.hashInsertInt("zero", 0));
    ASSERT_TRUE(original.saveBinary(filename));

    HashTable loaded(1, LINEAR_PROBING);
    ASSERT_TRUE(loaded.loadBinary(filename));

    int value = 0;
    EXPECT_TRUE(loaded.hashFindInt("first", value));
    EXPECT_EQ(value, 123);

    EXPECT_TRUE(loaded.hashFindInt("key with space", value));
    EXPECT_EQ(value, -7);

    EXPECT_TRUE(loaded.hashFindInt("zero", value));
    EXPECT_EQ(value, 0);

    EXPECT_FALSE(loaded.hashFindInt("missing", value));

    std::remove(filename.c_str());
}

TEST(HashTableTest, HandlesCollisionsAndFullTable) {
    HashTable table(1, LINEAR_PROBING);
    std::vector<std::string> keys;

    for (int i = 0; i < 13; ++i) {
        std::string key = "key_" + std::to_string(i);
        keys.push_back(key);
        ASSERT_TRUE(table.hashInsertInt(key, i));
    }

    EXPECT_EQ(table.getSize(), 13u);
    EXPECT_GT(table.getCollisions(), 0u);

    int value = -1;
    EXPECT_FALSE(table.hashFindInt("not_present", value));
    EXPECT_FALSE(table.hashInsertInt("overflow", 100));
    EXPECT_FALSE(table.hashErase("not_present"));

    ASSERT_TRUE(table.hashErase(keys[0]));
    EXPECT_TRUE(table.hashInsertInt("replacement", 200));
    EXPECT_EQ(table.getSize(), 13u);

    ASSERT_TRUE(table.hashFindInt("replacement", value));
    EXPECT_EQ(value, 200);
}

TEST(HashTableTest, DoubleHashingAndGenericPointerValues) {
    HashTable table(2, DOUBLE_HASHING);
    EXPECT_EQ(table.getCapacity(), 17u);

    EXPECT_TRUE(table.hashInsertInt("first", 1));
    EXPECT_TRUE(table.hashInsertInt("second", 2));

    int value = -1;
    ASSERT_TRUE(table.hashFindInt("first", value));
    EXPECT_EQ(value, 1);

    int stored = 42;
    EXPECT_TRUE(table.hashInsert("pointer", &stored));

    void* result = nullptr;
    ASSERT_TRUE(table.hashFind("pointer", result));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*static_cast<int*>(result), 42);

    HashTable zeroExpected(0, LINEAR_PROBING);
    EXPECT_EQ(zeroExpected.getCapacity(), 11u);
    EXPECT_TRUE(zeroExpected.isEmpty());
}

TEST(HashTableTest, ReusesOneOfSeveralDeletedSlots) {
    HashTable table(1, LINEAR_PROBING);
    std::vector<std::string> keys;

    for (int i = 0; i < 13; ++i) {
        keys.push_back("deleted_test_" + std::to_string(i));
        ASSERT_TRUE(table.hashInsertInt(keys.back(), i));
    }

    ASSERT_TRUE(table.hashErase(keys[0]));
    ASSERT_TRUE(table.hashErase(keys[1]));

    EXPECT_TRUE(table.hashInsertInt("replacement_after_two_deletes", 99));

    int value = -1;
    ASSERT_TRUE(table.hashFindInt("replacement_after_two_deletes", value));
    EXPECT_EQ(value, 99);
    EXPECT_EQ(table.getSize(), 12u);
}


// ------------------------------------------------------------
// 8. ОБЩИЕ ПРОВЕРКИ ОШИБОК ФАЙЛОВ
// ------------------------------------------------------------

TEST(CoverageTest, SaveFailsWhenFileCannotBeOpened) {
    const char* badPath =
        "/directory_that_does_not_exist_12345/test.txt";

    ListF listF;
    EXPECT_FALSE(listF.saveListFToFile(badPath));

    ListD listD;
    EXPECT_FALSE(listD.saveListDToFile(badPath));

    Queue queue;
    EXPECT_FALSE(queue.saveQueueToFile(badPath));

    Stack stack;
    EXPECT_FALSE(stack.saveStackToFile(badPath));

    AVLTree tree;
    EXPECT_FALSE(tree.saveTreeToFile(badPath));
}