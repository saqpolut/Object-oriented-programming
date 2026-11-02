#include "AVL_Tree.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cstdint>
#include <functional>

//статические хелперы
unsigned char AVLTree::getHeightT(NodeT* node) {
    return node ? node->height : 0;
}

int AVLTree::getBalanceFactorT(NodeT* node) {
    return node ? getHeightT(node->left) - getHeightT(node->right) : 0;
}

void AVLTree::fixHeightT(NodeT* node) {
    if (!node){
        return;
    }
    unsigned char hL = getHeightT(node->left);
    unsigned char hR = getHeightT(node->right);
    node->height = (hL > hR ? hL : hR) + 1;
}

NodeT* AVLTree::rotateRightT(NodeT* node) {
    NodeT* tmp = node->left;
    node->left = tmp->right;
    tmp->right = node;
    fixHeightT(node);
    fixHeightT(tmp);
    return tmp;
}

NodeT* AVLTree::rotateLeftT(NodeT* node) {
    NodeT* tmp = node->right;
    node->right = tmp->left;
    tmp->left = node;
    fixHeightT(node);
    fixHeightT(tmp);
    return tmp;
}

NodeT* AVLTree::insertT(NodeT* root, int key) {
    if (!root){
        return new NodeT(key);
    }

    if (key < root->key){
        root->left = insertT(root->left, key);
    } else if (key > root->key){
        root->right = insertT(root->right, key);
    } else{
        return root; // дубликаты не добавляем
    }

    fixHeightT(root);
    int balance = getBalanceFactorT(root);

    if (balance == 2){
        if (getBalanceFactorT(root->left) >= 0){
            return rotateRightT(root);
        } else{
            root->left = rotateLeftT(root->left);
            return rotateRightT(root);
        }
    }

    if (balance == -2){
        if (getBalanceFactorT(root->right) <= 0){
            return rotateLeftT(root);
        } else{
            root->right = rotateRightT(root->right);
            return rotateLeftT(root);
        }
    }

    return root;
}

NodeT* AVLTree::removeT(NodeT* root, int key) {
    if (!root){
        return nullptr;
    }

    if (key < root->key){
        root->left = removeT(root->left, key);
    } else if (key > root->key){
        root->right = removeT(root->right, key);
    } else{
        if (!root->left){
            NodeT* tmp = root->right;
            delete root;
            return tmp;
        } else if (!root->right){
            NodeT* tmp = root->left;
            delete root;
            return tmp;
        }

        NodeT* minNode = findMinT(root->right);
        root->key = minNode->key;
        root->right = removeT(root->right, minNode->key);
    }

    if (!root){
        return nullptr;
    }

    fixHeightT(root);
    int balance = getBalanceFactorT(root);

    if (balance == 2){
        if (getBalanceFactorT(root->left) >= 0){
            return rotateRightT(root);
        } else{
            root->left = rotateLeftT(root->left);
            return rotateRightT(root);
        }
    }

    if (balance == -2){
        if (getBalanceFactorT(root->right) <= 0){
            return rotateLeftT(root);
        } else{
            root->right = rotateRightT(root->right);
            return rotateLeftT(root);
        }
    }

    return root;
}

NodeT* AVLTree::findT(NodeT* root, int key) {
    if (!root){
        return nullptr;
    }
    if (key == root->key){
        return root;
    }
    if (key < root->key){
        return findT(root->left, key);
    }
    return findT(root->right, key);
}

NodeT* AVLTree::findWithStepsT(NodeT* root, int key, int& steps) {
    steps = 0;
    while (root != nullptr){
        ++steps;
        if (key == root->key){
            return root;
        }
        root = (key < root->key) ? root->left : root->right;
    }
    return nullptr;
}

NodeT* AVLTree::findMinT(NodeT* node) {
    if (!node){
        return nullptr;
    }
    while (node->left != nullptr){
        node = node->left;
    }
    return node;
}

void AVLTree::printInOrderT(NodeT* root) {
    if (!root){
        return;
    }
    printInOrderT(root->left);
    std::cout << root->key << " ";
    printInOrderT(root->right);
}

void AVLTree::printPreOrderT(NodeT* root) {
    if (!root){
        return;
    }
    std::cout << root->key << " ";
    printPreOrderT(root->left);
    printPreOrderT(root->right);
}

