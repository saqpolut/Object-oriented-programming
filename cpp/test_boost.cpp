#define BOOST_TEST_MODULE AbstractDataStructuresTests
#include <boost/test/included/unit_test.hpp>

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
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

// Вспомогательный доступ к reserve для тестирования ёмкости массива.
struct MassivTestAccess {
    static bool reserve(Massiv& massiv, std::size_t capacity) {
        return massiv.reserve(capacity);
    }
};

// Временно подменяет std::cin и восстанавливает его после теста.
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

// Записывает uint64_t в бинарный файл.
static void writeUint64(std::ofstream& file, std::uint64_t value) {
    file.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

// ============================================================
// Массив Massiv
// ============================================================

BOOST_AUTO_TEST_CASE(Massiv_ReserveDoesNotShrinkCapacity) {
    Massiv massiv(4);

    BOOST_REQUIRE(massiv.addToEnd("first"));
    BOOST_REQUIRE(massiv.addToEnd("second"));

    BOOST_CHECK(MassivTestAccess::reserve(massiv, 4));
    BOOST_CHECK(MassivTestAccess::reserve(massiv, 2));

    std::string value;
    BOOST_REQUIRE(massiv.getElement(0, value));
    BOOST_CHECK_EQUAL(value, "first");
    BOOST_REQUIRE(massiv.getElement(1, value));
    BOOST_CHECK_EQUAL(value, "second");
}

BOOST_AUTO_TEST_CASE(Massiv_OperationsCopyingAndBoundaries) {
    Massiv array;
    std::string value;

    BOOST_CHECK(array.isEmpty());
    BOOST_CHECK_EQUAL(array.getLength(), 0u);
    BOOST_CHECK(!array.getElement(0, value));
    BOOST_CHECK(!array.removeAtIndex(0));
    BOOST_CHECK(!array.replaceAtIndex(0, "x"));
    BOOST_CHECK(!array.addAtIndex(1, "x"));

    BOOST_CHECK(array.addToEnd("a"));
    BOOST_CHECK(array.addToEnd("b"));
    BOOST_CHECK(array.addAtIndex(1, "middle"));
    BOOST_CHECK_EQUAL(array.getLength(), 3u);

    BOOST_REQUIRE(array.getElement(0, value));
    BOOST_CHECK_EQUAL(value, "a");
    BOOST_REQUIRE(array.getElement(1, value));
    BOOST_CHECK_EQUAL(value, "middle");
    BOOST_REQUIRE(array.getElement(2, value));
    BOOST_CHECK_EQUAL(value, "b");

    BOOST_CHECK(array.replaceAtIndex(0, "changed"));
    BOOST_CHECK(!array.replaceAtIndex(10, "bad"));
    BOOST_CHECK(!array.getElement(10, value));

    Massiv copy(array);
    BOOST_REQUIRE(copy.getElement(0, value));
    BOOST_CHECK_EQUAL(value, "changed");

    Massiv assigned;
    assigned = copy;
    BOOST_REQUIRE(assigned.getElement(1, value));
    BOOST_CHECK_EQUAL(value, "middle");

    assigned = assigned;
    BOOST_CHECK_EQUAL(assigned.getLength(), 3u);

    BOOST_CHECK(array.removeAtIndex(1));
    BOOST_CHECK(!array.removeAtIndex(10));
    BOOST_CHECK_EQUAL(array.getLength(), 2u);

    BOOST_CHECK(array.removeAtIndex(1));
    BOOST_CHECK(array.removeAtIndex(0));
    BOOST_CHECK(array.isEmpty());

    BOOST_CHECK(array.addAtIndex(0, "first"));
    BOOST_REQUIRE(array.getElement(0, value));
    BOOST_CHECK_EQUAL(value, "first");
}

BOOST_AUTO_TEST_CASE(Massiv_ZeroCapacityCopyAndAssignment) {
    Massiv zeroCapacity(0);
    Massiv zeroCopy(zeroCapacity);
    BOOST_CHECK(zeroCopy.isEmpty());

    Massiv reserved(4);
    Massiv reservedCopy(reserved);
    BOOST_CHECK(reservedCopy.isEmpty());

    Massiv source(3);
    BOOST_REQUIRE(source.addToEnd("one"));
    BOOST_REQUIRE(source.addToEnd("two"));

    Massiv copy(source);
    std::string value;
    BOOST_REQUIRE(copy.getElement(0, value));
    BOOST_CHECK_EQUAL(value, "one");
    BOOST_REQUIRE(copy.getElement(1, value));
    BOOST_CHECK_EQUAL(value, "two");

    Massiv assigned(0);
    assigned = source;
    BOOST_REQUIRE(assigned.getElement(0, value));
    BOOST_CHECK_EQUAL(value, "one");

    assigned = assigned;
    BOOST_REQUIRE(assigned.getElement(1, value));
    BOOST_CHECK_EQUAL(value, "two");

    assigned = zeroCapacity;
    BOOST_CHECK(assigned.isEmpty());
}

BOOST_AUTO_TEST_CASE(Massiv_InsertIntoEmptyAndGrow) {
    Massiv array(0);

    BOOST_CHECK(array.isEmpty());
    BOOST_CHECK(!array.addAtIndex(5, "out-of-bounds"));
    BOOST_CHECK(array.addAtIndex(0, "first"));
    BOOST_CHECK(array.addAtIndex(1, "third"));
    BOOST_CHECK(array.addAtIndex(1, "second"));

    std::string value;
    BOOST_REQUIRE(array.getElement(1, value));
    BOOST_CHECK_EQUAL(value, "second");
    BOOST_REQUIRE(array.getElement(2, value));
    BOOST_CHECK_EQUAL(value, "third");
}

BOOST_AUTO_TEST_CASE(Massiv_ReadInputSuccessAndErrors) {
    Massiv array;
    std::string value;

    {
        std::istringstream input("3 alpha beta gamma");
        CinRedirect redirect(input);

        BOOST_REQUIRE(array.readMassiv());
        BOOST_REQUIRE_EQUAL(array.getLength(), 3u);
        BOOST_REQUIRE(array.getElement(0, value));
        BOOST_CHECK_EQUAL(value, "alpha");
        BOOST_REQUIRE(array.getElement(2, value));
        BOOST_CHECK_EQUAL(value, "gamma");
    }

    {
        std::istringstream input("not_a_number");
        CinRedirect redirect(input);
        BOOST_CHECK(!array.readMassiv());
    }

    {
        std::istringstream input("3 only_one");
        CinRedirect redirect(input);
        BOOST_CHECK(!array.readMassiv());
    }

    {
        std::istringstream input("0");
        CinRedirect redirect(input);
        BOOST_CHECK(array.readMassiv());
        BOOST_CHECK(array.isEmpty());
    }
}

BOOST_AUTO_TEST_CASE(Massiv_PrintEmptyAndNonEmpty) {
    Massiv array;
    array.printMassiv();

    BOOST_REQUIRE(array.addToEnd("one"));
    array.printMassiv();
}

BOOST_AUTO_TEST_CASE(Massiv_BinarySerializationRoundTrip) {
    const char* filename = "massiv_boost_test.bin";

    Massiv original;
    BOOST_REQUIRE(original.addToEnd("alpha"));
    BOOST_REQUIRE(original.addToEnd("beta"));
    BOOST_REQUIRE(original.saveBinary(filename));

    Massiv loaded;
    BOOST_REQUIRE(loaded.loadBinary(filename));
    BOOST_REQUIRE_EQUAL(loaded.getLength(), 2u);

    std::string value;
    BOOST_REQUIRE(loaded.getElement(0, value));
    BOOST_CHECK_EQUAL(value, "alpha");
    BOOST_REQUIRE(loaded.getElement(1, value));
    BOOST_CHECK_EQUAL(value, "beta");

    BOOST_CHECK(!loaded.loadBinary("massiv_file_that_does_not_exist.bin"));
    BOOST_CHECK(!original.saveBinary(
        "/directory_that_does_not_exist_12345/massiv.bin"));

    std::remove(filename);
}

BOOST_AUTO_TEST_CASE(Massiv_BinarySerializationEmptyStringAndWriteFailure) {
    const char* filename = "massiv_boost_empty_string.bin";

    Massiv original;
    BOOST_REQUIRE(original.addToEnd(""));
    BOOST_REQUIRE(original.addToEnd("data"));
    BOOST_REQUIRE(original.saveBinary(filename));

    Massiv loaded;
    BOOST_REQUIRE(loaded.loadBinary(filename));

    std::string value;
    BOOST_REQUIRE(loaded.getElement(0, value));
    BOOST_CHECK(value.empty());
    BOOST_REQUIRE(loaded.getElement(1, value));
    BOOST_CHECK_EQUAL(value, "data");

    std::remove(filename);
}

BOOST_AUTO_TEST_CASE(Massiv_LoadRejectsTruncatedBinaryFiles) {
    const char* filename = "massiv_boost_truncated.bin";
    Massiv massiv;

    // Файл не содержит даже поля count.
    {
        std::ofstream file(filename, std::ios::binary);
    }
    BOOST_CHECK(!massiv.loadBinary(filename));

    // count записан, длина строки отсутствует.
    {
        std::ofstream file(filename, std::ios::binary);
        const std::size_t count = 1;
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));
    }
    BOOST_CHECK(!massiv.loadBinary(filename));

    // count и длина строки есть, но содержимое обрезано.
    {
        std::ofstream file(filename, std::ios::binary);
        const std::size_t count = 1;
        const std::size_t length = 5;
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));
        file.write(reinterpret_cast<const char*>(&length), sizeof(length));
        file.write("ab", 2);
    }
    BOOST_CHECK(!massiv.loadBinary(filename));

    std::remove(filename);
}

