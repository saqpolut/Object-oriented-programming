#include <benchmark/benchmark.h>
#include <string>

#include "massiv.h"
#include "singlyLinkedList.h"
#include "doublyLinkedList.h"
#include "stack.h"
#include "queue.h"
#include "hashTable.h"
#include "AVL_Tree.h"

//	Создаётся Massiv, в него добавляются строки
static void BM_MassivPush(benchmark::State& state) {
    for (auto _ : state) {
        Massiv m;
        for (int i = 0; i < state.range(0); ++i) {
            m.addToEnd("val");
        }
    }
}
BENCHMARK(BM_MassivPush)->Range(64, 2048);

//Создаётся односвязный список, элементы добавляются в конец
static void BM_ListFPushBack(benchmark::State& state) {
    for (auto _ : state) {
        ListF lf;
        for (int i = 0; i < state.range(0); ++i) {
            lf.pushBackF("val");
        }
    }
}
BENCHMARK(BM_ListFPushBack)->Range(64, 2048);

//Элементы добавляются в стек, затем извлекаются
static void BM_StackPushPop(benchmark::State& state) {
    for (auto _ : state) {
        Stack st;
        for (int i = 0; i < state.range(0); ++i) {
            st.pushStack("val");
        }
        std::string s;
        while (!st.isStackEmpty()) {
            st.popStack(s);
        }
    }
}
BENCHMARK(BM_StackPushPop)->Range(64, 2048);

//Элементы добавляются в очередь, затем извлекаются
static void BM_QueuePushPop(benchmark::State& state) {
    for (auto _ : state) {
        Queue q;
        for (int i = 0; i < state.range(0); ++i) {
            q.pushQueue("val");
        }
        std::string s;
        while (!q.isQueueEmpty()) {
            q.popQueue(s);
        }
    }
}
BENCHMARK(BM_QueuePushPop)->Range(64, 2048);

//В AVL-дерево вставляются целые числа
static void BM_AVLTreeInsert(benchmark::State& state) {
    for (auto _ : state) {
        AVLTree tree;
        for (int i = 0; i < state.range(0); ++i) {
            tree.insertT(i);
        }
    }
}
BENCHMARK(BM_AVLTreeInsert)->Range(64, 2048);

//В хеш-таблицу вставляются пары «ключ — целое значение»
static void BM_HashTableInsert(benchmark::State& state) {
    for (auto _ : state) {
        HashTable ht(1, LINEAR_PROBING);
        for (int i = 0; i < state.range(0); ++i) {
            ht.hashInsertInt("k_" + std::to_string(i), i);
        }
    }
}
BENCHMARK(BM_HashTableInsert)->Range(64, 2048);

BENCHMARK_MAIN();
