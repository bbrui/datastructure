package main

import (
	"./list"
	"fmt"
)

func main()  {

	l := list.New()
	l.Init()
	l.PushBack(1)
	l.PushBack(4)
	l.PushBack(0)
	l.PushBack(9)

	fmt.Println("Len=", l.Len())
	for e:=l.Front(); e != nil ; e = e.Next(){
		fmt.Println(e.Value)
		//l.Remove(e)
	}

	for e:=l.Front();e != nil;e = l.Front() {
		l.Remove(e)
	}
	fmt.Println("Len=", l.Len())

}
