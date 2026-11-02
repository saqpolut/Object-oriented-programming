package ds

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"io"
	"os"
)

const (
	maxQueueItems      = 1_000_000
	maxQueueStringSize = 16 * 1024 * 1024
)

type queueNode struct {
	data string
	next *queueNode
}

type Queue struct {
	head  *queueNode // откуда извлекаем
	tail  *queueNode // куда добавляем
	count int
}

func NewQueue() *Queue {
	return &Queue{}
}

// Проверка и размер
func (q *Queue) IsQueueEmpty() bool {
	return q == nil || q.head == nil
}

func (q *Queue) SizeQueue() int {
	if q == nil {
		return 0
	}
	return q.count
}

// Основные операции
func (q *Queue) PushQueue(value string) {
	node := &queueNode{data: value}

	if q.tail == nil {
		q.head = node
		q.tail = node
	} else {
		q.tail.next = node
		q.tail = node
	}
	q.count++
}

// возвращаются извлечённое значение и признак успеха.
func (q *Queue) PopQueue() (string, bool) {
	if q.IsQueueEmpty() {
		return "", false
	}

	value := q.head.data
	q.head = q.head.next
	q.count--

	if q.head == nil {
		q.tail = nil
	}
	return value, true
}

func (q *Queue) PeekQueue() (string, bool) {
	if q.IsQueueEmpty() {
		return "", false
	}
	return q.head.data, true
}

// Выводит элементы от головы к хвосту.
func (q *Queue) PrintQueue(w io.Writer) {
	if q.IsQueueEmpty() {
		fmt.Fprintln(w, "очередь пуста")
		return
	}

	for current := q.head; current != nil; current = current.next {
		fmt.Fprint(w, current.data, " ")
	}
	fmt.Fprintln(w)
}

// Текстовая сериализация.
func (q *Queue) SaveQueueToFile(filename string) bool {
	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	writer := bufio.NewWriter(file)
	for current := q.head; current != nil; current = current.next {
		if _, err := fmt.Fprintln(writer, current.data); err != nil {
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

func (q *Queue) LoadQueueFromFile(filename string) bool {
	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	temporary := NewQueue()

	scanner := bufio.NewScanner(file)
	scanner.Split(bufio.ScanWords)
	scanner.Buffer(make([]byte, 4096), maxQueueStringSize)

	for scanner.Scan() {
		temporary.PushQueue(scanner.Text())
	}
	if scanner.Err() != nil {
		return false
	}

	// Заменяем текущую очередь только после успешного чтения.
	q.head = temporary.head
	q.tail = temporary.tail
	q.count = temporary.count
	return true
}

// Бинарный формат: сигнатура QUEUE01, число элементов,
// затем для каждого элемента длина строки и её байты.
func (q *Queue) SaveQueueBinary(filename string) bool {
	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	writeErr := func() error {
		magic := [8]byte{'Q', 'U', 'E', 'U', 'E', '0', '1', 0}
		if _, err := file.Write(magic[:]); err != nil {
			return err
		}

		if err := binary.Write(file, binary.LittleEndian, uint64(q.count)); err != nil {
			return err
		}

		for current := q.head; current != nil; current = current.next {
			if len(current.data) > maxQueueStringSize {
				return fmt.Errorf("элемент очереди превышает допустимый размер")
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

func (q *Queue) LoadQueueBinary(filename string) bool {
	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	var actualMagic [8]byte
	if _, err := io.ReadFull(file, actualMagic[:]); err != nil {
		return false
	}

	expectedMagic := [8]byte{'Q', 'U', 'E', 'U', 'E', '0', '1', 0}
	if actualMagic != expectedMagic {
		return false
	}

	var elementCount uint64
	if err := binary.Read(file, binary.LittleEndian, &elementCount); err != nil {
		return false
	}
	if elementCount > maxQueueItems {
		return false
	}

	temporary := NewQueue()

	for i := uint64(0); i < elementCount; i++ {
		var length uint64
		if err := binary.Read(file, binary.LittleEndian, &length); err != nil {
			return false
		}
		if length > maxQueueStringSize {
			return false
		}

		value := make([]byte, int(length))
		if _, err := io.ReadFull(file, value); err != nil {
			return false
		}

		temporary.PushQueue(string(value))
	}

	// Не принимаем файл с лишними данными в конце.
	var extra [1]byte
	if n, err := file.Read(extra[:]); n != 0 || err != io.EOF {
		return false
	}

	// При ошибке чтения исходная очередь не меняется.
	q.head = temporary.head
	q.tail = temporary.tail
	q.count = temporary.count
	return true
}
