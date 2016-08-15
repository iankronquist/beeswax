package filter

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"strings"
)

type FilterConfig struct {
	Ignore []string `json:"ignore"`
	IgnoreIp []string `json:"ignored ips"`
}

// Correct for a bug in znotify.
func filterLowBytes(contaminated []byte) []byte {
	// Remove all non-printing ascii characters
	for i, item := range contaminated {
		if item < byte(' ') {
			contaminated = append(contaminated[:i], contaminated[i+1:]...)
		}
	}
	// It's been cleaned
	return contaminated
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

type NetFilter struct{}

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
	conf, err := GetFilterConfig("filter_config.json")
	if err != nil {
		panic(err)
	}
	link := make(chan []byte)
	go fsFilter.Start(conf, link, receiving)
	go nopFilter.Start(conf, sending, link)
}

func StartNetFilterStream(sending chan<- []byte, receiving <-chan []byte) {
	netFilter := NetFilter{}
	nopFilter := NOPFilter{}
	conf, err := GetFilterConfig("filter_config.json")
	if err != nil {
		panic(err)
	}
	link := make(chan []byte)
	go netFilter.Start(conf, link, receiving)
	go nopFilter.Start(conf, sending, link)
}

func (f FSFilter) Start(c FilterConfig, sending chan<- []byte, receiving <-chan []byte) {
	for {
		message,ok := <-receiving
		if !ok {
			return
		}
		message = filterLowBytes(message)
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

type ipdata struct {
	Epoch       string `json:"timestamp"`
	SourceIP    string `json:"source_ip"`
	SourcePort  string `json:"source_port"`
	ReceiveIP   string `json:"dest_ip"`
	ReceivePort string `json:"dest_port"`
}

func (n NetFilter) Start(c FilterConfig, sending chan<- []byte, receiving <-chan []byte) {
	for {
		message,ok := <-receiving
		if !ok {
			return
		}
		ipdata := ipdata{}
		err := json.Unmarshal(message, &ipdata)
		if err != nil {
			fmt.Println("Error unmarshalling message: ", string(message),
				err.Error(), message)
			fmt.Println("Silently dropping the message")
			//panic(err)
		}
		notblacklisted := true
		for _, i := range c.IgnoreIp {
			if strings.HasPrefix(ipdata.SourceIP, i) ||
				strings.HasPrefix(ipdata.ReceiveIP, i) {
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
		message,ok := <-receiving
		if !ok{
			return
		}
		sending <- message
	}
}
