#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <fstream>

// Структура узла AVL-дерева
struct NodeT {
    int key;
    NodeT* left;
    NodeT* right;
    unsigned char height;
    NodeT(int k) { key = k; left = nullptr; right = nullptr; height = 1; }
};

class AVLTree {
private:
    NodeT* root;

    //статические хелперы (не зависят от this)
    static unsigned char getHeightT(NodeT* node);
    static int getBalanceFactorT(NodeT* node);
    static void fixHeightT(NodeT* node);

    static NodeT* rotateRightT(NodeT* node);
    static NodeT* rotateLeftT(NodeT* node);

    static NodeT* insertT(NodeT* root, int key);
    static NodeT* removeT(NodeT* root, int key);

    static NodeT* findT(NodeT* root, int key);
    static NodeT* findWithStepsT(NodeT* root, int key, int& steps);
    static NodeT* findMinT(NodeT* node);

    static void printInOrderT(NodeT* root);
    static void printPreOrderT(NodeT* root);
    static void printPostOrderT(NodeT* root);
    static void printLevelOrderT(NodeT* root);
    static int  countNodesT(NodeT* root);
    static void printTreeT(NodeT* root, int level);

    static void destroyTreeT(NodeT* root);

    static void saveTreeHelper(NodeT* root, std::ofstream& out);
    static NodeT* loadTreeHelper(std::ifstream& in);

public:
    AVLTree();
    ~AVLTree();

    //запрет копирования - защита от двойного delete
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    //публичный интерфейс
    void insertT(int key);
    void removeT(int key);

    NodeT* findT(int key) const;
    NodeT* findWithStepsT(int key, int& steps) const;
    NodeT* findMinT() const;

    void printInOrderT() const;
    void printPreOrderT() const;
    void printPostOrderT() const;
    void printLevelOrderT() const;
    int  countNodesT() const;
    void printTreeT() const;
    void printTreeInfoT(const char* title) const;

    unsigned char getHeightT() const;
    bool isEmpty() const;

    //сериализация
    bool saveTreeToFile(const char* filename) const;
    bool loadTreeFromFile(const char* filename);

    bool saveTreeBinary(const char* filename) const;
    bool loadTreeBinary(const char* filename);

};

#endif //AVL_TREE_H