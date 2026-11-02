package ds

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"
)

const maxAVLDepth = 128

// Узел AVL-дерева.
type NodeT struct {
	Key    int
	Left   *NodeT
	Right  *NodeT
	Height uint8
}

// AVLTree — объект дерева и указатель на его корень.
type AVLTree struct {
	root *NodeT
}

func NewAVLTree() *AVLTree {
	return &AVLTree{}
}

// Внутренние функции для высоты, балансировки и вращений.
func heightT(node *NodeT) uint8 {
	if node == nil {
		return 0
	}
	return node.Height
}

func balanceFactorT(node *NodeT) int {
	if node == nil {
		return 0
	}
	return int(heightT(node.Left)) - int(heightT(node.Right))
}

func fixHeightT(node *NodeT) {
	if node == nil {
		return
	}

	leftHeight := heightT(node.Left)
	rightHeight := heightT(node.Right)
	if leftHeight > rightHeight {
		node.Height = leftHeight + 1
	} else {
		node.Height = rightHeight + 1
	}
}

func rotateRightT(node *NodeT) *NodeT {
	newRoot := node.Left
	node.Left = newRoot.Right
	newRoot.Right = node

	fixHeightT(node)
	fixHeightT(newRoot)
	return newRoot
}

func rotateLeftT(node *NodeT) *NodeT {
	newRoot := node.Right
	node.Right = newRoot.Left
	newRoot.Left = node

	fixHeightT(node)
	fixHeightT(newRoot)
	return newRoot
}

func balanceT(node *NodeT) *NodeT {
	if node == nil {
		return nil
	}

	fixHeightT(node)
	balance := balanceFactorT(node)

	if balance == 2 {
		if balanceFactorT(node.Left) < 0 {
			node.Left = rotateLeftT(node.Left)
		}
		return rotateRightT(node)
	}

	if balance == -2 {
		if balanceFactorT(node.Right) > 0 {
			node.Right = rotateRightT(node.Right)
		}
		return rotateLeftT(node)
	}

	return node
}

// Вставка. Повторяющиеся ключи не добавляются.
func insertNodeT(node *NodeT, key int) *NodeT {
	if node == nil {
		return &NodeT{Key: key, Height: 1}
	}

	if key < node.Key {
		node.Left = insertNodeT(node.Left, key)
	} else if key > node.Key {
		node.Right = insertNodeT(node.Right, key)
	} else {
		return node
	}

	return balanceT(node)
}

func (tree *AVLTree) InsertT(key int) {
	if tree != nil {
		tree.root = insertNodeT(tree.root, key)
	}
}

// Удаление
func findMinNodeT(node *NodeT) *NodeT {
	if node == nil {
		return nil
	}
	for node.Left != nil {
		node = node.Left
	}
	return node
}

func removeMinT(node *NodeT) *NodeT {
	if node.Left == nil {
		return node.Right
	}
	node.Left = removeMinT(node.Left)
	return balanceT(node)
}

func removeNodeT(node *NodeT, key int) *NodeT {
	if node == nil {
		return nil
	}

	if key < node.Key {
		node.Left = removeNodeT(node.Left, key)
	} else if key > node.Key {
		node.Right = removeNodeT(node.Right, key)
	} else {
		left := node.Left
		right := node.Right

		if right == nil {
			return left
		}

		minimum := findMinNodeT(right)
		minimum.Right = removeMinT(right)
		minimum.Left = left
		return balanceT(minimum)
	}

	return balanceT(node)
}

func (tree *AVLTree) RemoveT(key int) {
	if tree != nil {
		tree.root = removeNodeT(tree.root, key)
	}
}

// Поиск
func findNodeT(node *NodeT, key int) *NodeT {
	for node != nil {
		if key == node.Key {
			return node
		}
		if key < node.Key {
			node = node.Left
		} else {
			node = node.Right
		}
	}
	return nil
}

func (tree *AVLTree) FindT(key int) *NodeT {
	if tree == nil {
		return nil
	}
	return findNodeT(tree.root, key)
}

func (tree *AVLTree) FindWithStepsT(key int) (*NodeT, int) {
	if tree == nil {
		return nil, 0
	}

	steps := 0
	for node := tree.root; node != nil; {
		steps++
		if key == node.Key {
			return node, steps
		}
		if key < node.Key {
			node = node.Left
		} else {
			node = node.Right
		}
	}
	return nil, steps
}

func (tree *AVLTree) FindMinT() *NodeT {
	if tree == nil {
		return nil
	}
	return findMinNodeT(tree.root)
}

