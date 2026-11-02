package ds

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"io"
	"os"
)

const (
	maxStackItems      = 1_000_000
	maxStackStringSize = 16 * 1024 * 1024
)

type stackNode struct {
	data string
	next *stackNode
}

type Stack struct {
	head *stackNode // вершина стека
}

func NewStack() *Stack {
	return &Stack{}
}

// Проверка на пустоту
func (s *Stack) IsStackEmpty() bool {
	return s == nil || s.head == nil
}

// Основные операции
func (s *Stack) PushStack(value string) {
	s.head = &stackNode{
		data: value,
		next: s.head,
	}
}

// возвращаем значение и признак успеха
func (s *Stack) PopStack() (string, bool) {
	if s.IsStackEmpty() {
		return "", false
	}

	value := s.head.data
	s.head = s.head.next
	return value, true
}

func (s *Stack) PeekStack() (string, bool) {
	if s.IsStackEmpty() {
		return "", false
	}
	return s.head.data, true
}

// Выводит стек от вершины к основанию.
func (s *Stack) PrintStack(w io.Writer) {
	if s.IsStackEmpty() {
		fmt.Fprintln(w, "стек пуст")
		return
	}

	for current := s.head; current != nil; current = current.next {
		fmt.Fprint(w, current.data, " ")
	}
	fmt.Fprintln(w)
}

// Текстовая сериализация.
func (s *Stack) SaveStackToFile(filename string) bool {
	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	// Список связан от вершины к основанию, поэтому собираем значения
	// и записываем их в обратном порядке.
	values := make([]string, 0)
	for current := s.head; current != nil; current = current.next {
		values = append(values, current.data)
	}

	writer := bufio.NewWriter(file)
	for i := len(values) - 1; i >= 0; i-- {
		if _, err := fmt.Fprintln(writer, values[i]); err != nil {
			_ = file.Close()
			return false
		}
	}

	if err := writer.Flush(); err != nil {
		_ = file.Close()
		return false
	}
	return file.Close() == nil
}

func (s *Stack) LoadStackFromFile(filename string) bool {
	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	temporary := NewStack()

	scanner := bufio.NewScanner(file)
	scanner.Split(bufio.ScanWords)
	scanner.Buffer(make([]byte, 4096), maxStackStringSize)

	// Файл содержит элементы от основания к вершине.
	// Последний прочитанный элемент станет вершиной.
	for scanner.Scan() {
		temporary.PushStack(scanner.Text())
	}
	if scanner.Err() != nil {
		return false
	}

	s.head = temporary.head
	return true
}

// Бинарная сериализация.
// Формат: сигнатура STACK01, количество элементов,
// далее длина и байты каждого элемента от вершины к основанию.
func (s *Stack) SaveStackBinary(filename string) bool {
	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	writeErr := func() error {
		magic := [8]byte{'S', 'T', 'A', 'C', 'K', '0', '1', 0}
		if _, err := file.Write(magic[:]); err != nil {
			return err
		}

		count := uint64(0)
		for current := s.head; current != nil; current = current.next {
			count++
		}
		if err := binary.Write(file, binary.LittleEndian, count); err != nil {
			return err
		}

		for current := s.head; current != nil; current = current.next {
			if len(current.data) > maxStackStringSize {
				return fmt.Errorf("элемент стека превышает допустимый размер")
			}

			if err := binary.Write(
				file,
				binary.LittleEndian,
				uint64(len(current.data)),
			); err != nil {
				return err
			}

			if _, err := io.WriteString(file, current.data); err != nil {
				return err
			}
		}
		return nil
	}()

	closeErr := file.Close()
	return writeErr == nil && closeErr == nil
}

func (s *Stack) LoadStackBinary(filename string) bool {
	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	var actualMagic [8]byte
	if _, err := io.ReadFull(file, actualMagic[:]); err != nil {
		return false
	}

	expectedMagic := [8]byte{'S', 'T', 'A', 'C', 'K', '0', '1', 0}
	if actualMagic != expectedMagic {
		return false
	}

	var count uint64
	if err := binary.Read(file, binary.LittleEndian, &count); err != nil {
		return false
	}
	if count > maxStackItems {
		return false
	}

	// Сохраняем порядок элементов файла: первый прочитанный —
	// вершина стека, как и в C++-версии.
	temporary := NewStack()
	var tail *stackNode

	for i := uint64(0); i < count; i++ {
		var length uint64
		if err := binary.Read(file, binary.LittleEndian, &length); err != nil {
			return false
		}
		if length > maxStackStringSize {
			return false
		}

		value := make([]byte, int(length))
		if _, err := io.ReadFull(file, value); err != nil {
			return false
		}

		node := &stackNode{data: string(value)}
		if temporary.head == nil {
			temporary.head = node
		} else {
			tail.next = node
		}
		tail = node
	}

	// Если файл повреждён или содержит лишние байты,
	// текущий стек остаётся без изменений.
	var extra [1]byte
	if n, err := file.Read(extra[:]); n != 0 || err != io.EOF {
		return false
	}

	s.head = temporary.head
	return true
}
