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

const (
	maxArrayItems  = 1_000_000
	maxStringBytes = 64 * 1024 * 1024
)

type Array struct {
	data []string
}

func NewArray() *Array {
	return &Array{}
}

func NewArrayWithCapacity(initialCapacity int) *Array {
	if initialCapacity < 0 {
		initialCapacity = 0
	}
	return &Array{
		data: make([]string, 0, initialCapacity),
	}
}

// Копия массива
func (a *Array) Clone() *Array {
	if a == nil {
		return NewArray()
	}

	copyData := make([]string, len(a.data), cap(a.data))
	copy(copyData, a.data)
	return &Array{data: copyData}
}

// копирование
func (a *Array) Assign(other *Array) bool {
	if a == nil || other == nil {
		return false
	}

	newData := make([]string, len(other.data), cap(other.data))
	copy(newData, other.data)
	a.data = newData
	return true
}

func (a *Array) reserve(newCapacity int) {
	if newCapacity <= cap(a.data) {
		return
	}

	newData := make([]string, len(a.data), newCapacity)
	copy(newData, a.data)
	a.data = newData
}

func (a *Array) AddToEnd(value string) bool {
	if len(a.data) == cap(a.data) {
		newCapacity := 1
		if cap(a.data) > 0 {
			newCapacity = cap(a.data) * 2
		}
		a.reserve(newCapacity)
	}

	a.data = append(a.data, value)
	return true
}

func (a *Array) AddAtIndex(index int, value string) bool {
	if index < 0 || index > len(a.data) {
		return false
	}

	if len(a.data) == cap(a.data) {
		newCapacity := 1
		if cap(a.data) > 0 {
			newCapacity = cap(a.data) * 2
		}
		a.reserve(newCapacity)
	}

	a.data = append(a.data, "")
	copy(a.data[index+1:], a.data[index:len(a.data)-1])
	a.data[index] = value
	return true
}

func (a *Array) RemoveAtIndex(index int) bool {
	if index < 0 || index >= len(a.data) {
		return false
	}

	copy(a.data[index:], a.data[index+1:])
	a.data[len(a.data)-1] = "" // освобождаем ссылку на удалённую строку
	a.data = a.data[:len(a.data)-1]
	return true
}

func (a *Array) ReplaceAtIndex(index int, value string) bool {
	if index < 0 || index >= len(a.data) {
		return false
	}

	a.data[index] = value
	return true
}

// Возвращает элемент
func (a *Array) GetElement(index int) (string, bool) {
	if index < 0 || index >= len(a.data) {
		return "", false
	}
	return a.data[index], true
}

func (a *Array) GetLength() int {
	return len(a.data)
}

func (a *Array) GetCapacity() int {
	return cap(a.data)
}

func (a *Array) IsEmpty() bool {
	return len(a.data) == 0
}

func (a *Array) PrintMassiv(w io.Writer) {
	if a.IsEmpty() {
		fmt.Fprintln(w, "Массив пуст")
		return
	}

	for _, value := range a.data {
		fmt.Fprint(w, value, " ")
	}
	fmt.Fprintln(w)
}

// Читает сначала количество элементов, затем сами элементы,
// разделённые пробелами или переводами строк.
func (a *Array) ReadMassiv(r io.Reader) bool {
	reader := bufio.NewReader(r)

	var count int
	if _, err := fmt.Fscan(reader, &count); err != nil || count < 0 || count > maxArrayItems {
		return false
	}

	temp := NewArrayWithCapacity(count)
	for i := 0; i < count; i++ {
		var value string
		if _, err := fmt.Fscan(reader, &value); err != nil {
			return false
		}
		temp.AddToEnd(value)
	}

	a.data = temp.data
	return true
}

// SaveText сохраняет массив в читаемый текстовый файл.
func (a *Array) SaveText(filename string) bool {
	if a == nil || len(a.data) > maxArrayItems {
		return false
	}

	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	writer := bufio.NewWriter(file)

	writeErr := writeTextLine(writer, "ARRAY_TEXT_V1")
	if writeErr == nil {
		writeErr = writeTextLine(writer, strconv.Itoa(len(a.data)))
	}

	for _, value := range a.data {
		if writeErr != nil {
			break
		}
		if len(value) > maxStringBytes {
			writeErr = fmt.Errorf("строка слишком большая")
			break
		}
		writeErr = writeTextLine(writer, strconv.Quote(value))
	}

	if writeErr == nil {
		writeErr = writer.Flush()
	}
	closeErr := file.Close()

	return writeErr == nil && closeErr == nil
}

// LoadText загружает массив из текстового файла.
func (a *Array) LoadText(filename string) bool {
	if a == nil {
		return false
	}

	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	reader := bufio.NewReader(file)

	magic, err := readTextLine(reader)
	if err != nil || magic != "ARRAY_TEXT_V1" {
		return false
	}

	countLine, err := readTextLine(reader)
	if err != nil {
		return false
	}

	count, err := strconv.Atoi(countLine)
	if err != nil || count < 0 || count > maxArrayItems {
		return false
	}

	items := make([]string, 0, count)
	for i := 0; i < count; i++ {
		line, err := readTextLine(reader)
		if err != nil {
			return false
		}

		value, err := strconv.Unquote(line)
		if err != nil || len(value) > maxStringBytes {
			return false
		}
		items = append(items, value)
	}

	// После элементов не должно быть дополнительных данных.
	if _, err := readTextLine(reader); err != io.EOF {
		return false
	}

	a.data = items
	return true
}

// сначала uint64-размер массива, затем для каждой строки
// uint64-длина и байты строки. Числа записываются в little-endian.
func (a *Array) SaveBinary(filename string) bool {
	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	writeErr := binary.Write(file, binary.LittleEndian, uint64(len(a.data)))
	if writeErr == nil {
		for _, value := range a.data {
			if len(value) > maxStringBytes {
				writeErr = fmt.Errorf("строка слишком большая")
				break
			}

			if err := binary.Write(file, binary.LittleEndian, uint64(len(value))); err != nil {
				writeErr = err
				break
			}
			if _, err := io.WriteString(file, value); err != nil {
				writeErr = err
				break
			}
		}
	}

	closeErr := file.Close()
	return writeErr == nil && closeErr == nil
}

func (a *Array) LoadBinary(filename string) bool {
	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	var count uint64
	if err := binary.Read(file, binary.LittleEndian, &count); err != nil {
		return false
	}
	if count > maxArrayItems {
		return false
	}

	temp := NewArrayWithCapacity(int(count))

	for i := uint64(0); i < count; i++ {
		var length uint64
		if err := binary.Read(file, binary.LittleEndian, &length); err != nil {
			return false
		}
		if length > maxStringBytes {
			return false
		}

		bytes := make([]byte, int(length))
		if _, err := io.ReadFull(file, bytes); err != nil {
			return false
		}
		temp.data = append(temp.data, string(bytes))
	}

	// Меняем массив только если файл прочитан полностью без ошибок.
	a.data = temp.data
	return true
}

func writeTextLine(w *bufio.Writer, line string) error {
	if _, err := w.WriteString(line); err != nil {
		return err
	}
	return w.WriteByte('\n')
}

func readTextLine(r *bufio.Reader) (string, error) {
	line, err := r.ReadString('\n')
	if err != nil && !(err == io.EOF && len(line) > 0) {
		return "", err
	}

	if err == nil {
		line = strings.TrimSuffix(line, "\n")
	}
	line = strings.TrimSuffix(line, "\r") // поддержка окончания строк CRLF

	return line, nil
}
