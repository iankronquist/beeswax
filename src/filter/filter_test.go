package filter

import "testing"

func TestStartFSFilter (t *testing.T) {
	sending2 := make(chan []byte)
	receiving2 := make(chan []byte)

	f := FSFilter{}
	c := FilterConfig{Ignore:[]string{"Test","Ignore","Filter"}}
	
	test_z :=[]string{
"{\"DATE\":\"1\",\"EVENT\":\"IN_OPEN\",\"PATH\":\"Testedasd\",\"TYPE\":\"FOLDER\"}",
"{\"DATE\":\"1\",\"EVENT\":\"IN_OPEN\",\"PATH\":\"Ignored\",\"TYPE\":\"FOLDER\"}",
"{\"DATE\":\"1\",\"EVENT\":\"IN_OPEN\",\"PATH\":\"Filtered\",\"TYPE\":\"FOLDER\"}",
"{\"DATE\":\"1\",\"EVENT\":\"IN_OPEN\",\"PATH\":\"Passing\",\"TYPE\":\"FOLDER\"}",
"{\"DATE\":\"1\",\"EVENT\":\"IN_OPEN\",\"PATH\":\"Parented\",\"TYPE\":\"FOLDER\"}",
"{\"DATE\":\"1\",\"EVENT\":\"IN_OPEN\",\"PATH\":\"NotFilter\",\"TYPE\":\"FOLDER\"}",
}
	results:=[]bool{false,false,false,true,true,true}
	go f.Start(c,receiving2,sending2)
	
	for index,item := range test_z {
		sending2<-[]byte(item)
		if results[index] {
			bob := <-receiving2
			if !(string(bob) == item){
				t.Error("Wrong Result Received")
			}
		}
	}	
	close(sending2)
	close(receiving2)	
}

func TestStartNOPFilter (t *testing.T) {
	sending3 := make(chan []byte)
	receiving3 := make(chan []byte)

	n := NOPFilter{}
	c := FilterConfig{Ignore:[]string{"Test","Ignore","Filter"}}
	
	test_z :=[]string{
"{\"DATE\":\"1\",\"EVENT\":\"IN_OPEN\",\"PATH\":\"Testedasd\",\"TYPE\":\"FOLDER\"}",
"{\"DATE\":\"1\",\"EVENT\":\"IN_OPEN\",\"PATH\":\"Ignored\",\"TYPE\":\"FOLDER\"}",
"{\"DATE\":\"1\",\"EVENT\":\"IN_OPEN\",\"PATH\":\"Filtered\",\"TYPE\":\"FOLDER\"}",
"{\"DATE\":\"1\",\"EVENT\":\"IN_OPEN\",\"PATH\":\"Passing\",\"TYPE\":\"FOLDER\"}",
"{\"DATE\":\"1\",\"EVENT\":\"IN_OPEN\",\"PATH\":\"Parented\",\"TYPE\":\"FOLDER\"}",
"{\"DATE\":\"1\",\"EVENT\":\"IN_OPEN\",\"PATH\":\"NotFilter\",\"TYPE\":\"FOLDER\"}",
}
	go n.Start(c,receiving3,sending3)
	
	for _,item := range test_z {
		sending3<-[]byte(item)
		bob := <-receiving3
		if !(string(bob) == item){
			t.Error("Wrong Result Received")
		}
	}	
	close(sending3)
	close(receiving3)	
}
