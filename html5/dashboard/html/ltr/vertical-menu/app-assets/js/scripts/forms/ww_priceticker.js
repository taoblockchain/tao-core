var xhrObject = false;

xhrObject = new XMLHttpRequest();

function loop(){
    if(xhrObject) {
        xhrObject.open("GET","https://api.coinmarketcap.com/v2/ticker/1391/?convert=BTC");

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