void AVLTree::printPostOrderT(NodeT* root) {
    if (!root){
        return;
    }
    printPostOrderT(root->left);
    printPostOrderT(root->right);
    std::cout << root->key << " ";
}

static void printLevelT(NodeT* root, int level) {
    if (!root){
        return;
    }
    if (level == 1){
        std::cout << root->key << " "; 
        return; 
    }
    printLevelT(root->left,  level - 1);
    printLevelT(root->right, level - 1);
}

void AVLTree::printLevelOrderT(NodeT* root) {
    int h = getHeightT(root);
    for (int lvl = 1; lvl <= h; ++lvl){
        printLevelT(root, lvl);
    }
}

int AVLTree::countNodesT(NodeT* root) {
    if (!root){
        return 0;
    }
    return 1 + countNodesT(root->left) + countNodesT(root->right);
}

void AVLTree::printTreeT(NodeT* root, int level) {
    if (!root){
        return;
    }
    printTreeT(root->right, level + 1);
    for (int i = 0; i < level; ++i){
        std::cout << "    ";
    }
    std::cout << root->key << std::endl;
    printTreeT(root->left, level + 1);
}

void AVLTree::destroyTreeT(NodeT* root) {
    if (!root){
        return;
    }
    destroyTreeT(root->left);
    destroyTreeT(root->right);
    delete root;
}

void AVLTree::saveTreeHelper(NodeT* root, std::ofstream& out) {
    if (!root){
        out << "# ";
        return;
    }
    out << root->key << " ";
    saveTreeHelper(root->left, out);
    saveTreeHelper(root->right, out);
}

NodeT* AVLTree::loadTreeHelper(std::ifstream& in) {
    std::string tok;
    if (!(in >> tok)){
        return nullptr;
    }
    if (tok == "#"){
        return nullptr;
    }

    int key;
    try {
        key = stoi(tok);
    }
    catch (const std::exception&) {
        return nullptr;
    }

    NodeT* node = new NodeT(key);
    node->left  = loadTreeHelper(in);
    node->right = loadTreeHelper(in);
    fixHeightT(node);
    return node;
}

//публичный интерфейс
AVLTree::AVLTree() : root(nullptr) {}

AVLTree::~AVLTree() {
    destroyTreeT(root);
}

void AVLTree::insertT(int key) {
    root = insertT(root, key);
}

void AVLTree::removeT(int key) {
    root = removeT(root, key);
}

NodeT* AVLTree::findT(int key) const {
    return findT(root, key);
}

NodeT* AVLTree::findWithStepsT(int key, int& steps) const {
    return findWithStepsT(root, key, steps);
}

NodeT* AVLTree::findMinT() const {
    return findMinT(root);
}

void AVLTree::printInOrderT() const {
    printInOrderT(root);
}
void AVLTree::printPreOrderT() const {
    printPreOrderT(root);
}
void AVLTree::printPostOrderT() const {
    printPostOrderT(root);
}
void AVLTree::printLevelOrderT() const { 
    printLevelOrderT(root);
}
int  AVLTree::countNodesT() const {
    return countNodesT(root);
}
void AVLTree::printTreeT() const {
    printTreeT(root, 0);
}

unsigned char AVLTree::getHeightT() const {
    return getHeightT(root);
}
bool AVLTree::isEmpty() const {
    return root == nullptr;
}

void AVLTree::printTreeInfoT(const char* title) const {
    std::cout << "\n" << title << "\n";
    if (!root) {
        std::cout << "(дерево пусто)\n"; 
        return;
    }

    std::cout << "По возрастанию (in-order): ";  printInOrderT(root);    
    std::cout << "\n";
    std::cout << "Прямой обход (pre-order): ";   printPreOrderT(root);   
    std::cout << "\n";
    std::cout << "Обратный обход (post-order): "; printPostOrderT(root);  
    std::cout << "\n";
    std::cout << "В ширину (по уровням): ";      printLevelOrderT(root); 
    std::cout << "\n";
    std::cout << "Высота дерева: " << (int)getHeightT(root) << "\n";
    std::cout << "Количество узлов: " << countNodesT(root) << "\n";
    std::cout << "Структура (повёрнуто на 90 градусов):\n";
    printTreeT(root, 0);
}