// ============================================================
// Односвязный список ListF
// ============================================================

BOOST_AUTO_TEST_CASE(ListF_OperationsAndBoundaries) {
    ListF list;

    BOOST_CHECK(list.isEmptyF());
    BOOST_CHECK_EQUAL(list.sizeF(), 0);
    BOOST_CHECK_EQUAL(list.searchF("missing"), -1);
    BOOST_CHECK(list.getAtF(-1) == nullptr);
    BOOST_CHECK(list.getAtF(0) == nullptr);
    BOOST_CHECK(list.findNodeF("missing") == nullptr);

    BOOST_CHECK(!list.popFrontF());
    BOOST_CHECK(!list.popBackF());
    BOOST_CHECK(!list.removeAfterF("missing"));
    BOOST_CHECK(!list.removeBeforeF("missing"));
    BOOST_CHECK(!list.removeByValueF("missing"));

    list.pushFrontF("b");
    list.pushFrontF("a");
    list.pushBackF("d");

    BOOST_REQUIRE_EQUAL(list.sizeF(), 3);
    BOOST_CHECK_EQUAL(list.searchF("a"), 0);
    BOOST_CHECK_EQUAL(list.searchF("d"), 2);
    BOOST_CHECK_EQUAL(list.searchF("missing"), -1);

    BOOST_REQUIRE(list.getAtF(1) != nullptr);
    BOOST_CHECK_EQUAL(list.getAtF(1)->data, "b");
    BOOST_CHECK(list.getAtF(3) == nullptr);
    BOOST_CHECK(list.findNodeF("d") != nullptr);

    BOOST_CHECK(list.insertAfterF("b", "c"));
    BOOST_CHECK(list.insertBeforeF("a", "first"));
    BOOST_CHECK(list.insertBeforeF("c", "between"));
    BOOST_CHECK(!list.insertAfterF("missing", "x"));
    BOOST_CHECK(!list.insertBeforeF("missing", "x"));

    BOOST_CHECK(list.removeBeforeF("a"));
    BOOST_CHECK(list.removeBeforeF("c"));
    BOOST_CHECK(!list.removeBeforeF("a"));

    BOOST_CHECK(list.removeAfterF("b"));
    list.pushBackF("tail");
    BOOST_CHECK(list.removeAfterF("d"));
    BOOST_CHECK(!list.removeAfterF("d"));

    BOOST_CHECK(list.removeByValueF("a"));
    BOOST_CHECK(list.removeByValueF("d"));
    BOOST_CHECK(!list.removeByValueF("missing"));

    while (list.popFrontF()) {
    }

    BOOST_CHECK(list.isEmptyF());
    BOOST_CHECK(!list.popBackF());
}

