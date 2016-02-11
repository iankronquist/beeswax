package filter
import (
	"encoding/json"
	"strings"
)

type FilterConfig struct {
	ignore []string `json:"ignore"`
}

type Filter interface {
	Start(c FilterConfig, sending chan<- string, receiving <-chan string)
}

type FSFilter struct { }

type NOP struct { }

type ZachsInotifyData struct {
	Date	 string `json:"DATE"`
	Event	 string `json:"EVENT"`
	FilePath string `json:"PATH"`
	Type	 string `json:"TYPE"`
}

func (f FSFilter) Start (c FilterConfig, sending chan<- string, receiving <-chan string){
	for {
		select {
			case message := <-receiving:
				zid := ZachsInotifyData{}
				err := json.Unmarshal([]byte(message), &zid)
				if err != nil {
					panic(err)
				}
				notblacklisted := true
				for _, i := range c.ignore {
					if strings.HasPrefix(zid.FilePath,i) {
						notblacklisted = false
						break
					}
				}
				if notblacklisted {
					sending <- message
				}
		}
	}

}

func (N NOP) Start( c FilterConfig, sending chan<- string, receiving <-chan string){
	for {
		select{
			case message := <- receiving:
				sending <- message
		}
	}

}
