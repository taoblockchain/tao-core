from flask import Flask, jsonify, send_from_directory

from bitcoinrpc.authproxy import AuthServiceProxy, JSONRPCException
from config import *
import json,decimal
from io import StringIO
from geolite2 import geolite2
import datetime
from ipaddress import ip_address
from flask_cors import CORS,cross_origin
import logging
import netifaces as ni

def rpc_connect():
	return AuthServiceProxy("http://%s:%s@127.0.0.1:%s"%(rpcuser, rpcpassword, rpcport))

def decimal_default(obj):
    if isinstance(obj, decimal.Decimal):
        return float(obj)
    raise TypeError

def normalize_result(strng):
	io = StringIO()
	x=json.dump(strng, io, default=decimal_default)
	return jsonify(io.getvalue())	

logging.getLogger('flask_cors').level = logging.DEBUG
app = Flask(__name__, static_url_path='', static_folder='dashboard/html/ltr/vertical-menu/')
CORS(app, resources={r"/api/*": {"origins": "*"}})

@app.route("/api/getinfo", methods=['GET'])
def getinfo():
	return normalize_result(rpc_connect().getinfo())

@app.route("/api/getstakinginfo", methods=['GET'])
def getstakinginfo():
	return normalize_result(rpc_connect().getstakinginfo())

@app.route("/api/listtransactions", methods=['GET'])
def listtransactions():
	return normalize_result(rpc_connect().listsinceblock())

@app.route("/api/listaddresses", methods=['GET'])
def listaddresses():
	return 

@app.route("/api/getnewaddress", methods=['GET'])
def getnewaddress():
	return normalize_result(rpc_connect().getnewaddress())

@app.route("/api/getrawtx", methods=['GET'])
def getrawtx():
	return normalize_result(rpc_connect().getrawtransaction(txid))

@app.route("/api/getrawmempool", methods=['GET'])
def getrawmempool():
	return normalize_result(rpc_connect().getrawmempool())

@app.route("/api/getpeerinfo", methods=['GET'])
@cross_origin()
def getpeerinfo():
	peers = normalize_result(rpc_connect().getpeerinfo())
	peers = json.loads(json.loads(peers.response[0]))
	peer_list = []
	for peer in peers:
		ip, separator, port = peer["addr"].rpartition(':')
		port = int(port) # convert to integer
		ip = ip_address(ip.strip("[]")) # convert to `IPv4Address` or `IPv6Address` 
		reader = geolite2.reader()
		match = reader.get(str(ip))
		geolite2.close()

		out = {}
		out['client'] = peer['subver']
		out['ip'] = peer['addr']
		out['last_seen'] = datetime.datetime.fromtimestamp(int(peer['lastrecv'])).strftime('%Y-%m-%d %H:%M:%S')			
		out['connected_since'] = datetime.datetime.fromtimestamp(int(peer['conntime'])).strftime('%Y-%m-%d %H:%M:%S')			
		out['ping'] = str(peer['pingtime'] * 1000) + "ms"
		if match is not None:
			out['location']=match
		peer_list.append(out)
	return jsonify(peer_list)

@app.route('/js/<path:path>')
def send_js(path):
    return send_from_directory('js', path)
"""
@app.route('/app-assets/<path:path>')
def send_app_assets(path):
	print(path)
	return send_from_directory('app-assets', path)

@app.route('/assets/<path:path>')
def send_assets(path):
    return send_from_directory('assets', path)
"""
@app.route("/")
def index():
	return send_from_directory('dashboard/html/ltr/vertical-menu/','index.html')

#api.add_resource(getrawtx, '/getrawtx/<string:txid>')

if __name__ == '__main__':
    app.run(debug=True,host='0.0.0.0')

#@app.route("/")
#def index():
#    return "Hello World!"F