BOOST_AUTO_TEST_CASE(ListF_RemoveBeforeAndTailCases) {
    ListF list;

    BOOST_CHECK(!list.removeBeforeF("missing"));

    list.pushBackF("a");
    BOOST_CHECK(!list.removeBeforeF("a"));
    BOOST_CHECK(!list.removeBeforeF("missing"));

    list.pushBackF("b");
    list.pushBackF("c");
    list.pushBackF("d");
    list.pushBackF("e");

    BOOST_CHECK(list.removeBeforeF("b"));
    BOOST_CHECK(list.removeBeforeF("e"));
    BOOST_CHECK(!list.removeBeforeF("missing"));

    BOOST_CHECK(list.removeByValueF("e"));
    BOOST_CHECK(list.removeByValueF("b"));
    BOOST_CHECK(!list.removeByValueF("missing"));

    ListF single;
    single.pushBackF("only");
    BOOST_CHECK(single.popBackF());
    BOOST_CHECK(single.isEmptyF());
}

BOOST_AUTO_TEST_CASE(ListF_TextSerializationAndPrinting) {
    const char* filename = "listf_boost_test.txt";

    ListF list;
    BOOST_CHECK(list.saveListFToFile(filename));
    BOOST_CHECK(list.loadListFFromFile(filename));

    list.pushBackF("one");
    list.pushBackF("two");
    list.printForwardF();
    list.printBackwardF();
    list.printIndexedF();

    BOOST_REQUIRE(list.saveListFToFile(filename));

    ListF loaded;
    BOOST_REQUIRE(loaded.loadListFFromFile(filename));
    BOOST_REQUIRE_EQUAL(loaded.sizeF(), 2);
    BOOST_CHECK_EQUAL(loaded.getAtF(0)->data, "one");
    BOOST_CHECK_EQUAL(loaded.getAtF(1)->data, "two");
    BOOST_CHECK(!loaded.loadListFFromFile("missing_listf_boost.txt"));

    ListF empty;
    empty.printForwardF();
    empty.printBackwardF();
    empty.printIndexedF();

    std::remove(filename);
}

BOOST_AUTO_TEST_CASE(ListF_BinarySerializationAndBadArguments) {
    const char* filename = "listf_boost_test.bin";

    ListF original;
    original.pushBackF("first value");
    original.pushBackF("second");

    BOOST_REQUIRE(original.saveListFBinary(filename));

    ListF loaded;
    BOOST_REQUIRE(loaded.loadListFBinary(filename));
    BOOST_REQUIRE_EQUAL(loaded.sizeF(), 2);
    BOOST_REQUIRE(loaded.getAtF(0) != nullptr);
    BOOST_REQUIRE(loaded.getAtF(1) != nullptr);
    BOOST_CHECK_EQUAL(loaded.getAtF(0)->data, "first value");
    BOOST_CHECK_EQUAL(loaded.getAtF(1)->data, "second");

    BOOST_CHECK(!loaded.saveListFBinary(nullptr));
    BOOST_CHECK(!loaded.loadListFBinary(nullptr));
    BOOST_CHECK(!loaded.loadListFBinary("missing_listf_boost.bin"));

    std::remove(filename);
}

BOOST_AUTO_TEST_CASE(ListF_EmptyStringAndReload) {
    const char* binaryFile = "listf_boost_empty.bin";
    const char* textFile = "listf_boost_reload.txt";

    ListF list;
    list.pushBackF("");
    list.pushBackF("value");
    BOOST_REQUIRE(list.saveListFBinary(binaryFile));

    ListF loadedBinary;
    BOOST_REQUIRE(loadedBinary.loadListFBinary(binaryFile));
    BOOST_REQUIRE(loadedBinary.getAtF(0) != nullptr);
    BOOST_CHECK(loadedBinary.getAtF(0)->data.empty());

    {
        std::ofstream file(textFile);
        file << "new1 new2";
    }

    list.pushBackF("old");
    BOOST_REQUIRE(list.loadListFFromFile(textFile));
    BOOST_CHECK_EQUAL(list.sizeF(), 2);
    BOOST_CHECK_EQUAL(list.getAtF(0)->data, "new1");

    std::remove(binaryFile);
    std::remove(textFile);
}

// ============================================================
// Двусвязный список ListD
// ============================================================

BOOST_AUTO_TEST_CASE(ListD_OperationsAndBoundaries) {
    ListD list;

    BOOST_CHECK(list.isEmptyD());
    BOOST_CHECK_EQUAL(list.sizeD(), 0);
    BOOST_CHECK_EQUAL(list.searchD("missing"), -1);
    BOOST_CHECK(list.getAtD(-1) == nullptr);
    BOOST_CHECK(list.getAtD(0) == nullptr);
    BOOST_CHECK(list.findNodeD("missing") == nullptr);

    BOOST_CHECK(!list.popFrontD());
    BOOST_CHECK(!list.popBackD());
    BOOST_CHECK(!list.removeAfterD("missing"));
    BOOST_CHECK(!list.removeBeforeD("missing"));
    BOOST_CHECK(!list.removeByValueD("missing"));

    list.pushFrontD("b");
    list.pushFrontD("a");
    list.pushBackD("d");

    BOOST_REQUIRE_EQUAL(list.sizeD(), 3);
    BOOST_CHECK_EQUAL(list.searchD("a"), 0);
    BOOST_CHECK_EQUAL(list.searchD("d"), 2);
    BOOST_CHECK_EQUAL(list.searchD("missing"), -1);

    BOOST_REQUIRE(list.getAtD(1) != nullptr);
    BOOST_CHECK_EQUAL(list.getAtD(1)->data, "b");
    BOOST_CHECK(list.getAtD(3) == nullptr);
    BOOST_CHECK(list.findNodeD("d") != nullptr);

    BOOST_CHECK(list.insertAfterD("b", "c"));
    BOOST_CHECK(list.insertBeforeD("a", "first"));
    BOOST_CHECK(!list.insertAfterD("missing", "x"));
    BOOST_CHECK(!list.insertBeforeD("missing", "x"));

    BOOST_CHECK(list.removeBeforeD("a"));
    BOOST_CHECK(list.removeBeforeD("c"));
    BOOST_CHECK(!list.removeBeforeD("first"));

    BOOST_CHECK(list.removeAfterD("a"));
    BOOST_CHECK(!list.removeAfterD("d"));

    list.pushFrontD("head");
    list.pushBackD("tail");
    BOOST_CHECK(list.removeByValueD("head"));
    BOOST_CHECK(list.removeByValueD("tail"));
    BOOST_CHECK(!list.removeByValueD("missing"));

    BOOST_CHECK(list.popFrontD());
    BOOST_CHECK(list.popBackD());
    BOOST_CHECK(list.isEmptyD());
}