bool AVLTree::saveTreeToFile(const char* filename) const {
    std::ofstream out(filename);
    if (!out){
        return false;
    }
    saveTreeHelper(root, out);
    out << "\n";
    return true;
}

bool AVLTree::loadTreeFromFile(const char* filename) {
    std::ifstream in(filename);
    if (!in){
        return false;
    }

    destroyTreeT(root);
    root = loadTreeHelper(in);
    return true;
}

bool AVLTree::saveTreeBinary(const char* filename) const {
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        return false;
    }

    // Сигнатура бинарного формата: "AVLBIN01"
    const char magic[] = {'A', 'V', 'L', 'B', 'I', 'N', '0', '1'};
    out.write(magic, sizeof(magic));

    // Для каждого узла записываем маркер, затем ключ.
    // Маркер 0 — пустой узел, маркер 1 — существующий узел.
    std::function<bool(const NodeT*)> writeNode =
        [&](const NodeT* node) -> bool {
            const std::uint8_t marker = node == nullptr ? 0 : 1;
            out.write(reinterpret_cast<const char*>(&marker), sizeof(marker));

            if (node == nullptr) {
                return static_cast<bool>(out);
            }

            const std::int32_t key = static_cast<std::int32_t>(node->key);
            out.write(reinterpret_cast<const char*>(&key), sizeof(key));

            return out &&
                   writeNode(node->left) &&
                   writeNode(node->right);
        };

    return writeNode(root) && static_cast<bool>(out);
}

bool AVLTree::loadTreeBinary(const char* filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        return false;
    }

    const char expectedMagic[] = {'A', 'V', 'L', 'B', 'I', 'N', '0', '1'};
    char actualMagic[sizeof(expectedMagic)]{};

    in.read(actualMagic, sizeof(actualMagic));
    if (!in || !std::equal(
            std::begin(expectedMagic), std::end(expectedMagic),
            std::begin(actualMagic))) {
        return false;
    }

    std::size_t nodeCount = 0;
    constexpr std::size_t maxNodes = 1'000'000;
    constexpr int maxDepth = 128;

    std::function<NodeT*(int, bool&)> readNode =
        [&](int depth, bool& ok) -> NodeT* {
            if (depth > maxDepth || nodeCount > maxNodes) {
                ok = false;
                return nullptr;
            }

            std::uint8_t marker = 0;
            in.read(reinterpret_cast<char*>(&marker), sizeof(marker));

            if (!in || marker > 1) {
                ok = false;
                return nullptr;
            }

            if (marker == 0) {
                return nullptr;
            }

            std::int32_t key = 0;
            in.read(reinterpret_cast<char*>(&key), sizeof(key));

            if (!in) {
                ok = false;
                return nullptr;
            }

            ++nodeCount;
            NodeT* node = new NodeT(static_cast<int>(key));

            node->left = readNode(depth + 1, ok);
            if (!ok) {
                delete node->left;
                delete node;
                return nullptr;
            }

            node->right = readNode(depth + 1, ok);
            if (!ok) {
                delete node->left;
                delete node->right;
                delete node;
                return nullptr;
            }

            const auto leftHeight =
                node->left == nullptr ? 0 : node->left->height;
            const auto rightHeight =
                node->right == nullptr ? 0 : node->right->height;
            node->height = static_cast<unsigned char>(
                1 + std::max(leftHeight, rightHeight));

            return node;
        };

    bool ok = true;
    NodeT* loadedRoot = readNode(0, ok);

    // Не принимаем файл с ошибочными или лишними данными.
    if (!ok || in.peek() != std::char_traits<char>::eof()) {
        std::function<void(NodeT*)> deleteTree =
            [&](NodeT* node) {
                if (node == nullptr) {
                    return;
                }
                deleteTree(node->left);
                deleteTree(node->right);
                delete node;
            };

        deleteTree(loadedRoot);
        return false;
    }

    // Заменяем старое дерево только после успешной загрузки.
    std::function<void(NodeT*)> deleteOldTree =
        [&](NodeT* node) {
            if (node == nullptr) {
                return;
            }
            deleteOldTree(node->left);
            deleteOldTree(node->right);
            delete node;
        };

    deleteOldTree(root);
    root = loadedRoot;
    return true;
}
