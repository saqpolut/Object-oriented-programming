package ds

import (
	"bufio"
	"encoding/binary"
	"io"
	"os"
	"strconv"
)

type HashMethod uint32

const (
	LinearProbing HashMethod = iota
	DoubleHashing
)

type slotState uint8

const (
	slotEmpty    slotState = iota //ячейка никогда не использовалась
	slotOccupied                  //в ячейке хранятся пары ключ значение
	slotDeleted                   // "надгробие"
)

const (
	maxHashItems     = 1_000_000
	maxHashKeyLength = 1_048_576
)

type hashEntry struct {
	key   string
	value any
	state slotState
}

type HashTable struct {
	entries    []hashEntry
	size       int
	method     HashMethod
	probes     uint64
	collisions uint64
}

func NewHashTable(expectedElements int, method HashMethod) *HashTable {
	if expectedElements < 0 {
		expectedElements = 0
	}
	if expectedElements > maxHashItems {
		expectedElements = maxHashItems
	}
	if method != LinearProbing && method != DoubleHashing {
		method = LinearProbing
	}

	capacity := nextPrimeHash(expectedElements*2 + 11)

	return &HashTable{
		entries: make([]hashEntry, capacity),
		method:  method,
	}
}

// Хеш-функции
// uint64 переполняется по модулю 2^64, как 64-битный size_t.
func hash1(key string) uint64 {
	hash := uint64(5381)
	for i := 0; i < len(key); i++ {
		hash = ((hash << 5) + hash) + uint64(key[i])
	}
	return hash
}

func hash2(key string) uint64 {
	var hash uint64
	for i := 0; i < len(key); i++ {
		hash = uint64(key[i]) + (hash << 6) + (hash << 16) - hash
	}
	return hash
}

func isPrimeHash(number int) bool {
	if number < 2 {
		return false
	}
	for divisor := 2; divisor <= number/divisor; divisor++ {
		if number%divisor == 0 {
			return false
		}
	}
	return true
}

func nextPrimeHash(number int) int {
	if number < 2 {
		return 2
	}
	for !isPrimeHash(number) {
		number++
	}
	return number
}

func (table *HashTable) stepFor(key string) int {
	if table.method == LinearProbing || len(table.entries) <= 2 {
		return 1
	}
	return 1 + int(hash2(key)%uint64(len(table.entries)-1))
}

// Поиск. Для значений любого типа используй HashFind.
// Если значение должно быть int — HashFindInt.
func (table *HashTable) HashFind(key string) (any, bool) {
	if table == nil || len(table.entries) == 0 {
		return nil, false
	}

	index := int(hash1(key) % uint64(len(table.entries)))
	step := table.stepFor(key)

	for checked := 0; checked < len(table.entries); checked++ {
		table.probes++
		entry := &table.entries[index]

		if entry.state == slotEmpty {
			return nil, false
		}
		if entry.state == slotOccupied && entry.key == key {
			return entry.value, true
		}

		table.collisions++
		index = (index + step) % len(table.entries)
	}
	return nil, false
}

// Вставляет значение или обновляет уже существующий ключ.
func (table *HashTable) HashInsert(key string, value any) bool {
	if table == nil || len(table.entries) == 0 {
		return false
	}

	index := int(hash1(key) % uint64(len(table.entries)))
	step := table.stepFor(key)
	firstDeleted := -1

	for checked := 0; checked < len(table.entries); checked++ {
		table.probes++
		entry := &table.entries[index]

		if entry.state == slotOccupied && entry.key == key {
			entry.value = value
			return true
		}

		if entry.state == slotDeleted && firstDeleted == -1 {
			firstDeleted = index
		}

		if entry.state == slotEmpty {
			target := index
			if firstDeleted != -1 {
				target = firstDeleted
			}

			table.entries[target] = hashEntry{
				key:   key,
				value: value,
				state: slotOccupied,
			}
			table.size++
			return true
		}

		table.collisions++
		index = (index + step) % len(table.entries)
	}

	if firstDeleted != -1 {
		table.entries[firstDeleted] = hashEntry{
			key:   key,
			value: value,
			state: slotOccupied,
		}
		table.size++
		return true
	}

	return false
}