BOOST_AUTO_TEST_CASE(ListD_SingleAndMultipleElementRemoval) {
    ListD singleFront;
    singleFront.pushBackD("one");
    BOOST_CHECK(singleFront.popFrontD());
    BOOST_CHECK(singleFront.isEmptyD());

    ListD singleBack;
    singleBack.pushBackD("one");
    BOOST_CHECK(singleBack.popBackD());
    BOOST_CHECK(singleBack.isEmptyD());

    ListD multiple;
    multiple.pushBackD("a");
    multiple.pushBackD("b");
    multiple.pushBackD("c");
    BOOST_CHECK(multiple.popBackD());
    BOOST_CHECK_EQUAL(multiple.sizeD(), 2);
}

BOOST_AUTO_TEST_CASE(ListD_TextSerializationAndPrinting) {
    const char* filename = "listd_boost_test.txt";

    ListD list;
    BOOST_CHECK(list.saveListDToFile(filename));
    BOOST_CHECK(list.loadListDFromFile(filename));

    list.pushBackD("one");
    list.pushBackD("two");
    list.printForwardD();
    list.printBackwardD();
    list.printIndexedD();

    BOOST_REQUIRE(list.saveListDToFile(filename));

    ListD loaded;
    BOOST_REQUIRE(loaded.loadListDFromFile(filename));
    BOOST_REQUIRE_EQUAL(loaded.sizeD(), 2);
    BOOST_CHECK_EQUAL(loaded.getAtD(0)->data, "one");
    BOOST_CHECK_EQUAL(loaded.getAtD(1)->data, "two");
    BOOST_CHECK(!loaded.loadListDFromFile("missing_listd_boost.txt"));

    ListD empty;
    empty.printForwardD();
    empty.printBackwardD();
    empty.printIndexedD();

    std::remove(filename);
}

BOOST_AUTO_TEST_CASE(ListD_BinarySerializationAndBadMagic) {
    const char* filename = "listd_boost_test.bin";

    ListD original;
    original.pushBackD("first value");
    original.pushBackD("second");
    BOOST_REQUIRE(original.saveListDBinary(filename));

    ListD loaded;
    BOOST_REQUIRE(loaded.loadListDBinary(filename));
    BOOST_REQUIRE_EQUAL(loaded.sizeD(), 2);
    BOOST_CHECK_EQUAL(loaded.getAtD(0)->data, "first value");
    BOOST_CHECK_EQUAL(loaded.getAtD(1)->data, "second");

    BOOST_CHECK(!loaded.saveListDBinary(nullptr));
    BOOST_CHECK(!loaded.loadListDBinary(nullptr));

    {
        std::ofstream file("listd_boost_bad_magic.bin", std::ios::binary);
        const char badMagic[8] = {'B', 'A', 'D', 'M', 'A', 'G', 'I', 'C'};
        file.write(badMagic, sizeof(badMagic));
    }

    ListD invalid;
    BOOST_CHECK(!invalid.loadListDBinary("listd_boost_bad_magic.bin"));

    std::remove(filename);
    std::remove("listd_boost_bad_magic.bin");
}

BOOST_AUTO_TEST_CASE(ListD_EmptyStringAndReload) {
    const char* binaryFile = "listd_boost_empty.bin";
    const char* textFile = "listd_boost_reload.txt";

    ListD list;
    list.pushBackD("");
    BOOST_REQUIRE(list.saveListDBinary(binaryFile));

    ListD loaded;
    BOOST_REQUIRE(loaded.loadListDBinary(binaryFile));
    BOOST_REQUIRE(loaded.getAtD(0) != nullptr);
    BOOST_CHECK(loaded.getAtD(0)->data.empty());

    {
        std::ofstream file(textFile);
        file << "new1 new2";
    }

    list.pushBackD("old");
    BOOST_REQUIRE(list.loadListDFromFile(textFile));
    BOOST_CHECK_EQUAL(list.sizeD(), 2);
    BOOST_CHECK_EQUAL(list.getAtD(0)->data, "new1");

    std::remove(binaryFile);
    std::remove(textFile);
}

// ============================================================
// Очередь Queue
// ============================================================

BOOST_AUTO_TEST_CASE(Queue_OperationsTextSerializationAndPrinting) {
    const char* filename = "queue_boost_test.txt";

    Queue queue;
    std::string value;

    BOOST_CHECK(queue.isQueueEmpty());
    BOOST_CHECK_EQUAL(queue.sizeQueue(), 0);
    BOOST_CHECK(!queue.peekQueue(value));
    BOOST_CHECK(!queue.popQueue(value));

    queue.printQueue();
    BOOST_CHECK(queue.saveQueueToFile(filename));

    queue.pushQueue("first");
    queue.pushQueue("second");
    BOOST_CHECK(!queue.isQueueEmpty());
    BOOST_CHECK_EQUAL(queue.sizeQueue(), 2);

    BOOST_REQUIRE(queue.peekQueue(value));
    BOOST_CHECK_EQUAL(value, "first");

    queue.printQueue();
    BOOST_REQUIRE(queue.saveQueueToFile(filename));

    Queue loaded;
    BOOST_REQUIRE(loaded.loadQueueFromFile(filename));
    BOOST_CHECK_EQUAL(loaded.sizeQueue(), 2);

    BOOST_REQUIRE(loaded.popQueue(value));
    BOOST_CHECK_EQUAL(value, "first");
    BOOST_REQUIRE(loaded.popQueue(value));
    BOOST_CHECK_EQUAL(value, "second");
    BOOST_CHECK(loaded.isQueueEmpty());
    BOOST_CHECK(!loaded.popQueue(value));
    BOOST_CHECK(!loaded.loadQueueFromFile("missing_queue_boost.txt"));

    std::remove(filename);
}

