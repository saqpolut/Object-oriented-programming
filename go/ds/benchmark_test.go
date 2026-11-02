package ds

import "testing"

func BenchmarkMassivAddToEnd(b *testing.B) {
	array := NewArray()
	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		array.AddToEnd("value")
	}
}

func BenchmarkFListPushBack(b *testing.B) {
	list := NewFList()
	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		list.PushBackF("value")
	}
}

func BenchmarkDListPushBack(b *testing.B) {
	list := NewDList()
	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		list.PushBackD("value")
	}
}

func BenchmarkQueuePushPop(b *testing.B) {
	queue := NewQueue()
	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		queue.PushQueue("value")
		queue.PopQueue()
	}
}

func BenchmarkStackPushPop(b *testing.B) {
	stack := NewStack()
	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		stack.PushStack("value")
		stack.PopStack()
	}
}

func BenchmarkAVLInsert(b *testing.B) {
	tree := NewAVLTree()
	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		tree.InsertT(i)
	}
}

func BenchmarkHashFind(b *testing.B) {
	table := NewHashTable(1, LinearProbing)
	table.HashInsertInt("key", 42)
	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		table.HashFindInt("key")
	}
}
