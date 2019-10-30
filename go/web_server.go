package main

import (
    "fmt"
    "log"
    "os/exec"
    "net/http"
    "strings"
)

var counter int
var relay1 = false
var relay2 = false 
var relay3 = false 
var relay4 = false

const LABEL1="Circuit 1"
const LABEL2="Circuit 2"
const LABEL3="Circuit 3"
const LABEL4="Pomp"
const LABEL5="Alles Aan"
const LABEL6="Alles uit"



var s = `<html>
<body>
<form action="doe_iet" method="post">
<center>
<table border="0" width="100%" height="100%" class="buttons">
<tr>
<td bgcolor="#fdfd96" align="center" border="0" style="font-size: 25px; font-style: italic;">
<input type="submit" name="knop" value="__LABEL1__" style="font-size : 50px; width: 100%; height: 100px; background:none; padding:none; border:none;">
<br>
(__STATE1__)
</td>
<td bgcolor="#ffa500" align ="center" border="0" style="font-size: 25px; font-style: italic;">
<input type="submit" name="knop" value="__LABEL2__" style="font-size : 50px; width: 100%; height: 100px; background:none; padding:none; border:none;">
<br>
(__STATE2__)
</td>
</tr>
<tr>
<td bgcolor="#87ceeb" align="center" border="0" style="font-size: 25px; font-style: italic;">
<input type="submit" name="knop" value="__LABEL3__" style="font-size : 50px; width: 100%; height: 100px; background:none; padding:none; border:none;">
<br>
(__STATE3__)
</td>
<td bgcolor="#adff2f" align="center" border="0" style="font-size: 25px; font-style: italic;">
<input type="submit" name="knop" value="__LABEL4__" style="font-size : 50px; width: 100%; height: 100px; background:none; padding:none; border:none;">
<br>
(__STATE4__)
</td>
</tr>
<tr>
<td bgcolor="#cd5c5c" align="center" border="0">
<input type="submit" name="knop" value="__LABEL5__" style="font-size : 50px; width: 100%; height: 100px; background:none; padding:none; border:none;">
</td>
<td bgcolor="#ffbcd9" align="center" border="0">
<input type="submit" name="knop" value="__LABEL6__" style="font-size : 50px; width: 100%; height: 100px; background:none; padding:none; border:none;">
</td>
</tr>
<table>
</center>
</form>
</body>
</html>`


func createResponse() string {
	var x = strings.Replace(s, "__LABEL1__", LABEL1, -1)
	var state1 = "Inactive"
	var state2 = "Inactive"
	var state3 = "Inactive"
	var state4 = "Inactive"

	x = strings.Replace(x, "__LABEL2__", LABEL2, -1)
	x = strings.Replace(x, "__LABEL3__", LABEL3, -1)
	x = strings.Replace(x, "__LABEL4__", LABEL4, -1)
	x = strings.Replace(x, "__LABEL5__", LABEL5, -1)
	x = strings.Replace(x, "__LABEL6__", LABEL6, -1)


	if relay1 {
		state1="Active" 
	}

	if relay2 {
		state2="Active" 
	}

	if relay3 {
		state3="Active" 
	}

	if relay4 {
		state4="Active" 
	}

	x = strings.Replace(x, "__STATE1__", state1, -1)
	x = strings.Replace(x, "__STATE2__", state2, -1)
	x = strings.Replace(x, "__STATE3__", state3, -1)
	x = strings.Replace(x, "__STATE4__", state4, -1)

	return x 
}

func echoString(w http.ResponseWriter, r *http.Request) {
    fmt.Fprintf(w, createResponse() )
}

func updateState(name string) {
	if name == LABEL1 {
		relay1 = !relay1
	} else if name == LABEL2 {
		relay2 = !relay2
	} else if name == LABEL3 {
		relay3 = !relay3
	} else if name == LABEL4 {
		relay4 = !relay4
	} else if name == LABEL5 {
		relay1 = true
		relay2 = true
		relay3 = true
		relay4 = true
	} else if name == LABEL6 {
		relay1 = false
		relay2 = false
		relay3 = false
		relay4 = false
	}

}


func test(w http.ResponseWriter, r *http.Request) {
    var value = r.FormValue("knop")
    if value == LABEL1 {
	cmd := exec.Command("/usr/local/bin/toggle.py", "1")
	cmd.Run()
    } else if value == LABEL2 {
	cmd := exec.Command("/usr/local/bin/toggle.py", "2")
	cmd.Run()
    } else if value == LABEL3 {
	cmd := exec.Command("/usr/local/bin/toggle.py", "3")
	cmd.Run()
    } else if value == LABEL4 {
	cmd := exec.Command("/usr/local/bin/toggle.py", "4")
	cmd.Run()
    } else if value == LABEL5 {
	cmd := exec.Command("/usr/local/bin/toggle.py", "5")
	cmd.Run()
    } else if value == LABEL6 {
	cmd := exec.Command("/usr/local/bin/toggle.py", "6")
	cmd.Run()
    }
    updateState(value)

    fmt.Fprintf(w, createResponse())
}

func main() {
    http.HandleFunc("/", echoString)
    http.HandleFunc("/doe_iet", test)

    log.Fatal(http.ListenAndServe(":8081", nil))

}