BOOST_AUTO_TEST_CASE(Queue_BinarySerializationRoundTrip) {
    const char* filename = "queue_boost_test.bin";

    Queue original;
    original.pushQueue("first value");
    original.pushQueue("second");
    BOOST_REQUIRE(original.saveQueueBinary(filename));

    Queue loaded;
    BOOST_REQUIRE(loaded.loadQueueBinary(filename));
    BOOST_CHECK_EQUAL(loaded.sizeQueue(), 2);

    std::string value;
    BOOST_REQUIRE(loaded.peekQueue(value));
    BOOST_CHECK_EQUAL(value, "first value");
    BOOST_REQUIRE(loaded.popQueue(value));
    BOOST_CHECK_EQUAL(value, "first value");
    BOOST_REQUIRE(loaded.popQueue(value));
    BOOST_CHECK_EQUAL(value, "second");
    BOOST_CHECK(loaded.isQueueEmpty());

    BOOST_CHECK(!loaded.saveQueueBinary(nullptr));
    BOOST_CHECK(!loaded.loadQueueBinary(nullptr));
    BOOST_CHECK(!loaded.loadQueueBinary("missing_queue_boost.bin"));

    std::remove(filename);
}

BOOST_AUTO_TEST_CASE(Queue_BinaryMalformedFiles) {
    Queue queue;

    {
        std::ofstream file("queue_boost_bad_magic.bin", std::ios::binary);
        file.write("BADMAGIC", 8);
    }
    BOOST_CHECK(!queue.loadQueueBinary("queue_boost_bad_magic.bin"));

    {
        std::ofstream file("queue_boost_huge_count.bin", std::ios::binary);
        const char magic[8] = {'Q', 'U', 'E', 'U', 'E', '0', '1', '\0'};
        file.write(magic, sizeof(magic));
        writeUint64(file, 2'000'000);
    }
    BOOST_CHECK(!queue.loadQueueBinary("queue_boost_huge_count.bin"));

    {
        std::ofstream file("queue_boost_huge_length.bin", std::ios::binary);
        const char magic[8] = {'Q', 'U', 'E', 'U', 'E', '0', '1', '\0'};
        file.write(magic, sizeof(magic));
        writeUint64(file, 1);
        writeUint64(file, 20u * 1024u * 1024u);
    }
    BOOST_CHECK(!queue.loadQueueBinary("queue_boost_huge_length.bin"));

    {
        std::ofstream file("queue_boost_truncated.bin", std::ios::binary);
        const char magic[8] = {'Q', 'U', 'E', 'U', 'E', '0', '1', '\0'};
        file.write(magic, sizeof(magic));
        writeUint64(file, 1);
        writeUint64(file, 50);
        file.write("abc", 3);
    }
    BOOST_CHECK(!queue.loadQueueBinary("queue_boost_truncated.bin"));

    std::remove("queue_boost_bad_magic.bin");
    std::remove("queue_boost_huge_count.bin");
    std::remove("queue_boost_huge_length.bin");
    std::remove("queue_boost_truncated.bin");
}

BOOST_AUTO_TEST_CASE(Queue_EmptyStringAndReload) {
    const char* binaryFile = "queue_boost_empty.bin";
    const char* textFile = "queue_boost_reload.txt";

    Queue original;
    original.pushQueue("");
    original.pushQueue("non-empty");
    BOOST_REQUIRE(original.saveQueueBinary(binaryFile));

    Queue loaded;
    BOOST_REQUIRE(loaded.loadQueueBinary(binaryFile));

    std::string value;
    BOOST_REQUIRE(loaded.popQueue(value));
    BOOST_CHECK(value.empty());
    BOOST_REQUIRE(loaded.popQueue(value));
    BOOST_CHECK_EQUAL(value, "non-empty");

    {
        std::ofstream file(textFile);
        file << "new1\nnew2\n";
    }

    Queue textLoaded;
    textLoaded.pushQueue("old");
    BOOST_REQUIRE(textLoaded.loadQueueFromFile(textFile));
    BOOST_CHECK_EQUAL(textLoaded.sizeQueue(), 2);
    BOOST_REQUIRE(textLoaded.popQueue(value));
    BOOST_CHECK_EQUAL(value, "new1");

    std::remove(binaryFile);
    std::remove(textFile);
}

// ============================================================
// Стек Stack
// ============================================================

BOOST_AUTO_TEST_CASE(Stack_OperationsTextSerializationAndPrinting) {
    const char* filename = "stack_boost_test.txt";

    Stack stack;
    std::string value;

    BOOST_CHECK(stack.isStackEmpty());
    BOOST_CHECK(!stack.peekStack(value));
    BOOST_CHECK(!stack.popStack(value));

    stack.printStack();
    BOOST_CHECK(stack.saveStackToFile(filename));

    stack.pushStack("bottom");
    stack.pushStack("top");

    BOOST_REQUIRE(stack.peekStack(value));
    BOOST_CHECK_EQUAL(value, "top");

    stack.printStack();
    BOOST_REQUIRE(stack.saveStackToFile(filename));

    Stack loaded;
    BOOST_REQUIRE(loaded.loadStackFromFile(filename));
    BOOST_REQUIRE(loaded.popStack(value));
    BOOST_CHECK_EQUAL(value, "top");
    BOOST_REQUIRE(loaded.popStack(value));
    BOOST_CHECK_EQUAL(value, "bottom");
    BOOST_CHECK(loaded.isStackEmpty());
    BOOST_CHECK(!loaded.popStack(value));
    BOOST_CHECK(!loaded.loadStackFromFile("missing_stack_boost.txt"));

    std::remove(filename);
}