// Свойства дерева
func (tree *AVLTree) CountNodesT() int {
	if tree == nil {
		return 0
	}

	count := 0
	nodes := []*NodeT{tree.root}
	for len(nodes) > 0 {
		node := nodes[len(nodes)-1]
		nodes = nodes[:len(nodes)-1]
		if node == nil {
			continue
		}

		count++
		nodes = append(nodes, node.Left, node.Right)
	}
	return count
}

func (tree *AVLTree) GetHeightT() uint8 {
	if tree == nil {
		return 0
	}
	return heightT(tree.root)
}

func (tree *AVLTree) IsEmpty() bool {
	return tree == nil || tree.root == nil
}

// Обходы и печать
func printInOrderNodeT(w io.Writer, node *NodeT) {
	if node == nil {
		return
	}
	printInOrderNodeT(w, node.Left)
	fmt.Fprint(w, node.Key, " ")
	printInOrderNodeT(w, node.Right)
}

func (tree *AVLTree) PrintInOrderT(w io.Writer) {
	if tree != nil {
		printInOrderNodeT(w, tree.root)
	}
}

func printPreOrderNodeT(w io.Writer, node *NodeT) {
	if node == nil {
		return
	}
	fmt.Fprint(w, node.Key, " ")
	printPreOrderNodeT(w, node.Left)
	printPreOrderNodeT(w, node.Right)
}

func (tree *AVLTree) PrintPreOrderT(w io.Writer) {
	if tree != nil {
		printPreOrderNodeT(w, tree.root)
	}
}

func printPostOrderNodeT(w io.Writer, node *NodeT) {
	if node == nil {
		return
	}
	printPostOrderNodeT(w, node.Left)
	printPostOrderNodeT(w, node.Right)
	fmt.Fprint(w, node.Key, " ")
}

func (tree *AVLTree) PrintPostOrderT(w io.Writer) {
	if tree != nil {
		printPostOrderNodeT(w, tree.root)
	}
}

func (tree *AVLTree) PrintLevelOrderT(w io.Writer) {
	if tree == nil || tree.root == nil {
		return
	}

	queue := []*NodeT{tree.root}
	for len(queue) > 0 {
		node := queue[0]
		queue = queue[1:]

		fmt.Fprint(w, node.Key, " ")
		if node.Left != nil {
			queue = append(queue, node.Left)
		}
		if node.Right != nil {
			queue = append(queue, node.Right)
		}
	}
}

func printTreeNodeT(w io.Writer, node *NodeT, level int) {
	if node == nil {
		return
	}

	printTreeNodeT(w, node.Right, level+1)
	for i := 0; i < level; i++ {
		fmt.Fprint(w, "    ")
	}
	fmt.Fprintln(w, node.Key)
	printTreeNodeT(w, node.Left, level+1)
}

func (tree *AVLTree) PrintTreeT(w io.Writer) {
	if tree != nil {
		printTreeNodeT(w, tree.root, 0)
	}
}

func (tree *AVLTree) PrintTreeInfoT(w io.Writer, title string) {
	fmt.Fprintf(w, "\n%s\n", title)
	if tree == nil || tree.root == nil {
		fmt.Fprintln(w, "(дерево пусто)")
		return
	}

	fmt.Fprint(w, "По возрастанию (in-order): ")
	tree.PrintInOrderT(w)
	fmt.Fprintln(w)

	fmt.Fprint(w, "Прямой обход (pre-order): ")
	tree.PrintPreOrderT(w)
	fmt.Fprintln(w)

	fmt.Fprint(w, "Обратный обход (post-order): ")
	tree.PrintPostOrderT(w)
	fmt.Fprintln(w)

	fmt.Fprint(w, "В ширину (по уровням): ")
	tree.PrintLevelOrderT(w)
	fmt.Fprintln(w)

	fmt.Fprintf(w, "Высота дерева: %d\n", tree.GetHeightT())
	fmt.Fprintf(w, "Количество узлов: %d\n", tree.CountNodesT())
	fmt.Fprintln(w, "Структура (повёрнуто на 90 градусов):")
	tree.PrintTreeT(w)
}

// Текстовая сериализация в формате pre-order:
// ключ левого поддерева правого поддерева; # обозначает пустой узел.
func saveTreeTextT(w io.Writer, node *NodeT) error {
	if node == nil {
		_, err := io.WriteString(w, "# ")
		return err
	}

	if _, err := fmt.Fprintf(w, "%d ", node.Key); err != nil {
		return err
	}
	if err := saveTreeTextT(w, node.Left); err != nil {
		return err
	}
	return saveTreeTextT(w, node.Right)
}

