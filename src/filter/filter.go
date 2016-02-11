package filter
import (
	"encoding/json"
	"strings"
)

type FilterConfig struct {
	ignore []string `json:"ignore"`
}

func GetFilterConfig(fileName string) (FilterConfig, error) {
	/* Takes a json file name as a string. Reads the file, unmarshals json.
	Returns the unmarshalled FilterConfig object
	See configurator.ReadConfig for an example
	In detail:
	1. Read all of the file with ioutil.ReadFile
	2. Instantiate an empty config object.
	3. Unmarshal the byte array which is the file's contents as json into the
	new config object using jsom.Unmarshal.
	4. Return the config object.
	If there is an error, return it.
	*/
	// Dummy implementation for testing. Remove later
	newFilterConf := FilterConfig{ignore: []string{"/dev/"}}
	return newFilterConf, nil
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