BOOST_AUTO_TEST_CASE(Stack_BinarySerializationRoundTrip) {
    const char* filename = "stack_boost_test.bin";

    Stack original;
    original.pushStack("bottom value");
    original.pushStack("top value");
    BOOST_REQUIRE(original.saveStackBinary(filename));

    Stack loaded;
    BOOST_REQUIRE(loaded.loadStackBinary(filename));

    std::string value;
    BOOST_REQUIRE(loaded.popStack(value));
    BOOST_CHECK_EQUAL(value, "top value");
    BOOST_REQUIRE(loaded.popStack(value));
    BOOST_CHECK_EQUAL(value, "bottom value");
    BOOST_CHECK(loaded.isStackEmpty());

    BOOST_CHECK(!loaded.saveStackBinary(nullptr));
    BOOST_CHECK(!loaded.loadStackBinary(nullptr));
    BOOST_CHECK(!loaded.loadStackBinary("missing_stack_boost.bin"));

    std::remove(filename);
}

BOOST_AUTO_TEST_CASE(Stack_BinaryMalformedFiles) {
    Stack stack;

    {
        std::ofstream file("stack_boost_bad_magic.bin", std::ios::binary);
        file.write("BADMAGIC", 8);
    }
    BOOST_CHECK(!stack.loadStackBinary("stack_boost_bad_magic.bin"));

    {
        std::ofstream file("stack_boost_huge_count.bin", std::ios::binary);
        const char magic[8] = {'S', 'T', 'A', 'C', 'K', '0', '1', '\0'};
        file.write(magic, sizeof(magic));
        writeUint64(file, 2'000'000);
    }
    BOOST_CHECK(!stack.loadStackBinary("stack_boost_huge_count.bin"));

    {
        std::ofstream file("stack_boost_huge_length.bin", std::ios::binary);
        const char magic[8] = {'S', 'T', 'A', 'C', 'K', '0', '1', '\0'};
        file.write(magic, sizeof(magic));
        writeUint64(file, 1);
        writeUint64(file, 20u * 1024u * 1024u);
    }
    BOOST_CHECK(!stack.loadStackBinary("stack_boost_huge_length.bin"));

    {
        std::ofstream file("stack_boost_truncated.bin", std::ios::binary);
        const char magic[8] = {'S', 'T', 'A', 'C', 'K', '0', '1', '\0'};
        file.write(magic, sizeof(magic));
        writeUint64(file, 1);
        writeUint64(file, 50);
        file.write("abc", 3);
    }
    BOOST_CHECK(!stack.loadStackBinary("stack_boost_truncated.bin"));

    std::remove("stack_boost_bad_magic.bin");
    std::remove("stack_boost_huge_count.bin");
    std::remove("stack_boost_huge_length.bin");
    std::remove("stack_boost_truncated.bin");
}

BOOST_AUTO_TEST_CASE(Stack_EmptyStringAndReload) {
    const char* binaryFile = "stack_boost_empty.bin";
    const char* textFile = "stack_boost_reload.txt";

    Stack original;
    original.pushStack("");
    original.pushStack("top");
    BOOST_REQUIRE(original.saveStackBinary(binaryFile));

    Stack loaded;
    BOOST_REQUIRE(loaded.loadStackBinary(binaryFile));

    std::string value;
    BOOST_REQUIRE(loaded.popStack(value));
    BOOST_CHECK_EQUAL(value, "top");
    BOOST_REQUIRE(loaded.popStack(value));
    BOOST_CHECK(value.empty());

    {
        std::ofstream file(textFile);
        file << "new1\nnew2\n";
    }

    Stack textLoaded;
    textLoaded.pushStack("old");
    BOOST_REQUIRE(textLoaded.loadStackFromFile(textFile));
    BOOST_REQUIRE(textLoaded.popStack(value));
    BOOST_CHECK_EQUAL(value, "new2");

    std::remove(binaryFile);
    std::remove(textFile);
}

// ============================================================
// AVL-дерево
// ============================================================

BOOST_AUTO_TEST_CASE(AVL_FourInsertionRotations) {
    // LL
    {
        AVLTree tree;
        tree.insertT(30);
        tree.insertT(20);
        tree.insertT(10);

        BOOST_REQUIRE(tree.findT(20) != nullptr);
        BOOST_CHECK_EQUAL(tree.findT(20)->key, 20);
        BOOST_CHECK_EQUAL(tree.getHeightT(), 2);
    }

    // LR
    {
        AVLTree tree;
        tree.insertT(30);
        tree.insertT(10);
        tree.insertT(20);

        BOOST_REQUIRE(tree.findT(20) != nullptr);
        BOOST_CHECK_EQUAL(tree.getHeightT(), 2);
    }

    // RR
    {
        AVLTree tree;
        tree.insertT(10);
        tree.insertT(20);
        tree.insertT(30);

        BOOST_REQUIRE(tree.findT(20) != nullptr);
        BOOST_CHECK_EQUAL(tree.getHeightT(), 2);
    }

    // RL
    {
        AVLTree tree;
        tree.insertT(10);
        tree.insertT(30);
        tree.insertT(20);

        BOOST_REQUIRE(tree.findT(20) != nullptr);
        BOOST_CHECK_EQUAL(tree.getHeightT(), 2);
    }
}

