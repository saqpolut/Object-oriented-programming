package ds

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

// ============================================================
// 1. МАССИВ (Massiv)
// ============================================================

func TestMassiv_OperationsAndBoundaries(t *testing.T) {
	m := NewArray()
	assert.True(t, m.IsEmpty())
	assert.Equal(t, 0, m.GetLength())

	_, ok := m.GetElement(0)
	assert.False(t, ok)
	assert.False(t, m.RemoveAtIndex(0))
	assert.False(t, m.ReplaceAtIndex(0, "x"))
	assert.False(t, m.AddAtIndex(1, "x"))

	assert.True(t, m.AddToEnd("a"))
	assert.True(t, m.AddToEnd("b"))
	assert.True(t, m.AddAtIndex(1, "middle"))
	assert.Equal(t, 3, m.GetLength())

	val, ok := m.GetElement(0)
	assert.True(t, ok)
	assert.Equal(t, "a", val)

	val, ok = m.GetElement(1)
	assert.True(t, ok)
	assert.Equal(t, "middle", val)

	val, ok = m.GetElement(2)
	assert.True(t, ok)
	assert.Equal(t, "b", val)

	assert.True(t, m.ReplaceAtIndex(0, "changed"))
	assert.False(t, m.ReplaceAtIndex(10, "bad"))
	_, ok = m.GetElement(10)
	assert.False(t, ok)

	assert.True(t, m.RemoveAtIndex(1))
	assert.False(t, m.RemoveAtIndex(10))
	assert.Equal(t, 2, m.GetLength())

	assert.True(t, m.RemoveAtIndex(1))
	assert.True(t, m.RemoveAtIndex(0))
	assert.True(t, m.IsEmpty())

	assert.True(t, m.AddAtIndex(0, "first"))
	val, ok = m.GetElement(0)
	assert.True(t, ok)
	assert.Equal(t, "first", val)
}

func TestMassiv_ReadAndPrint(t *testing.T) {
	m := NewArray()
	var buf bytes.Buffer
	m.PrintMassiv(&buf)
	assert.Contains(t, buf.String(), "Массив пуст")

	buf.Reset()
	m.AddToEnd("x")
	m.AddToEnd("y")
	m.PrintMassiv(&buf)
	assert.Contains(t, buf.String(), "x y")

	// Тест чтения ReadMassiv
	inputGood := bytes.NewBufferString("3 alpha beta gamma\n")
	assert.True(t, m.ReadMassiv(inputGood))
	assert.Equal(t, 3, m.GetLength())
	v, _ := m.GetElement(2)
	assert.Equal(t, "gamma", v)

	inputBadCount := bytes.NewBufferString("not_a_number\n")
	assert.False(t, m.ReadMassiv(inputBadCount))

	inputShort := bytes.NewBufferString("3 only_one\n")
	assert.False(t, m.ReadMassiv(inputShort))
}

func TestMassiv_BinarySerialization(t *testing.T) {
	tmpDir := t.TempDir()
	file := filepath.Join(tmpDir, "massiv.bin")

	m := NewArray()
	m.AddToEnd("alpha")
	m.AddToEnd("")
	m.AddToEnd("beta")

	assert.True(t, m.SaveBinary(file))
	assert.False(t, m.SaveBinary("/bad_dir_123/file.bin"))

	loaded := NewArray()
	assert.True(t, loaded.LoadBinary(file))
	assert.Equal(t, 3, loaded.GetLength())
	v0, _ := loaded.GetElement(0)
	v1, _ := loaded.GetElement(1)
	v2, _ := loaded.GetElement(2)
	assert.Equal(t, "alpha", v0)
	assert.Equal(t, "", v1)
	assert.Equal(t, "beta", v2)

	assert.False(t, loaded.LoadBinary(filepath.Join(tmpDir, "missing.bin")))

	// Битая сигнатура / усечённый файл
	truncFile := filepath.Join(tmpDir, "trunc.bin")
	_ = os.WriteFile(truncFile, []byte("BADMAGIC"), 0644)
	assert.False(t, loaded.LoadBinary(truncFile))

	// Усечённое число элементов
	shortFile := filepath.Join(tmpDir, "short.bin")
	var data bytes.Buffer
	data.WriteString("MASSIV01")
	_ = binary.Write(&data, binary.LittleEndian, uint64(5))
	_ = os.WriteFile(shortFile, data.Bytes(), 0644)
	assert.False(t, loaded.LoadBinary(shortFile))
}

