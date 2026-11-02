package ds

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"io"
	"os"
)

const (
	maxDListItems      = 1_000_000
	maxDListStringSize = 16 * 1024 * 1024
)

// Узел двусвязного списка.
type NodeD struct {
	Data string
	Prev *NodeD
	Next *NodeD
}

// Сам список.
type DList struct {
	head *NodeD
	tail *NodeD
}

func NewDList() *DList {
	return &DList{}
}

// Проверка и размер

func (list *DList) IsEmptyD() bool {
	return list == nil || list.head == nil
}

func (list *DList) SizeD() int {
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

func (list *DList) FindNodeD(value string) *NodeD {
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

func (list *DList) GetAtD(index int) *NodeD {
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

func (list *DList) PushFrontD(value string) {
	node := &NodeD{
		Data: value,
		Next: list.head,
	}

	if list.head != nil {
		list.head.Prev = node
	} else {
		list.tail = node
	}
	list.head = node
}

func (list *DList) PushBackD(value string) {
	node := &NodeD{
		Data: value,
		Prev: list.tail,
	}

	if list.tail != nil {
		list.tail.Next = node
	} else {
		list.head = node
	}
	list.tail = node
}

func (list *DList) InsertAfterD(target, value string) bool {
	targetNode := list.FindNodeD(target)
	if targetNode == nil {
		return false
	}

	node := &NodeD{
		Data: value,
		Prev: targetNode,
		Next: targetNode.Next,
	}

	if targetNode.Next != nil {
		targetNode.Next.Prev = node
	} else {
		list.tail = node
	}
	targetNode.Next = node
	return true
}

func (list *DList) InsertBeforeD(target, value string) bool {
	targetNode := list.FindNodeD(target)
	if targetNode == nil {
		return false
	}

	node := &NodeD{
		Data: value,
		Prev: targetNode.Prev,
		Next: targetNode,
	}

	if targetNode.Prev != nil {
		targetNode.Prev.Next = node
	} else {
		list.head = node
	}
	targetNode.Prev = node
	return true
}

// Удаление

func (list *DList) PopFrontD() bool {
	if list == nil || list.head == nil {
		return false
	}

	list.head = list.head.Next
	if list.head != nil {
		list.head.Prev = nil
	} else {
		list.tail = nil
	}
	return true
}

func (list *DList) PopBackD() bool {
	if list == nil || list.tail == nil {
		return false
	}

	list.tail = list.tail.Prev
	if list.tail != nil {
		list.tail.Next = nil
	} else {
		list.head = nil
	}
	return true
}

func (list *DList) RemoveAfterD(target string) bool {
	targetNode := list.FindNodeD(target)
	if targetNode == nil || targetNode.Next == nil {
		return false
	}

	removed := targetNode.Next
	targetNode.Next = removed.Next

	if removed.Next != nil {
		removed.Next.Prev = targetNode
	} else {
		list.tail = targetNode
	}
	return true
}

func (list *DList) RemoveBeforeD(target string) bool {
	targetNode := list.FindNodeD(target)
	if targetNode == nil || targetNode.Prev == nil {
		return false
	}

	removed := targetNode.Prev
	targetNode.Prev = removed.Prev

	if removed.Prev != nil {
		removed.Prev.Next = targetNode
	} else {
		list.head = targetNode
	}
	return true
}

func (list *DList) RemoveByValueD(value string) bool {
	node := list.FindNodeD(value)
	if node == nil {
		return false
	}

	if node.Prev != nil {
		node.Prev.Next = node.Next
	} else {
		list.head = node.Next
	}

	if node.Next != nil {
		node.Next.Prev = node.Prev
	} else {
		list.tail = node.Prev
	}
	return true
}

// Поиск индекса. Возвращает -1, если значение не найдено.

func (list *DList) SearchD(value string) int {
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

// Вывод. Для печати в консоль передавай os.Stdout.

func (list *DList) PrintForwardD(w io.Writer) {
	if list.IsEmptyD() {
		fmt.Fprintln(w, "Список пуст")
		return
	}

	for current := list.head; current != nil; current = current.Next {
		fmt.Fprint(w, current.Data, " <-> ")
	}
	fmt.Fprintln(w, "nullptr")
}

func (list *DList) PrintBackwardD(w io.Writer) {
	if list.IsEmptyD() {
		fmt.Fprintln(w, "Список пуст")
		return
	}

	for current := list.tail; current != nil; current = current.Prev {
		fmt.Fprint(w, current.Data, " <-> ")
	}
	fmt.Fprintln(w, "nullptr")
}

func (list *DList) PrintIndexedD(w io.Writer) {
	if list.IsEmptyD() {
		fmt.Fprintln(w, "Список пуст")
		return
	}

	for index, current := 0, list.head; current != nil; index, current = index+1, current.Next {
		fmt.Fprintf(w, "[%d] %s\n", index, current.Data)
	}
}

// Текстовая сериализация.
func (list *DList) SaveListDToFile(filename string) bool {
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

func (list *DList) LoadListDFromFile(filename string) bool {
	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	temporary := NewDList()
	scanner := bufio.NewScanner(file)
	scanner.Split(bufio.ScanWords)
	scanner.Buffer(make([]byte, 4096), maxDListStringSize)

	for scanner.Scan() {
		temporary.PushBackD(scanner.Text())
	}
	if scanner.Err() != nil {
		return false
	}

	list.head = temporary.head
	list.tail = temporary.tail
	return true
}

// Бинарная сериализация.
// Формат: сигнатура LISTD01, количество элементов,
// затем для каждой строки длина и байты строки.
func (list *DList) SaveListDBinary(filename string) bool {
	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	magic := [8]byte{'L', 'I', 'S', 'T', 'D', '0', '1', 0}
	writeErr := writeDListBinary(file, list, magic)
	closeErr := file.Close()

	return writeErr == nil && closeErr == nil
}

func writeDListBinary(file *os.File, list *DList, magic [8]byte) error {
	if _, err := file.Write(magic[:]); err != nil {
		return err
	}
	if err := binary.Write(file, binary.LittleEndian, uint64(list.SizeD())); err != nil {
		return err
	}

	for current := list.head; current != nil; current = current.Next {
		if len(current.Data) > maxDListStringSize {
			return fmt.Errorf("строка превышает допустимый размер")
		}

		if err := binary.Write(file, binary.LittleEndian, uint64(len(current.Data))); err != nil {
			return err
		}
		if _, err := io.WriteString(file, current.Data); err != nil {
			return err
		}
	}

	return nil
}

func (list *DList) LoadListDBinary(filename string) bool {
	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	var actualMagic [8]byte
	if _, err := io.ReadFull(file, actualMagic[:]); err != nil {
		return false
	}

	expectedMagic := [8]byte{'L', 'I', 'S', 'T', 'D', '0', '1', 0}
	if actualMagic != expectedMagic {
		return false
	}

	var count uint64
	if err := binary.Read(file, binary.LittleEndian, &count); err != nil {
		return false
	}
	if count > maxDListItems {
		return false
	}

	temporary := NewDList()

	for i := uint64(0); i < count; i++ {
		var length uint64
		if err := binary.Read(file, binary.LittleEndian, &length); err != nil {
			return false
		}
		if length > maxDListStringSize {
			return false
		}

		value := make([]byte, int(length))
		if _, err := io.ReadFull(file, value); err != nil {
			return false
		}
		temporary.PushBackD(string(value))
	}

	// Проверяем, что в файле после списка нет лишних данных.
	var extra [1]byte
	if n, err := file.Read(extra[:]); n != 0 || err != io.EOF {
		return false
	}

	// Заменяем список только после успешного чтения всего файла.
	list.head = temporary.head
	list.tail = temporary.tail
	return true
}