BOOST_AUTO_TEST_CASE(AVL_SearchTraversalAndSerialization) {
    const char* filename = "avl_boost_test.txt";

    AVLTree tree;
    BOOST_CHECK(tree.isEmpty());
    BOOST_CHECK_EQUAL(tree.countNodesT(), 0);
    BOOST_CHECK_EQUAL(tree.getHeightT(), 0);

    int steps = -1;
    BOOST_CHECK(tree.findWithStepsT(10, steps) == nullptr);
    BOOST_CHECK_EQUAL(steps, 0);
    BOOST_CHECK(tree.findMinT() == nullptr);

    tree.printInOrderT();
    tree.printPreOrderT();
    tree.printPostOrderT();
    tree.printLevelOrderT();
    tree.printTreeT();
    tree.printTreeInfoT("Пустое дерево");

    BOOST_CHECK(tree.saveTreeToFile(filename));
    BOOST_CHECK(tree.loadTreeFromFile(filename));
    BOOST_CHECK(!tree.loadTreeFromFile("missing_avl_boost.txt"));

    const int values[] = {40, 20, 60, 10, 30, 50, 70, 25, 35};
    for (int value : values) {
        tree.insertT(value);
    }

    tree.insertT(30); // Дубликат не должен увеличить число узлов.
    BOOST_CHECK_EQUAL(tree.countNodesT(), 9);
    BOOST_CHECK(!tree.isEmpty());

    BOOST_REQUIRE(tree.findT(30) != nullptr);
    BOOST_CHECK_EQUAL(tree.findT(30)->key, 30);
    BOOST_CHECK(tree.findT(999) == nullptr);

    BOOST_REQUIRE(tree.findWithStepsT(25, steps) != nullptr);
    BOOST_CHECK(steps > 0);
    BOOST_CHECK(tree.findWithStepsT(999, steps) == nullptr);
    BOOST_CHECK(steps > 0);

    BOOST_REQUIRE(tree.findMinT() != nullptr);
    BOOST_CHECK_EQUAL(tree.findMinT()->key, 10);

    tree.printInOrderT();
    tree.printPreOrderT();
    tree.printPostOrderT();
    tree.printLevelOrderT();
    tree.printTreeT();
    tree.printTreeInfoT("Непустое дерево");

    BOOST_REQUIRE(tree.saveTreeToFile(filename));

    AVLTree loaded;
    BOOST_REQUIRE(loaded.loadTreeFromFile(filename));
    BOOST_CHECK_EQUAL(loaded.countNodesT(), tree.countNodesT());
    BOOST_CHECK(loaded.findT(25) != nullptr);

    tree.removeT(999);
    tree.removeT(10);
    tree.removeT(20);
    tree.removeT(40);

    BOOST_CHECK(tree.findT(10) == nullptr);
    BOOST_CHECK(tree.findT(20) == nullptr);
    BOOST_CHECK(tree.findT(40) == nullptr);

    for (int value : {25, 30, 35, 50, 60, 70}) {
        tree.removeT(value);
    }

    BOOST_CHECK(tree.isEmpty());
    BOOST_CHECK_EQUAL(tree.countNodesT(), 0);

    std::remove(filename);
}

BOOST_AUTO_TEST_CASE(AVL_RemoveNodeWithOneChild) {
    AVLTree tree;
    tree.insertT(20);
    tree.insertT(10);

    BOOST_REQUIRE(tree.findT(20) != nullptr);
    tree.removeT(20);

    BOOST_CHECK(tree.findT(20) == nullptr);
    BOOST_REQUIRE(tree.findT(10) != nullptr);
    BOOST_CHECK_EQUAL(tree.countNodesT(), 1);
}

BOOST_AUTO_TEST_CASE(AVL_DeletionRebalancingCases) {
    // Левый перекос, баланс левого потомка >= 0.
    {
        AVLTree tree;
        for (int value : {4, 2, 6, 1, 3}) {
            tree.insertT(value);
        }
        tree.removeT(6);

        BOOST_CHECK_EQUAL(tree.countNodesT(), 4);
        BOOST_CHECK(tree.findT(1) != nullptr);
        BOOST_CHECK(tree.findT(2) != nullptr);
        BOOST_CHECK(tree.findT(3) != nullptr);
        BOOST_CHECK(tree.findT(4) != nullptr);
    }

    // Левый перекос, баланс левого потомка < 0.
    {
        AVLTree tree;
        for (int value : {4, 2, 6, 3}) {
            tree.insertT(value);
        }
        tree.removeT(6);

        BOOST_CHECK_EQUAL(tree.countNodesT(), 3);
        BOOST_CHECK(tree.findT(2) != nullptr);
        BOOST_CHECK(tree.findT(3) != nullptr);
        BOOST_CHECK(tree.findT(4) != nullptr);
    }

    // Правый перекос, баланс правого потомка <= 0.
    {
        AVLTree tree;
        for (int value : {4, 2, 6, 5, 7}) {
            tree.insertT(value);
        }
        tree.removeT(2);

        BOOST_CHECK_EQUAL(tree.countNodesT(), 4);
        BOOST_CHECK(tree.findT(4) != nullptr);
        BOOST_CHECK(tree.findT(5) != nullptr);
        BOOST_CHECK(tree.findT(6) != nullptr);
        BOOST_CHECK(tree.findT(7) != nullptr);
    }

    // Правый перекос, баланс правого потомка > 0.
    {
        AVLTree tree;
        for (int value : {4, 2, 6, 5}) {
            tree.insertT(value);
        }
        tree.removeT(2);

        BOOST_CHECK_EQUAL(tree.countNodesT(), 3);
        BOOST_CHECK(tree.findT(4) != nullptr);
        BOOST_CHECK(tree.findT(5) != nullptr);
        BOOST_CHECK(tree.findT(6) != nullptr);
    }
}

BOOST_AUTO_TEST_CASE(AVL_LoadInvalidToken) {
    const char* filename = "avl_boost_invalid.txt";

    {
        std::ofstream file(filename);
        BOOST_REQUIRE(file.is_open());
        file << "not_an_integer";
    }

    AVLTree tree;
    BOOST_CHECK(tree.loadTreeFromFile(filename));
    BOOST_CHECK(tree.isEmpty());

    std::remove(filename);
}

// ============================================================
// Хеш-таблица HashTable
// ============================================================

