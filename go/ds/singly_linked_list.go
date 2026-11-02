package ds

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"io"
	"os"
)

const (
	maxFListItems      = 1_000_000
	maxFListStringSize = 16 * 1024 * 1024
)

type NodeF struct {
	Data string
	Next *NodeF
}

type FList struct {
	head *NodeF
	tail *NodeF
}

func NewFList() *FList {
	return &FList{}
}

// Проверка и размер
func (list *FList) IsEmptyF() bool {
	return list == nil || list.head == nil
}

func (list *FList) SizeF() int {
	if list == nil {
		return 0
	}

	count := 0
	for current := list.head; current != nil; current = current.Next {
		count++
	}
	return count
}

// Поиск
func (list *FList) FindNodeF(value string) *NodeF {
	if list == nil {
		return nil
	}

	for current := list.head; current != nil; current = current.Next {
		if current.Data == value {
			return current
		}
	}
	return nil
}

func (list *FList) GetAtF(index int) *NodeF {
	if list == nil || index < 0 {
		return nil
	}

	current := list.head
	for i := 0; current != nil && i < index; i++ {
		current = current.Next
	}
	return current
}

// Добавление
func (list *FList) PushFrontF(value string) {
	node := &NodeF{
		Data: value,
		Next: list.head,
	}
	list.head = node

	if list.tail == nil {
		list.tail = node
	}
}

func (list *FList) PushBackF(value string) {
	node := &NodeF{Data: value}

	if list.tail == nil {
		list.head = node
		list.tail = node
		return
	}

	list.tail.Next = node
	list.tail = node
}

func (list *FList) InsertAfterF(target, value string) bool {
	targetNode := list.FindNodeF(target)
	if targetNode == nil {
		return false
	}

	node := &NodeF{
		Data: value,
		Next: targetNode.Next,
	}
	targetNode.Next = node

	if list.tail == targetNode {
		list.tail = node
	}
	return true
}

func (list *FList) InsertBeforeF(target, value string) bool {
	if list == nil || list.head == nil {
		return false
	}

	if list.head.Data == target {
		list.PushFrontF(value)
		return true
	}

	previous := list.head
	for previous.Next != nil && previous.Next.Data != target {
		previous = previous.Next
	}

	if previous.Next == nil {
		return false
	}

	node := &NodeF{
		Data: value,
		Next: previous.Next,
	}
	previous.Next = node
	return true
}

// Удаление
func (list *FList) PopFrontF() bool {
	if list == nil || list.head == nil {
		return false
	}

	list.head = list.head.Next
	if list.head == nil {
		list.tail = nil
	}
	return true
}

func (list *FList) PopBackF() bool {
	if list == nil || list.head == nil {
		return false
	}

	if list.head == list.tail {
		list.head = nil
		list.tail = nil
		return true
	}

	previous := list.head
	for previous.Next != list.tail {
		previous = previous.Next
	}

	previous.Next = nil
	list.tail = previous
	return true
}

func (list *FList) RemoveAfterF(target string) bool {
	targetNode := list.FindNodeF(target)
	if targetNode == nil || targetNode.Next == nil {
		return false
	}

	removed := targetNode.Next
	targetNode.Next = removed.Next

	if list.tail == removed {
		list.tail = targetNode
	}
	return true
}

func (list *FList) RemoveBeforeF(target string) bool {
	if list == nil || list.head == nil || list.head.Data == target {
		return false
	}

	// Узел перед target — это голова.
	if list.head.Next != nil && list.head.Next.Data == target {
		return list.PopFrontF()
	}

	// Ищем узел, стоящий за два шага до target.
	previousPrevious := list.head
	for previousPrevious.Next != nil &&
		previousPrevious.Next.Next != nil &&
		previousPrevious.Next.Next.Data != target {
		previousPrevious = previousPrevious.Next
	}

	if previousPrevious.Next == nil || previousPrevious.Next.Next == nil {
		return false
	}

	removed := previousPrevious.Next
	previousPrevious.Next = removed.Next
	return true
}

func (list *FList) RemoveByValueF(value string) bool {
	if list == nil || list.head == nil {
		return false
	}

	if list.head.Data == value {
		return list.PopFrontF()
	}

	previous := list.head
	for previous.Next != nil && previous.Next.Data != value {
		previous = previous.Next
	}

	if previous.Next == nil {
		return false
	}

	removed := previous.Next
	previous.Next = removed.Next

	if list.tail == removed {
		list.tail = previous
	}
	return true
}

// Поиск индекса. Если значение не найдено, возвращается -1.
func (list *FList) SearchF(value string) int {
	if list == nil {
		return -1
	}

	index := 0
	for current := list.head; current != nil; current = current.Next {
		if current.Data == value {
			return index
		}
		index++
	}
	return -1
}