func (tree *AVLTree) SaveTreeToFile(filename string) bool {
	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	writer := bufio.NewWriter(file)
	if tree == nil {
		err = saveTreeTextT(writer, nil)
	} else {
		err = saveTreeTextT(writer, tree.root)
	}
	if err == nil {
		_, err = fmt.Fprintln(writer)
	}
	if err == nil {
		err = writer.Flush()
	}

	closeErr := file.Close()
	return err == nil && closeErr == nil
}

func loadTreeTextNodeT(tokens []string, position *int, depth int) (*NodeT, bool) {
	if depth > maxAVLDepth || *position >= len(tokens) {
		return nil, false
	}

	token := tokens[*position]
	*position++

	if token == "#" {
		return nil, true
	}

	key, err := strconv.Atoi(token)
	if err != nil {
		return nil, false
	}

	node := &NodeT{Key: key, Height: 1}

	left, ok := loadTreeTextNodeT(tokens, position, depth+1)
	if !ok {
		return nil, false
	}
	right, ok := loadTreeTextNodeT(tokens, position, depth+1)
	if !ok {
		return nil, false
	}

	node.Left = left
	node.Right = right
	fixHeightT(node)
	return node, true
}

func (tree *AVLTree) LoadTreeFromFile(filename string) bool {
	data, err := os.ReadFile(filename)
	if err != nil {
		return false
	}

	tokens := strings.Fields(string(data))
	position := 0

	loadedRoot, ok := loadTreeTextNodeT(tokens, &position, 0)
	if !ok || position != len(tokens) {
		return false
	}

	if tree != nil {
		tree.root = loadedRoot
	}
	return true
}

// AVLBIN01, затем маркеры узлов (0 — nil, 1 — узел)
// и для каждого узла int32-ключ, левое и правое поддеревья.
func writeTreeBinaryT(w io.Writer, node *NodeT, depth int) error {
	if depth > maxAVLDepth {
		return fmt.Errorf("превышена допустимая глубина дерева")
	}

	if node == nil {
		return binary.Write(w, binary.LittleEndian, uint8(0))
	}

	if err := binary.Write(w, binary.LittleEndian, uint8(1)); err != nil {
		return err
	}
	if err := binary.Write(w, binary.LittleEndian, int32(node.Key)); err != nil {
		return err
	}
	if err := writeTreeBinaryT(w, node.Left, depth+1); err != nil {
		return err
	}
	return writeTreeBinaryT(w, node.Right, depth+1)
}

func (tree *AVLTree) SaveTreeBinary(filename string) bool {
	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	magic := [8]byte{'A', 'V', 'L', 'B', 'I', 'N', '0', '1'}
	_, err = file.Write(magic[:])
	if err == nil {
		var root *NodeT
		if tree != nil {
			root = tree.root
		}
		err = writeTreeBinaryT(file, root, 0)
	}

	closeErr := file.Close()
	return err == nil && closeErr == nil
}

func readTreeBinaryT(r io.Reader, depth int, count *int) (*NodeT, bool) {
	if depth > maxAVLDepth || *count > 1_000_000 {
		return nil, false
	}

	var marker uint8
	if err := binary.Read(r, binary.LittleEndian, &marker); err != nil {
		return nil, false
	}
	if marker == 0 {
		return nil, true
	}
	if marker != 1 {
		return nil, false
	}

	var key int32
	if err := binary.Read(r, binary.LittleEndian, &key); err != nil {
		return nil, false
	}

	*count++
	node := &NodeT{Key: int(key), Height: 1}

	left, ok := readTreeBinaryT(r, depth+1, count)
	if !ok {
		return nil, false
	}
	right, ok := readTreeBinaryT(r, depth+1, count)
	if !ok {
		return nil, false
	}

	node.Left = left
	node.Right = right
	fixHeightT(node)
	return node, true
}

func (tree *AVLTree) LoadTreeBinary(filename string) bool {
	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	var actualMagic [8]byte
	if _, err := io.ReadFull(file, actualMagic[:]); err != nil {
		return false
	}

	expectedMagic := [8]byte{'A', 'V', 'L', 'B', 'I', 'N', '0', '1'}
	if actualMagic != expectedMagic {
		return false
	}

	count := 0
	loadedRoot, ok := readTreeBinaryT(file, 0, &count)
	if !ok {
		return false
	}

	// Отклоняем файл, если после дерева есть лишние байты.
	var extra [1]byte
	if n, err := file.Read(extra[:]); n != 0 || err != io.EOF {
		return false
	}

	if tree != nil {
		tree.root = loadedRoot
	}
	return true
}
