package main

import (
    "fmt"
    "log"
    "os/exec"
    "net/http"
    "strings"
)

const LABEL1="Valve 1"
const LABEL2="Valve 2"
const LABEL3="Valve 3"
const LABEL4="Pomp"
const LABEL5="All On"
const LABEL6="All Off"

const STATUS_SCRIPT="/usr/local/bin/status.py"
const TOGGLE_SCRIPT="/usr/local/bin/toggle.py"



var s = `<html>
<body>
<form action="status" method="post">
<center>
<table border="0" rules="none" width="100%" height="100%">
<tr>
<td bgcolor="#fdfd96" align="center" border="0" style="font-size: 25px; font-style: italic;">
<input type="submit" name="knop" value="__LABEL1__" style="font-size : 50px; width: 100%; height: 100px; background:none; padding:none; border:none;">
<br>
(__STATE1__)
</td>
<td bgcolor="#ffb347" align ="center" border="0" style="font-size: 25px; font-style: italic;">
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
<td bgcolor="#77dd77" align="center" border="0" style="font-size: 25px; font-style: italic;">
<input type="submit" name="knop" value="__LABEL4__" style="font-size : 50px; width: 100%; height: 100px; background:none; padding:none; border:none;">
<br>
(__STATE4__)
</td>
</tr>
<tr>
<td bgcolor="#ff6961" align="center" border="0">
<input type="submit" name="knop" value="__LABEL5__" style="font-size : 50px; width: 100%; height: 100px; background:none; padding:none; border:none;">
</td>
<td bgcolor="#cfcfc4" align="center" border="0">
<input type="submit" name="knop" value="__LABEL6__" style="font-size : 50px; width: 100%; height: 100px; background:none; padding:none; border:none;">
</td>
</tr>
<table>
</center>
</form>
</body>
</html>`

func getStatus(button string) string {
	var output, _ = exec.Command(STATUS_SCRIPT,button).Output()
	return string(output)
}


func createResponse() string {
	var x = strings.Replace(s, "__LABEL1__", LABEL1, -1)
	var state1 = getStatus("1") 
	var state2 = getStatus("2")
	var state3 = getStatus("3")
	var state4 = getStatus("4")

	x = strings.Replace(x, "__LABEL2__", LABEL2, -1)
	x = strings.Replace(x, "__LABEL3__", LABEL3, -1)
	x = strings.Replace(x, "__LABEL4__", LABEL4, -1)
	x = strings.Replace(x, "__LABEL5__", LABEL5, -1)
	x = strings.Replace(x, "__LABEL6__", LABEL6, -1)

	x = strings.Replace(x, "__STATE1__", state1, -1)
	x = strings.Replace(x, "__STATE2__", state2, -1)
	x = strings.Replace(x, "__STATE3__", state3, -1)
	x = strings.Replace(x, "__STATE4__", state4, -1)

	return x 
}

func handle_get_request(w http.ResponseWriter, r *http.Request) {
    fmt.Fprintf(w, createResponse() )
}

func handle_post_request(w http.ResponseWriter, r *http.Request) {
    var value = r.FormValue("knop")
    if value == LABEL1 {
	exec.Command(TOGGLE_SCRIPT, "1").Run()
    } else if value == LABEL2 {
	exec.Command(TOGGLE_SCRIPT, "2").Run()
    } else if value == LABEL3 {
	exec.Command(TOGGLE_SCRIPT, "3").Run()
    } else if value == LABEL4 {
	exec.Command(TOGGLE_SCRIPT, "4").Run()
    } else if value == LABEL5 {
	exec.Command(TOGGLE_SCRIPT, "5").Run()
    } else if value == LABEL6 {
	exec.Command(TOGGLE_SCRIPT, "6").Run()
    }

    fmt.Fprintf(w, createResponse())
}

func main() {
    http.HandleFunc("/", handle_get_request)
    http.HandleFunc("/status", handle_post_request)

    log.Fatal(http.ListenAndServe(":8081", nil))

}