// Вывод
func (list *FList) PrintForwardF(w io.Writer) {
	if list.IsEmptyF() {
		fmt.Fprintln(w, "Список пуст")
		return
	}

	for current := list.head; current != nil; current = current.Next {
		fmt.Fprint(w, current.Data, " -> ")
	}
	fmt.Fprintln(w, "nullptr")
}

func printBackwardRecF(w io.Writer, node *NodeF) {
	if node == nil {
		return
	}

	printBackwardRecF(w, node.Next)
	fmt.Fprint(w, node.Data, " ")
}

func (list *FList) PrintBackwardF(w io.Writer) {
	if list.IsEmptyF() {
		fmt.Fprintln(w, "Список пуст")
		return
	}

	printBackwardRecF(w, list.head)
	fmt.Fprintln(w)
}

func (list *FList) PrintIndexedF(w io.Writer) {
	if list.IsEmptyF() {
		fmt.Fprintln(w, "Список пуст")
		return
	}

	for index, current := 0, list.head; current != nil; index, current = index+1, current.Next {
		fmt.Fprintf(w, "[%d] %s\n", index, current.Data)
	}
}

// Текстовая сериализация.
func (list *FList) SaveListFToFile(filename string) bool {
	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	writer := bufio.NewWriter(file)
	for current := list.head; current != nil; current = current.Next {
		if _, err := fmt.Fprint(writer, current.Data, " "); err != nil {
			_ = file.Close()
			return false
		}
	}

	if _, err := fmt.Fprintln(writer); err != nil {
		_ = file.Close()
		return false
	}
	if err := writer.Flush(); err != nil {
		_ = file.Close()
		return false
	}

	return file.Close() == nil
}

func (list *FList) LoadListFFromFile(filename string) bool {
	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	temporary := NewFList()

	scanner := bufio.NewScanner(file)
	scanner.Split(bufio.ScanWords)
	scanner.Buffer(make([]byte, 4096), maxFListStringSize)

	for scanner.Scan() {
		temporary.PushBackF(scanner.Text())
	}
	if scanner.Err() != nil {
		return false
	}

	// Исходный список меняется только после успешного чтения файла.
	list.head = temporary.head
	list.tail = temporary.tail
	return true
}

// Бинарный формат: сигнатура LISTF01, количество элементов,
// затем для каждой строки её длина и содержимое.
func (list *FList) SaveListFBinary(filename string) bool {
	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	magic := [8]byte{'L', 'I', 'S', 'T', 'F', '0', '1', 0}
	writeErr := writeBinaryFList(file, list, magic)

	closeErr := file.Close()
	return writeErr == nil && closeErr == nil
}

func writeBinaryFList(file *os.File, list *FList, magic [8]byte) error {
	if _, err := file.Write(magic[:]); err != nil {
		return err
	}

	if err := binary.Write(file, binary.LittleEndian, uint64(list.SizeF())); err != nil {
		return err
	}

	for current := list.head; current != nil; current = current.Next {
		if len(current.Data) > maxFListStringSize {
			return fmt.Errorf("строка превышает допустимый размер")
		}

		if err := binary.Write(
			file,
			binary.LittleEndian,
			uint64(len(current.Data)),
		); err != nil {
			return err
		}

		if _, err := io.WriteString(file, current.Data); err != nil {
			return err
		}
	}

	return nil
}

func (list *FList) LoadListFBinary(filename string) bool {
	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	var actualMagic [8]byte
	if _, err := io.ReadFull(file, actualMagic[:]); err != nil {
		return false
	}

	expectedMagic := [8]byte{'L', 'I', 'S', 'T', 'F', '0', '1', 0}
	if actualMagic != expectedMagic {
		return false
	}

	var count uint64
	if err := binary.Read(file, binary.LittleEndian, &count); err != nil {
		return false
	}
	if count > maxFListItems {
		return false
	}

	temporary := NewFList()

	for i := uint64(0); i < count; i++ {
		var length uint64
		if err := binary.Read(file, binary.LittleEndian, &length); err != nil {
			return false
		}
		if length > maxFListStringSize {
			return false
		}

		value := make([]byte, int(length))
		if _, err := io.ReadFull(file, value); err != nil {
			return false
		}

		temporary.PushBackF(string(value))
	}

	// Отклоняем файл, если после сериализованных данных есть лишние байты.
	var extra [1]byte
	if n, err := file.Read(extra[:]); n != 0 || err != io.EOF {
		return false
	}

	// Список заменяется только после полного успешного чтения.
	list.head = temporary.head
	list.tail = temporary.tail
	return true
}