func TestArray_DeepCoverage(t *testing.T) {
	// Непокрытые конструкторы и геттеры
	neg := NewArrayWithCapacity(-5)
	assert.Equal(t, 0, neg.GetCapacity())
	assert.Equal(t, 0, neg.GetLength())

	a := NewArray()
	a.AddToEnd("x")
	a.AddToEnd("y")
	assert.Equal(t, 2, a.GetCapacity())

	// Clone
	var nilArr *Array
	assert.NotNil(t, nilArr.Clone())
	clone := a.Clone()
	assert.Equal(t, 2, clone.GetLength())
	v, ok := clone.GetElement(1)
	assert.True(t, ok)
	assert.Equal(t, "y", v)

	// Assign
	var target Array
	assert.False(t, target.Assign(nil))
	assert.False(t, nilArr.Assign(a))
	assert.True(t, target.Assign(a))
	assert.Equal(t, 2, target.GetLength())

	// ReadMassiv границы
	assert.False(t, a.ReadMassiv(strings.NewReader("-1")))
	assert.False(t, a.ReadMassiv(strings.NewReader("10000000")))
	assert.False(t, a.ReadMassiv(strings.NewReader("2 only_one")))

	// Ошибки бинарных файлов
	assert.False(t, a.SaveBinary("/dev/null/impossible"))
	assert.False(t, a.LoadBinary("non_existent_file.bin"))

	badMagicFile := filepath.Join(t.TempDir(), "bad_arr.bin")
	os.WriteFile(badMagicFile, []byte{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 0644)
	assert.False(t, a.LoadBinary(badMagicFile))
}
func TestArray_TextSerialization(t *testing.T) {
	filename := filepath.Join(t.TempDir(), "array.txt")

	original := NewArray()
	original.AddToEnd("текст с пробелами")
	original.AddToEnd("строка\nс переносом")

	require.True(t, original.SaveText(filename))

	loaded := NewArray()
	require.True(t, loaded.LoadText(filename))
	require.Equal(t, original.GetLength(), loaded.GetLength())

	for i := 0; i < original.GetLength(); i++ {
		want, _ := original.GetElement(i)
		got, ok := loaded.GetElement(i)
		require.True(t, ok)
		assert.Equal(t, want, got)
	}
}

// ============================================================
// 2. ОДНОСВЯЗНЫЙ СПИСОК (FList)
// ============================================================

func TestFList_OperationsAndBoundaries(t *testing.T) {
	list := NewFList()

	assert.True(t, list.IsEmptyF())
	assert.Equal(t, 0, list.SizeF())
	assert.Equal(t, -1, list.SearchF("missing"))
	assert.Nil(t, list.GetAtF(-1))
	assert.Nil(t, list.GetAtF(0))
	assert.Nil(t, list.FindNodeF("missing"))

	assert.False(t, list.PopFrontF())
	assert.False(t, list.PopBackF())
	assert.False(t, list.RemoveAfterF("missing"))
	assert.False(t, list.RemoveBeforeF("missing"))
	assert.False(t, list.RemoveByValueF("missing"))
	assert.False(t, list.InsertBeforeF("missing", "x"))
	assert.False(t, list.InsertAfterF("missing", "x"))

	list.PushFrontF("b")
	list.PushFrontF("a")
	list.PushBackF("d")

	assert.Equal(t, 3, list.SizeF())
	assert.Equal(t, 0, list.SearchF("a"))
	assert.Equal(t, 2, list.SearchF("d"))
	assert.Equal(t, -1, list.SearchF("missing"))
	require.NotNil(t, list.GetAtF(1))
	assert.Equal(t, "b", list.GetAtF(1).Data)
	assert.Nil(t, list.GetAtF(3))

	assert.True(t, list.InsertAfterF("b", "c"))
	assert.True(t, list.InsertAfterF("d", "tail")) // вставка после хвоста
	assert.True(t, list.InsertBeforeF("a", "first"))
	assert.True(t, list.InsertBeforeF("c", "between"))

	assert.True(t, list.RemoveBeforeF("a"))
	assert.True(t, list.RemoveBeforeF("c"))
	assert.False(t, list.RemoveBeforeF("first"))
	assert.False(t, list.RemoveBeforeF("missing"))

	assert.True(t, list.RemoveAfterF("b"))
	assert.False(t, list.RemoveAfterF("tail"))

	assert.True(t, list.RemoveByValueF("a"))
	assert.True(t, list.RemoveByValueF("tail"))
	assert.False(t, list.RemoveByValueF("missing"))

	// Удаление до пустоты
	for list.PopFrontF() {
	}
	assert.True(t, list.IsEmptyF())
	assert.False(t, list.PopBackF())

	// PopBack для 1 и более элементов
	single := NewFList()
	single.PushBackF("only")
	assert.True(t, single.PopBackF())
	assert.True(t, single.IsEmptyF())

	multi := NewFList()
	multi.PushBackF("1")
	multi.PushBackF("2")
	multi.PushBackF("3")
	assert.True(t, multi.PopBackF())
	assert.Equal(t, 2, multi.SizeF())
}

func TestFList_PrintAndSerialization(t *testing.T) {
	tmpDir := t.TempDir()
	txtFile := filepath.Join(tmpDir, "flist.txt")
	binFile := filepath.Join(tmpDir, "flist.bin")

	list := NewFList()
	var buf bytes.Buffer
	list.PrintForwardF(&buf)
	list.PrintBackwardF(&buf)
	list.PrintIndexedF(&buf)
	assert.Contains(t, buf.String(), "Список пуст")

	list.PushBackF("one")
	list.PushBackF("two")
	buf.Reset()
	list.PrintForwardF(&buf)
	list.PrintBackwardF(&buf)
	list.PrintIndexedF(&buf)
	assert.Contains(t, buf.String(), "one -> two -> nullptr")

	// Текст
	assert.True(t, list.SaveListFToFile(txtFile))
	assert.False(t, list.SaveListFToFile("/bad_dir/file.txt"))

	loadedTxt := NewFList()
	assert.True(t, loadedTxt.LoadListFFromFile(txtFile))
	assert.Equal(t, 2, loadedTxt.SizeF())
	assert.Equal(t, "one", loadedTxt.GetAtF(0).Data)
	assert.False(t, loadedTxt.LoadListFFromFile(filepath.Join(tmpDir, "missing.txt")))

	// Бинарь
	assert.True(t, list.SaveListFBinary(binFile))
	assert.False(t, list.SaveListFBinary("/bad_dir/file.bin"))

	loadedBin := NewFList()
	assert.True(t, loadedBin.LoadListFBinary(binFile))
	assert.Equal(t, 2, loadedBin.SizeF())
	assert.Equal(t, "two", loadedBin.GetAtF(1).Data)
	assert.False(t, loadedBin.LoadListFBinary(filepath.Join(tmpDir, "missing.bin")))

	// Битый бинарь
	badBin := filepath.Join(tmpDir, "bad.bin")
	_ = os.WriteFile(badBin, []byte("BADMAGIC"), 0644)
	assert.False(t, loadedBin.LoadListFBinary(badBin))
}

// ============================================================
// 3. ДВУСВЯЗНЫЙ СПИСОК (DList)
// ============================================================

func TestDList_OperationsAndBoundaries(t *testing.T) {
	list := NewDList()

	assert.True(t, list.IsEmptyD())
	assert.Equal(t, 0, list.SizeD())
	assert.Equal(t, -1, list.SearchD("missing"))
	assert.Nil(t, list.GetAtD(-1))
	assert.Nil(t, list.GetAtD(0))
	assert.Nil(t, list.FindNodeD("missing"))

	assert.False(t, list.PopFrontD())
	assert.False(t, list.PopBackD())
	assert.False(t, list.RemoveAfterD("missing"))
	assert.False(t, list.RemoveBeforeD("missing"))
	assert.False(t, list.RemoveByValueD("missing"))
	assert.False(t, list.InsertAfterD("missing", "x"))
	assert.False(t, list.InsertBeforeD("missing", "x"))

	list.PushFrontD("b")
	list.PushFrontD("a")
	list.PushBackD("d")

	assert.Equal(t, 3, list.SizeD())
	assert.Equal(t, 0, list.SearchD("a"))
	assert.Equal(t, 2, list.SearchD("d"))
	require.NotNil(t, list.GetAtD(1))
	assert.Equal(t, "b", list.GetAtD(1).Data)
	assert.Nil(t, list.GetAtD(3))

	assert.True(t, list.InsertAfterD("b", "c"))
	assert.True(t, list.InsertAfterD("d", "tail"))
	assert.True(t, list.InsertBeforeD("a", "first"))
	assert.True(t, list.InsertBeforeD("c", "mid"))

	assert.True(t, list.RemoveBeforeD("a"))
	assert.True(t, list.RemoveBeforeD("c"))
	assert.False(t, list.RemoveBeforeD("first"))
	assert.False(t, list.RemoveBeforeD("missing"))

	assert.True(t, list.RemoveAfterD("a"))
	assert.False(t, list.RemoveAfterD("tail"))

	assert.True(t, list.RemoveByValueD("a"))
	assert.True(t, list.RemoveByValueD("tail"))
	assert.False(t, list.RemoveByValueD("missing"))

	assert.True(t, list.PopFrontD())
	assert.True(t, list.PopBackD())

	single := NewDList()
	single.PushBackD("only")
	assert.True(t, single.PopFrontD())
	assert.True(t, single.IsEmptyD())

	single2 := NewDList()
	single2.PushBackD("only")
	assert.True(t, single2.PopBackD())
	assert.True(t, single2.IsEmptyD())
}

func TestDList_PrintAndSerialization(t *testing.T) {
	tmpDir := t.TempDir()
	txtFile := filepath.Join(tmpDir, "dlist.txt")
	binFile := filepath.Join(tmpDir, "dlist.bin")

	list := NewDList()
	var buf bytes.Buffer
	list.PrintForwardD(&buf)
	list.PrintBackwardD(&buf)
	list.PrintIndexedD(&buf)
	assert.Contains(t, buf.String(), "Список пуст")

	list.PushBackD("first val")
	list.PushBackD("second")

	buf.Reset()
	list.PrintForwardD(&buf)
	list.PrintBackwardD(&buf)
	list.PrintIndexedD(&buf)
	assert.Contains(t, buf.String(), "<->")

	// Текст
	assert.True(t, list.SaveListDToFile(txtFile))
	assert.False(t, list.SaveListDToFile("/bad/file.txt"))

	loadedTxt := NewDList()
	assert.True(t, loadedTxt.LoadListDFromFile(txtFile))
	assert.False(t, loadedTxt.LoadListDFromFile(filepath.Join(tmpDir, "no.txt")))

	// Бинарная
	assert.True(t, list.SaveListDBinary(binFile))
	assert.False(t, list.SaveListDBinary("/bad/file.bin"))

	loadedBin := NewDList()
	assert.True(t, loadedBin.LoadListDBinary(binFile))
	assert.Equal(t, 2, loadedBin.SizeD())
	assert.Equal(t, "first val", loadedBin.GetAtD(0).Data)
	assert.Equal(t, "second", loadedBin.GetAtD(1).Data)
	assert.False(t, loadedBin.LoadListDBinary(filepath.Join(tmpDir, "no.bin")))

	// Битая сигнатура
	badFile := filepath.Join(tmpDir, "bad_dlist.bin")
	_ = os.WriteFile(badFile, []byte("BADMAGIC"), 0644)
	assert.False(t, loadedBin.LoadListDBinary(badFile))
}

// ============================================================
// 4. ОЧЕРЕДЬ (Queue)
// ============================================================

func TestQueue_OperationsAndSerialization(t *testing.T) {
	tmpDir := t.TempDir()
	txtFile := filepath.Join(tmpDir, "queue.txt")
	binFile := filepath.Join(tmpDir, "queue.bin")

	q := NewQueue()
	assert.True(t, q.IsQueueEmpty())
	assert.Equal(t, 0, q.SizeQueue())

	_, ok := q.PeekQueue()
	assert.False(t, ok)
	_, ok = q.PopQueue()
	assert.False(t, ok)

	var buf bytes.Buffer
	q.PrintQueue(&buf)
	assert.Contains(t, buf.String(), "очередь пуста")

	q.PushQueue("first")
	q.PushQueue("second")
	assert.False(t, q.IsQueueEmpty())
	assert.Equal(t, 2, q.SizeQueue())

	val, ok := q.PeekQueue()
	assert.True(t, ok)
	assert.Equal(t, "first", val)

	buf.Reset()
	q.PrintQueue(&buf)
	assert.Contains(t, buf.String(), "first second")

	// Текст
	assert.True(t, q.SaveQueueToFile(txtFile))
	assert.False(t, q.SaveQueueToFile("/bad/queue.txt"))

	loadedTxt := NewQueue()
	assert.True(t, loadedTxt.LoadQueueFromFile(txtFile))
	assert.Equal(t, 2, loadedTxt.SizeQueue())
	v, _ := loadedTxt.PopQueue()
	assert.Equal(t, "first", v)
	assert.False(t, loadedTxt.LoadQueueFromFile(filepath.Join(tmpDir, "no.txt")))

	// Бинарная
	assert.True(t, q.SaveQueueBinary(binFile))
	assert.False(t, q.SaveQueueBinary("/bad/queue.bin"))

	loadedBin := NewQueue()
	assert.True(t, loadedBin.LoadQueueBinary(binFile))
	assert.Equal(t, 2, loadedBin.SizeQueue())
	v1, ok1 := loadedBin.PopQueue()
	v2, ok2 := loadedBin.PopQueue()
	assert.True(t, ok1 && ok2)
	assert.Equal(t, "first", v1)
	assert.Equal(t, "second", v2)
	assert.True(t, loadedBin.IsQueueEmpty())
	assert.False(t, loadedBin.LoadQueueBinary(filepath.Join(tmpDir, "no.bin")))

	// Битая сигнатура
	badBin := filepath.Join(tmpDir, "bad_q.bin")
	_ = os.WriteFile(badBin, []byte("BADMAGIC"), 0644)
	assert.False(t, loadedBin.LoadQueueBinary(badBin))
}

func TestQueue_DeepCoverage(t *testing.T) {
	var nilQ *Queue
	assert.True(t, nilQ.IsQueueEmpty())
	assert.Equal(t, 0, nilQ.SizeQueue())
	_, ok := nilQ.PopQueue()
	assert.False(t, ok)
	_, ok = nilQ.PeekQueue()
	assert.False(t, ok)

	q := NewQueue()
	var buf bytes.Buffer
	q.PrintQueue(&buf) // пустая очередь

	assert.False(t, q.SaveQueueToFile("/dev/null/impossible"))
	assert.False(t, q.LoadQueueFromFile("non_existent_queue.txt"))

	assert.False(t, q.SaveQueueBinary("/dev/null/impossible"))
	assert.False(t, q.LoadQueueBinary("non_existent_queue.bin"))

	tmp := filepath.Join(t.TempDir(), "bad_q.bin")
	// Неверная сигнатура
	os.WriteFile(tmp, []byte("BADMAGIC"), 0644)
	assert.False(t, q.LoadQueueBinary(tmp))

	// Сигнатура верная, но размер элементов зашкаливает
	bufBad := append([]byte{'Q', 'U', 'E', 'U', 'E', '0', '1', 0}, []byte{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}...)
	os.WriteFile(tmp, bufBad, 0644)
	assert.False(t, q.LoadQueueBinary(tmp))
}

// ============================================================
// 5. СТЕК (Stack)
// ============================================================

func TestStack_OperationsAndSerialization(t *testing.T) {
	tmpDir := t.TempDir()
	txtFile := filepath.Join(tmpDir, "stack.txt")
	binFile := filepath.Join(tmpDir, "stack.bin")

	s := NewStack()
	assert.True(t, s.IsStackEmpty())
	_, ok := s.PeekStack()
	assert.False(t, ok)
	_, ok = s.PopStack()
	assert.False(t, ok)

	var buf bytes.Buffer
	s.PrintStack(&buf)
	assert.Contains(t, buf.String(), "стек пуст")

	s.PushStack("bottom")
	s.PushStack("top")
	assert.False(t, s.IsStackEmpty())

	val, ok := s.PeekStack()
	assert.True(t, ok)
	assert.Equal(t, "top", val)

	buf.Reset()
	s.PrintStack(&buf)
	assert.Contains(t, buf.String(), "top bottom")

	// Текст
	assert.True(t, s.SaveStackToFile(txtFile))
	assert.False(t, s.SaveStackToFile("/bad/stack.txt"))

	loadedTxt := NewStack()
	assert.True(t, loadedTxt.LoadStackFromFile(txtFile))
	v1, _ := loadedTxt.PopStack()
	v2, _ := loadedTxt.PopStack()
	assert.Equal(t, "top", v1)
	assert.Equal(t, "bottom", v2)
	assert.False(t, loadedTxt.LoadStackFromFile(filepath.Join(tmpDir, "no.txt")))

	// Бинарная
	assert.True(t, s.SaveStackBinary(binFile))
	assert.False(t, s.SaveStackBinary("/bad/stack.bin"))

	loadedBin := NewStack()
	assert.True(t, loadedBin.LoadStackBinary(binFile))
	b1, _ := loadedBin.PopStack()
	b2, _ := loadedBin.PopStack()
	assert.Equal(t, "top", b1)
	assert.Equal(t, "bottom", b2)
	assert.True(t, loadedBin.IsStackEmpty())
	assert.False(t, loadedBin.LoadStackBinary(filepath.Join(tmpDir, "no.bin")))

	// Битая сигнатура
	badBin := filepath.Join(tmpDir, "bad_s.bin")
	_ = os.WriteFile(badBin, []byte("BADMAGIC"), 0644)
	assert.False(t, loadedBin.LoadStackBinary(badBin))
}

func TestStack_DeepCoverage(t *testing.T) {
	var nilS *Stack
	assert.True(t, nilS.IsStackEmpty())
	_, ok := nilS.PopStack()
	assert.False(t, ok)
	_, ok = nilS.PeekStack()
	assert.False(t, ok)

	s := NewStack()
	var buf bytes.Buffer
	s.PrintStack(&buf) // пустой стек

	assert.False(t, s.SaveStackToFile("/dev/null/impossible"))
	assert.False(t, s.LoadStackFromFile("non_existent_stack.txt"))

	assert.False(t, s.SaveStackBinary("/dev/null/impossible"))
	assert.False(t, s.LoadStackBinary("non_existent_stack.bin"))

	tmp := filepath.Join(t.TempDir(), "bad_s.bin")
	os.WriteFile(tmp, []byte("BADMAGIC"), 0644)
	assert.False(t, s.LoadStackBinary(tmp))

	bufBad := append([]byte{'S', 'T', 'A', 'C', 'K', '0', '1', 0}, []byte{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}...)
	os.WriteFile(tmp, bufBad, 0644)
	assert.False(t, s.LoadStackBinary(tmp))
}

// ошибочные сценарии бинарной загрузки и сохранения стека
func TestStackAdditionalErrorPaths(t *testing.T) {
	stack := NewStack()
	dir := t.TempDir()

	// Ошибки открытия/создания файлов.
	if stack.LoadStackBinary(filepath.Join(dir, "missing.bin")) {
		t.Fatal("загрузка отсутствующего файла должна завершиться ошибкой")
	}
	if stack.SaveStackBinary(dir) {
		t.Fatal("сохранение по пути к каталогу должно завершиться ошибкой")
	}
	if stack.LoadStackFromFile(filepath.Join(dir, "missing.txt")) {
		t.Fatal("загрузка отсутствующего текстового файла должна завершиться ошибкой")
	}
	if stack.SaveStackToFile(dir) {
		t.Fatal("сохранение текстового файла по пути к каталогу должно завершиться ошибкой")
	}

	magic := []byte{'S', 'T', 'A', 'C', 'K', '0', '1', 0}
	writeUint64 := func(data []byte, value uint64) []byte {
		var encoded [8]byte
		binary.LittleEndian.PutUint64(encoded[:], value)
		return append(data, encoded[:]...)
	}
	checkInvalidFile := func(name string, data []byte) {
		t.Helper()
		filename := filepath.Join(dir, name)
		if err := os.WriteFile(filename, data, 0o600); err != nil {
			t.Fatal(err)
		}
		if stack.LoadStackBinary(filename) {
			t.Errorf("%s: ожидалась ошибка загрузки", name)
		}
	}

	checkInvalidFile("short_header.bin", []byte{1, 2})
	checkInvalidFile("wrong_magic.bin", []byte("BADMAGIC"))
	checkInvalidFile("missing_count.bin", magic)

	tooMany := writeUint64(append([]byte{}, magic...), uint64(maxStackItems+1))
	checkInvalidFile("too_many_items.bin", tooMany)

	missingLength := writeUint64(append([]byte{}, magic...), 1)
	checkInvalidFile("missing_length.bin", missingLength)

	tooLong := writeUint64(append([]byte{}, magic...), 1)
	tooLong = writeUint64(tooLong, uint64(maxStackStringSize+1))
	checkInvalidFile("too_long_string.bin", tooLong)

	truncatedString := writeUint64(append([]byte{}, magic...), 1)
	truncatedString = writeUint64(truncatedString, 2)
	truncatedString = append(truncatedString, 'x')
	checkInvalidFile("truncated_string.bin", truncatedString)

	trailingData := writeUint64(append([]byte{}, magic...), 0)
	trailingData = append(trailingData, 0xFF)
	checkInvalidFile("trailing_data.bin", trailingData)
}

// ============================================================
// 6. АВЛ-ДЕРЕВО (AVLTree)
// ============================================================

func TestAVLTree_FourRotations(t *testing.T) {
	// LL
	{
		tree := NewAVLTree()
		tree.InsertT(30)
		tree.InsertT(20)
		tree.InsertT(10)
		require.NotNil(t, tree.FindT(20))
		assert.Equal(t, 20, tree.FindT(20).Key)
		assert.Equal(t, uint8(2), tree.GetHeightT())
	}
	// LR
	{
		tree := NewAVLTree()
		tree.InsertT(30)
		tree.InsertT(10)
		tree.InsertT(20)
		require.NotNil(t, tree.FindT(20))
		assert.Equal(t, uint8(2), tree.GetHeightT())
	}
	// RR
	{
		tree := NewAVLTree()
		tree.InsertT(10)
		tree.InsertT(20)
		tree.InsertT(30)
		require.NotNil(t, tree.FindT(20))
		assert.Equal(t, uint8(2), tree.GetHeightT())
	}
	// RL
	{
		tree := NewAVLTree()
		tree.InsertT(10)
		tree.InsertT(30)
		tree.InsertT(20)
		require.NotNil(t, tree.FindT(20))
		assert.Equal(t, uint8(2), tree.GetHeightT())
	}
}

func TestAVLTree_OperationsAndTraversals(t *testing.T) {
	tmpDir := t.TempDir()
	txtFile := filepath.Join(tmpDir, "avl.txt")
	binFile := filepath.Join(tmpDir, "avl.bin")

	tree := NewAVLTree()
	assert.True(t, tree.IsEmpty())
	assert.Equal(t, 0, tree.CountNodesT())
	assert.Equal(t, uint8(0), tree.GetHeightT())

	node, steps := tree.FindWithStepsT(10)
	assert.Nil(t, node)
	assert.Equal(t, 0, steps)
	assert.Nil(t, tree.FindMinT())

	var buf bytes.Buffer
	tree.PrintInOrderT(&buf)
	tree.PrintPreOrderT(&buf)
	tree.PrintPostOrderT(&buf)
	tree.PrintLevelOrderT(&buf)
	tree.PrintTreeT(&buf)
	tree.PrintTreeInfoT(&buf, "Пустое дерево")

	values := []int{40, 20, 60, 10, 30, 50, 70, 25, 35}
	for _, val := range values {
		tree.InsertT(val)
	}

	tree.InsertT(30) // дубликат не должен добавиться
	assert.Equal(t, 9, tree.CountNodesT())
	assert.False(t, tree.IsEmpty())

	require.NotNil(t, tree.FindT(30))
	assert.Equal(t, 30, tree.FindT(30).Key)
	assert.Nil(t, tree.FindT(999))

	node, steps = tree.FindWithStepsT(25)
	assert.NotNil(t, node)
	assert.Greater(t, steps, 0)

	node, steps = tree.FindWithStepsT(999)
	assert.Nil(t, node)
	assert.Greater(t, steps, 0)

	minNode := tree.FindMinT()
	require.NotNil(t, minNode)
	assert.Equal(t, 10, minNode.Key)

	buf.Reset()
	tree.PrintInOrderT(&buf)
	tree.PrintPreOrderT(&buf)
	tree.PrintPostOrderT(&buf)
	tree.PrintLevelOrderT(&buf)
	tree.PrintTreeT(&buf)
	tree.PrintTreeInfoT(&buf, "Непустое дерево")

	// Текст
	assert.True(t, tree.SaveTreeToFile(txtFile))
	assert.False(t, tree.SaveTreeToFile("/bad/avl.txt"))

	loadedTxt := NewAVLTree()
	assert.True(t, loadedTxt.LoadTreeFromFile(txtFile))
	assert.Equal(t, tree.CountNodesT(), loadedTxt.CountNodesT())
	assert.NotNil(t, loadedTxt.FindT(25))
	assert.False(t, loadedTxt.LoadTreeFromFile(filepath.Join(tmpDir, "no.txt")))

	// Бинарная
	assert.True(t, tree.SaveTreeBinary(binFile))
	assert.False(t, tree.SaveTreeBinary("/bad/avl.bin"))

	loadedBin := NewAVLTree()
	assert.True(t, loadedBin.LoadTreeBinary(binFile))
	assert.Equal(t, tree.CountNodesT(), loadedBin.CountNodesT())
	assert.NotNil(t, loadedBin.FindT(25))
	assert.False(t, loadedBin.LoadTreeBinary(filepath.Join(tmpDir, "no.bin")))

	// Битая сигнатура
	badBin := filepath.Join(tmpDir, "bad_avl.bin")
	_ = os.WriteFile(badBin, []byte("BADMAGIC"), 0644)
	assert.False(t, loadedBin.LoadTreeBinary(badBin))

	// Удаления и перебалансировки
	tree.RemoveT(999) // несуществующий
	tree.RemoveT(10)  // лист
	tree.RemoveT(20)  // узел с двумя потомками
	tree.RemoveT(40)  // корень
	assert.Nil(t, tree.FindT(10))
	assert.Nil(t, tree.FindT(20))
	assert.Nil(t, tree.FindT(40))

	for _, v := range []int{25, 30, 35, 50, 60, 70} {
		tree.RemoveT(v)
	}
	assert.True(t, tree.IsEmpty())
	assert.Equal(t, 0, tree.CountNodesT())
}

func TestAVLTree_DeletionRebalanceCases(t *testing.T) {
	// Case 1: L-0/L-1
	{
		tree := NewAVLTree()
		for _, v := range []int{4, 2, 6, 1, 3} {
			tree.InsertT(v)
		}
		tree.RemoveT(6)
		assert.Equal(t, 4, tree.CountNodesT())
		assert.NotNil(t, tree.FindT(1))
	}
	// Case 2: L-(-1)
	{
		tree := NewAVLTree()
		for _, v := range []int{4, 2, 6, 3} {
			tree.InsertT(v)
		}
		tree.RemoveT(6)
		assert.Equal(t, 3, tree.CountNodesT())
		assert.NotNil(t, tree.FindT(2))
	}
	// Case 3: R-0/R-(-1)
	{
		tree := NewAVLTree()
		for _, v := range []int{4, 2, 6, 5, 7} {
			tree.InsertT(v)
		}
		tree.RemoveT(2)
		assert.Equal(t, 4, tree.CountNodesT())
		assert.NotNil(t, tree.FindT(7))
	}
	// Case 4: R-1
	{
		tree := NewAVLTree()
		for _, v := range []int{4, 2, 6, 5} {
			tree.InsertT(v)
		}
		tree.RemoveT(2)
		assert.Equal(t, 3, tree.CountNodesT())
		assert.NotNil(t, tree.FindT(6))
	}
}

// ============================================================
// 7. ХЕШ-ТАБЛИЦА (HashTable)
// ============================================================

func TestHashTable_BasicOperationsAndTombstones(t *testing.T) {
	table := NewHashTable(1, LinearProbing)
	assert.Equal(t, 13, table.GetCapacity())
	assert.True(t, table.IsEmpty())

	_, found := table.HashFindInt("missing")
	assert.False(t, found)
	assert.False(t, table.HashErase("missing"))

	assert.True(t, table.HashInsertInt("key", 10))
	assert.False(t, table.IsEmpty())
	assert.Equal(t, 1, table.GetSize())

	val, found := table.HashFindInt("key")
	assert.True(t, found)
	assert.Equal(t, 10, val)

	// Перезапись существующего ключа
	assert.True(t, table.HashInsertInt("key", 20))
	assert.Equal(t, 1, table.GetSize())
	val, found = table.HashFindInt("key")
	assert.True(t, found)
	assert.Equal(t, 20, val)

	// Удаление
	assert.True(t, table.HashErase("key"))
	assert.True(t, table.IsEmpty())
	_, found = table.HashFindInt("key")
	assert.False(t, found)
	assert.False(t, table.HashErase("key"))

	// Повторная вставка в удалённую ячейку
	assert.True(t, table.HashInsertInt("another", 30))
	val, found = table.HashFindInt("another")
	assert.True(t, found)
	assert.Equal(t, 30, val)
	assert.Greater(t, table.GetProbes(), uint64(0))
}

func TestHashTable_CollisionsAndDoubleHashing(t *testing.T) {
	table := NewHashTable(1, DoubleHashing)
	assert.Equal(t, 13, table.GetCapacity())

	// Заполняем таблицу до отказа
	var keys []string
	for i := 0; i < 13; i++ {
		key := "key_" + strings.Repeat("x", i)
		keys = append(keys, key)
		assert.True(t, table.HashInsertInt(key, i))
	}
	assert.Equal(t, 13, table.GetSize())

	// Попытка вставки в переполненную таблицу
	assert.False(t, table.HashInsertInt("overflow", 999))
	_, found := table.HashFindInt("overflow")
	assert.False(t, found)

	// Освобождаем ячейку и вставляем снова
	assert.True(t, table.HashErase(keys[0]))
	assert.True(t, table.HashInsertInt("new_item", 777))
	val, found := table.HashFindInt("new_item")
	assert.True(t, found)
	assert.Equal(t, 777, val)
}

func TestHashTable_BinarySerialization(t *testing.T) {
	tmpDir := t.TempDir()
	binFile := filepath.Join(tmpDir, "hashtable.bin")

	table := NewHashTable(4, DoubleHashing)
	assert.True(t, table.HashInsertInt("first", 123))
	assert.True(t, table.HashInsertInt("key with space", -7))
	assert.True(t, table.HashInsertInt("zero", 0))

	assert.True(t, table.SaveBinary(binFile))
	assert.False(t, table.SaveBinary("/bad/hashtable.bin"))

	loaded := NewHashTable(1, LinearProbing)
	assert.True(t, loaded.LoadBinary(binFile))
	assert.Equal(t, 3, loaded.GetSize())

	v1, ok1 := loaded.HashFindInt("first")
	v2, ok2 := loaded.HashFindInt("key with space")
	v3, ok3 := loaded.HashFindInt("zero")
	assert.True(t, ok1 && ok2 && ok3)
	assert.Equal(t, 123, v1)
	assert.Equal(t, -7, v2)
	assert.Equal(t, 0, v3)

	_, ok4 := loaded.HashFindInt("missing")
	assert.False(t, ok4)

	assert.False(t, loaded.LoadBinary(filepath.Join(tmpDir, "no.bin")))

	// Битая сигнатура
	badBin := filepath.Join(tmpDir, "bad_ht.bin")
	_ = os.WriteFile(badBin, []byte("BADMAGIC"), 0644)
	assert.False(t, loaded.LoadBinary(badBin))
}
func TestHashTableNilGetters(t *testing.T) {
	var table *HashTable

	assert.Equal(t, 0, table.GetSize())
	assert.Equal(t, 0, table.GetCapacity())
	assert.Equal(t, uint64(0), table.GetProbes())
	assert.Equal(t, uint64(0), table.GetCollisions())
	assert.True(t, table.IsEmpty())

	nonNilTable := NewHashTable(1, LinearProbing)
	assert.Equal(t, uint64(0), nonNilTable.GetCollisions())
}

func TestValueAsInt32Coverage(t *testing.T) {
	values := []any{
		int(1),
		int8(2),
		int16(3),
		int32(4),
		int64(5),
		uint(6),
		uint8(7),
		uint16(8),
		uint32(9),
		uint64(10),
		uintptr(11),
	}

	for _, value := range values {
		_, ok := valueAsInt32(value)
		assert.True(t, ok, "не преобразовано значение типа %T", value)
	}

	_, ok := valueAsInt32("не число")
	assert.False(t, ok)
}

func TestHashTable_DeepCoverage(t *testing.T) {
	// Неверные параметры конструктора
	htBad := NewHashTable(-10, HashMethod(99))
	assert.NotNil(t, htBad)
	htHuge := NewHashTable(2_000_000, LinearProbing)
	assert.NotNil(t, htHuge)

	// nil-вызовы и пустая таблица
	var nilHT *HashTable
	assert.False(t, nilHT.HashInsert("k", 1))
	_, found := nilHT.HashFind("k")
	assert.False(t, found)
	assert.False(t, nilHT.HashErase("k"))
	assert.False(t, nilHT.SaveBinary("/dev/null/impossible"))

	ht := NewHashTable(5, DoubleHashing)
	assert.False(t, ht.HashErase("missing"))

	// Сохранение и ошибки
	assert.False(t, ht.SaveBinary("/dev/null/impossible"))
	assert.False(t, ht.LoadBinary("non_existent_ht.bin"))

	// Сохранение неподдерживаемого типа значения (вызовет errInvalidHashFile)
	htBadVal := NewHashTable(3, LinearProbing)
	htBadVal.HashInsert("bad", "not_an_int")
	tmpBadVal := filepath.Join(t.TempDir(), "ht_bad_val.bin")
	assert.False(t, htBadVal.SaveBinary(tmpBadVal))

	// Повреждённая сигнатура
	tmp := filepath.Join(t.TempDir(), "bad_ht.bin")
	os.WriteFile(tmp, []byte("BADMAGIC"), 0644)
	assert.False(t, ht.LoadBinary(tmp))

	// Проверка функции ошибки
	assert.Equal(t, "неподдерживаемое значение или слишком длинный ключ", errInvalidHashFile.Error())
}

func TestHashTable_TextSerialization(t *testing.T) {
	filename := filepath.Join(t.TempDir(), "hash_table.txt")

	original := NewHashTable(5, DoubleHashing)
	require.True(t, original.HashInsertInt("ключ с пробелами", 42))
	require.True(t, original.HashInsertInt("второй ключ", -7))
	require.True(t, original.SaveText(filename))

	loaded := NewHashTable(1, LinearProbing)
	require.True(t, loaded.LoadText(filename))
	assert.Equal(t, DoubleHashing, loaded.method)
	assert.Equal(t, 2, loaded.GetSize())

	value, ok := loaded.HashFindInt("ключ с пробелами")
	require.True(t, ok)
	assert.Equal(t, 42, value)

	value, ok = loaded.HashFindInt("второй ключ")
	require.True(t, ok)
	assert.Equal(t, -7, value)
}
func TestHashTable_LoadText_InvalidInputs(t *testing.T) {
	dir := t.TempDir()

	tests := []struct {
		name string
		data string
	}{
		{"wrong magic", "WRONG\n0\n0\n"},
		{"invalid method", "HASHTABLE_TEXT_V1\nx\n0\n"},
		{"unsupported method", "HASHTABLE_TEXT_V1\n999\n0\n"},
		{"invalid count", "HASHTABLE_TEXT_V1\n0\nx\n"},
		{"negative count", "HASHTABLE_TEXT_V1\n0\n-1\n"},
		{"invalid key", "HASHTABLE_TEXT_V1\n0\n1\nnot-quoted\n1\n"},
		{"invalid value", "HASHTABLE_TEXT_V1\n0\n1\n\"key\"\nx\n"},
		{"trailing data", "HASHTABLE_TEXT_V1\n0\n0\nextra\n"},
		{"duplicate key", "HASHTABLE_TEXT_V1\n0\n2\n\"key\"\n1\n\"key\"\n2\n"},
	}

	table := NewHashTable(3, LinearProbing)

	for i, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			filename := filepath.Join(dir, fmt.Sprintf("invalid_%d.txt", i))
			if err := os.WriteFile(filename, []byte(tc.data), 0o600); err != nil {
				t.Fatal(err)
			}
			if table.LoadText(filename) {
				t.Fatal("LoadText() вернул true для некорректного файла")
			}
		})
	}
}