func (table *HashTable) HashErase(key string) bool {
	if table == nil || len(table.entries) == 0 {
		return false
	}

	index := int(hash1(key) % uint64(len(table.entries)))
	step := table.stepFor(key)

	for checked := 0; checked < len(table.entries); checked++ {
		table.probes++
		entry := &table.entries[index]

		if entry.state == slotEmpty {
			return false
		}

		if entry.state == slotOccupied && entry.key == key {
			entry.state = slotDeleted
			entry.value = nil
			table.size--
			return true
		}

		index = (index + step) % len(table.entries)
	}
	return false
}

// Обёртки для целочисленных значений.
func (table *HashTable) HashInsertInt(key string, value int) bool {
	return table.HashInsert(key, value)
}

func (table *HashTable) HashFindInt(key string) (int, bool) {
	value, found := table.HashFind(key)
	if !found {
		return 0, false
	}

	number, ok := value.(int)
	return number, ok
}

// Геттеры.
func (table *HashTable) GetSize() int {
	if table == nil {
		return 0
	}
	return table.size
}

func (table *HashTable) GetCapacity() int {
	if table == nil {
		return 0
	}
	return len(table.entries)
}

func (table *HashTable) GetProbes() uint64 {
	if table == nil {
		return 0
	}
	return table.probes
}

func (table *HashTable) GetCollisions() uint64 {
	if table == nil {
		return 0
	}
	return table.collisions
}

func (table *HashTable) IsEmpty() bool {
	return table == nil || table.size == 0
}

// Преобразует целочисленное значение к формату int32,
func valueAsInt32(value any) (int32, bool) {
	switch number := value.(type) {
	case int:
		return int32(number), true
	case int8:
		return int32(number), true
	case int16:
		return int32(number), true
	case int32:
		return number, true
	case int64:
		return int32(number), true
	case uint:
		return int32(number), true
	case uint8:
		return int32(number), true
	case uint16:
		return int32(number), true
	case uint32:
		return int32(number), true
	case uint64:
		return int32(number), true
	case uintptr:
		return int32(number), true
	default:
		return 0, false
	}
}

// Сохраняет таблицу
// HTBIN01, метод, количество записей, затем ключи и значения int32.
func (table *HashTable) SaveBinary(filename string) bool {
	if table == nil {
		return false
	}

	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	writeErr := func() error {
		magic := [8]byte{'H', 'T', 'B', 'I', 'N', '0', '1', 0}
		if _, err := file.Write(magic[:]); err != nil {
			return err
		}

		if err := binary.Write(file, binary.LittleEndian, uint32(table.method)); err != nil {
			return err
		}
		if err := binary.Write(file, binary.LittleEndian, uint64(table.size)); err != nil {
			return err
		}

		for i := range table.entries {
			entry := &table.entries[i]
			if entry.state != slotOccupied {
				continue
			}
			if len(entry.key) > maxHashKeyLength {
				return errInvalidHashFile
			}

			value, ok := valueAsInt32(entry.value)
			if !ok {
				return errInvalidHashFile
			}

			if err := binary.Write(file, binary.LittleEndian, uint64(len(entry.key))); err != nil {
				return err
			}
			if _, err := io.WriteString(file, entry.key); err != nil {
				return err
			}
			if err := binary.Write(file, binary.LittleEndian, value); err != nil {
				return err
			}
		}
		return nil
	}()

	closeErr := file.Close()
	return writeErr == nil && closeErr == nil
}

var errInvalidHashFile = &hashFileError{}

type hashFileError struct{}

func (*hashFileError) Error() string {
	return "неподдерживаемое значение или слишком длинный ключ"
}

