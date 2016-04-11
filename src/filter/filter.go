package filter

import (
	"fmt"
	"encoding/json"
	"io/ioutil"
	"strings"
)

type FilterConfig struct {
	Ignore []string `json:"ignore"`
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

	config := FilterConfig{}
	data, err := ioutil.ReadFile(fileName)
	if err != nil {
		return config, err
	}
	err = json.Unmarshal(data, &config)
	if err != nil {
		return config, err
	}

	return config, nil

	// Dummy implementation for testing. Remove later
	// newFilterConf := FilterConfig{Ignore: []string{"/dev/"}}
	// return newFilterConf, nil
}

type Filter interface {
	Start(c FilterConfig, sending chan<- []byte, receiving <-chan []byte)
}

type FSFilter struct{}

type NOPFilter struct{}

type ZachsInotifyData struct {
	Date     string `json:"DATE"`
	Event    string `json:"EVENT"`
	FilePath string `json:"PATH"`
	Type     string `json:"TYPE"`
}

func StartFilterStream(sending chan<- []byte, receiving <-chan []byte) {
	fsFilter := FSFilter{}
	nopFilter := NOPFilter{}
	conf := FilterConfig{Ignore: []string{"/dev/"}}
	link := make(chan []byte)
	go fsFilter.Start(conf, link, receiving)
	go nopFilter.Start(conf, sending, link)
}

func (f FSFilter) Start(c FilterConfig, sending chan<- []byte, receiving <-chan []byte) {
	for {
		message := <-receiving
		zid := ZachsInotifyData{}
		err := json.Unmarshal(message, &zid)
		if err != nil {
			fmt.Println("Error unmarshalling message: ", string(message),
				err.Error(), message)
			fmt.Println("Silently dropping the message")
			//panic(err)
		}
		notblacklisted := true
		for _, i := range c.Ignore {
			if strings.HasPrefix(zid.FilePath, i) {
				notblacklisted = false
				break
			}
		}
		if notblacklisted {
			sending <- message
		}
	}
}

func (N NOPFilter) Start(c FilterConfig, sending chan<- []byte, receiving <-chan []byte) {
	for {
		message := <-receiving
		sending <- message
	}
}
