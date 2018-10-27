var xhrObject = false;

xhrObject = new XMLHttpRequest();

function loop(){
    if(xhrObject) {
        xhrObject.open("GET","http://192.168.0.10:5000/api/getinfo");

        xhrObject.onreadystatechange = function () {
            if(xhrObject.readyState == 4 && xhrObject.status == 200) {
                postMessage(xhrObject.responseText);
                setTimeout(loop(), 10000);					
            } 
        }
        xhrObject.send(null);
	}
} 
loop();