func (table *HashTable) LoadBinary(filename string) bool {
	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	var actualMagic [8]byte
	if _, err := io.ReadFull(file, actualMagic[:]); err != nil {
		return false
	}

	expectedMagic := [8]byte{'H', 'T', 'B', 'I', 'N', '0', '1', 0}
	if actualMagic != expectedMagic {
		return false
	}

	var savedMethod uint32
	var savedSize uint64

	if binary.Read(file, binary.LittleEndian, &savedMethod) != nil ||
		binary.Read(file, binary.LittleEndian, &savedSize) != nil {
		return false
	}
	if savedMethod > uint32(DoubleHashing) || savedSize > maxHashItems {
		return false
	}

	temporary := NewHashTable(int(savedSize), HashMethod(savedMethod))

	for i := uint64(0); i < savedSize; i++ {
		var keyLength uint64
		if binary.Read(file, binary.LittleEndian, &keyLength) != nil {
			return false
		}
		if keyLength > maxHashKeyLength {
			return false
		}

		keyBytes := make([]byte, int(keyLength))
		if _, err := io.ReadFull(file, keyBytes); err != nil {
			return false
		}

		var value int32
		if binary.Read(file, binary.LittleEndian, &value) != nil {
			return false
		}
		if !temporary.HashInsertInt(string(keyBytes), int(value)) {
			return false
		}
	}

	// Не загружаем файл, если после данных остались лишние байты.
	var extra [1]byte
	if n, err := file.Read(extra[:]); n != 0 || err != io.EOF {
		return false
	}

	if uint64(temporary.size) != savedSize {
		return false
	}

	temporary.probes = 0
	temporary.collisions = 0
	*table = *temporary
	return true
}

type hashTableTextEntry struct {
	key   string
	value int32
}

// SaveText сохраняет хеш-таблицу в текстовый файл.
// Сохраняются только целочисленные значения, помещающиеся в int32.
func (table *HashTable) SaveText(filename string) bool {
	if table == nil {
		return false
	}

	file, err := os.Create(filename)
	if err != nil {
		return false
	}

	writer := bufio.NewWriter(file)

	writeErr := writeTextLine(writer, "HASHTABLE_TEXT_V1")
	if writeErr == nil {
		writeErr = writeTextLine(writer, strconv.FormatUint(uint64(table.method), 10))
	}
	if writeErr == nil {
		writeErr = writeTextLine(writer, strconv.Itoa(table.size))
	}

	for i := range table.entries {
		if writeErr != nil {
			break
		}

		entry := &table.entries[i]
		if entry.state != slotOccupied {
			continue
		}
		if len(entry.key) > maxHashKeyLength {
			writeErr = errInvalidHashFile
			break
		}

		value, ok := valueAsInt32(entry.value)
		if !ok {
			writeErr = errInvalidHashFile
			break
		}

		writeErr = writeTextLine(writer, strconv.Quote(entry.key))
		if writeErr == nil {
			writeErr = writeTextLine(writer, strconv.FormatInt(int64(value), 10))
		}
	}

	if writeErr == nil {
		writeErr = writer.Flush()
	}
	closeErr := file.Close()

	return writeErr == nil && closeErr == nil
}

// LoadText загружает хеш-таблицу из текстового файла.
func (table *HashTable) LoadText(filename string) bool {
	if table == nil {
		return false
	}

	file, err := os.Open(filename)
	if err != nil {
		return false
	}
	defer file.Close()

	reader := bufio.NewReader(file)

	magic, err := readTextLine(reader)
	if err != nil || magic != "HASHTABLE_TEXT_V1" {
		return false
	}

	methodLine, err := readTextLine(reader)
	if err != nil {
		return false
	}

	methodValue, err := strconv.ParseUint(methodLine, 10, 32)
	if err != nil || methodValue > uint64(DoubleHashing) {
		return false
	}
	method := HashMethod(methodValue)

	countLine, err := readTextLine(reader)
	if err != nil {
		return false
	}

	count, err := strconv.Atoi(countLine)
	if err != nil || count < 0 || count > maxHashItems {
		return false
	}

	temporary := NewHashTable(count, method)

	for i := 0; i < count; i++ {
		keyLine, err := readTextLine(reader)
		if err != nil {
			return false
		}

		key, err := strconv.Unquote(keyLine)
		if err != nil || len(key) > maxHashKeyLength {
			return false
		}

		valueLine, err := readTextLine(reader)
		if err != nil {
			return false
		}

		value, err := strconv.ParseInt(valueLine, 10, 32)
		if err != nil {
			return false
		}

		oldSize := temporary.size
		if !temporary.HashInsertInt(key, int(value)) || temporary.size == oldSize {
			return false // ошибка вставки или повторяющийся ключ
		}
	}

	// После записей не должно быть дополнительных данных.
	if _, err := readTextLine(reader); err != io.EOF {
		return false
	}

	temporary.probes = 0
	temporary.collisions = 0
	*table = *temporary
	return true
}