BOOST_AUTO_TEST_CASE(HashTable_BasicOperationsAndTombstones) {
    HashTable table(1, LINEAR_PROBING);
    int value = -1;

    BOOST_CHECK_EQUAL(table.getCapacity(), 13u);
    BOOST_CHECK(table.isEmpty());
    BOOST_CHECK(!table.hashFindInt("missing", value));
    BOOST_CHECK(!table.hashErase("missing"));

    BOOST_CHECK(table.hashInsertInt("key", 10));
    BOOST_CHECK(!table.isEmpty());
    BOOST_CHECK_EQUAL(table.getSize(), 1u);

    BOOST_REQUIRE(table.hashFindInt("key", value));
    BOOST_CHECK_EQUAL(value, 10);

    BOOST_CHECK(table.hashInsertInt("key", 20));
    BOOST_CHECK_EQUAL(table.getSize(), 1u);
    BOOST_REQUIRE(table.hashFindInt("key", value));
    BOOST_CHECK_EQUAL(value, 20);

    BOOST_CHECK(table.hashErase("key"));
    BOOST_CHECK(table.isEmpty());
    BOOST_CHECK(!table.hashFindInt("key", value));
    BOOST_CHECK(!table.hashErase("key"));

    BOOST_CHECK(table.hashInsertInt("another", 30));
    BOOST_REQUIRE(table.hashFindInt("another", value));
    BOOST_CHECK_EQUAL(value, 30);
    BOOST_CHECK(table.getProbes() > 0u);
}

BOOST_AUTO_TEST_CASE(HashTable_ReusesDeletedSlot) {
    HashTable table(1, LINEAR_PROBING);

    int value = 42;
    BOOST_REQUIRE(table.hashInsert("reuse-slot", &value));
    BOOST_REQUIRE(table.hashErase("reuse-slot"));
    BOOST_REQUIRE(table.hashInsert("reuse-slot", &value));

    void* found = nullptr;
    BOOST_REQUIRE(table.hashFind("reuse-slot", found));
    BOOST_CHECK(found == &value);
}

BOOST_AUTO_TEST_CASE(HashTable_CollisionsAndFullTable) {
    HashTable table(1, LINEAR_PROBING);
    std::vector<std::string> keys;

    for (int i = 0; i < 13; ++i) {
        keys.push_back("key_" + std::to_string(i));
        BOOST_REQUIRE(table.hashInsertInt(keys.back(), i));
    }

    BOOST_CHECK_EQUAL(table.getSize(), 13u);
    BOOST_CHECK(table.getCollisions() > 0u);

    int value = -1;
    BOOST_CHECK(!table.hashFindInt("not_present", value));
    BOOST_CHECK(!table.hashInsertInt("overflow", 100));
    BOOST_CHECK(!table.hashErase("not_present"));

    BOOST_REQUIRE(table.hashErase(keys[0]));
    BOOST_CHECK(table.hashInsertInt("replacement", 200));
    BOOST_CHECK_EQUAL(table.getSize(), 13u);

    BOOST_REQUIRE(table.hashFindInt("replacement", value));
    BOOST_CHECK_EQUAL(value, 200);
}

BOOST_AUTO_TEST_CASE(HashTable_DoubleHashingAndPointerValues) {
    HashTable table(2, DOUBLE_HASHING);
    BOOST_CHECK_EQUAL(table.getCapacity(), 17u);

    BOOST_CHECK(table.hashInsertInt("first", 1));
    BOOST_CHECK(table.hashInsertInt("second", 2));

    int value = -1;
    BOOST_REQUIRE(table.hashFindInt("first", value));
    BOOST_CHECK_EQUAL(value, 1);

    int stored = 42;
    BOOST_CHECK(table.hashInsert("pointer", &stored));

    void* result = nullptr;
    BOOST_REQUIRE(table.hashFind("pointer", result));
    BOOST_REQUIRE(result != nullptr);
    BOOST_CHECK_EQUAL(*static_cast<int*>(result), 42);

    HashTable zeroExpected(0, LINEAR_PROBING);
    BOOST_CHECK_EQUAL(zeroExpected.getCapacity(), 11u);
    BOOST_CHECK(zeroExpected.isEmpty());
}

BOOST_AUTO_TEST_CASE(HashTable_BinarySerialization) {
    const std::string filename = "hash_table_boost_test.bin";

    HashTable original(4, DOUBLE_HASHING);
    BOOST_REQUIRE(original.hashInsertInt("first", 123));
    BOOST_REQUIRE(original.hashInsertInt("key with space", -7));
    BOOST_REQUIRE(original.hashInsertInt("zero", 0));
    BOOST_REQUIRE(original.saveBinary(filename));

    HashTable loaded(1, LINEAR_PROBING);
    BOOST_REQUIRE(loaded.loadBinary(filename));

    int value = 0;
    BOOST_CHECK(loaded.hashFindInt("first", value));
    BOOST_CHECK_EQUAL(value, 123);

    BOOST_CHECK(loaded.hashFindInt("key with space", value));
    BOOST_CHECK_EQUAL(value, -7);

    BOOST_CHECK(loaded.hashFindInt("zero", value));
    BOOST_CHECK_EQUAL(value, 0);

    BOOST_CHECK(!loaded.hashFindInt("missing", value));

    std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE(HashTable_ReusesOneOfSeveralDeletedSlots) {
    HashTable table(1, LINEAR_PROBING);
    std::vector<std::string> keys;

    for (int i = 0; i < 13; ++i) {
        keys.push_back("deleted_test_" + std::to_string(i));
        BOOST_REQUIRE(table.hashInsertInt(keys.back(), i));
    }

    BOOST_REQUIRE(table.hashErase(keys[0]));
    BOOST_REQUIRE(table.hashErase(keys[1]));

    BOOST_CHECK(table.hashInsertInt("replacement_after_two_deletes", 99));

    int value = -1;
    BOOST_REQUIRE(
        table.hashFindInt("replacement_after_two_deletes", value));
    BOOST_CHECK_EQUAL(value, 99);
    BOOST_CHECK_EQUAL(table.getSize(), 12u);
}

// ============================================================
// Общие проверки ошибок открытия файлов
// ============================================================

BOOST_AUTO_TEST_CASE(FileSave_FailsWhenPathDoesNotExist) {
    const char* badPath =
        "/directory_that_does_not_exist_12345/test.txt";

    ListF listF;
    BOOST_CHECK(!listF.saveListFToFile(badPath));

    ListD listD;
    BOOST_CHECK(!listD.saveListDToFile(badPath));

    Queue queue;
    BOOST_CHECK(!queue.saveQueueToFile(badPath));

    Stack stack;
    BOOST_CHECK(!stack.saveStackToFile(badPath));

    AVLTree tree;
    BOOST_CHECK(!tree.saveTreeToFile(badPath));
}
