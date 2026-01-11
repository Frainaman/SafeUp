from flask import Flask, jsonify
from web3 import Web3
import json

app = Flask(__name__)

@app.route('/')
def home():
    return "<h1>Il server funziona! Ora vai su /check</h1>"

# --- CONFIGURAZIONE ---

# 1. Ci colleghiamo alla rete Sepolia (Uso Ankr perché è più stabile di rpc.sepolia.org)
sepolia_rpc_url = "https://ethereum-sepolia-rpc.publicnode.com"
web3 = Web3(Web3.HTTPProvider(sepolia_rpc_url))

# 2. INCOLLA QUI IL NUOVO INDIRIZZO (quello di FirmwareRegistry preso da Remix)
# Deve rimanere tra le virgolette!
contract_address = "0xA1559910135bBD8F67E51c1Ab651Bd82c650696C" 

# 3. INCOLLA QUI LA TUA NUOVA ABI (Tutta quanta, copiata da Remix)
# Incollala tra le tre virgolette qui sotto. Non preoccuparti se va a capo.
abi_string = """
[
	{
		"inputs": [
			{
				"internalType": "string",
				"name": "_version",
				"type": "string"
			},
			{
				"internalType": "string",
				"name": "_url",
				"type": "string"
			},
			{
				"internalType": "string",
				"name": "_checksum",
				"type": "string"
			}
		],
		"name": "releaseFirmware",
		"outputs": [],
		"stateMutability": "nonpayable",
		"type": "function"
	},
	{
		"inputs": [],
		"stateMutability": "nonpayable",
		"type": "constructor"
	},
	{
		"inputs": [],
		"name": "checksum",
		"outputs": [
			{
				"internalType": "string",
				"name": "",
				"type": "string"
			}
		],
		"stateMutability": "view",
		"type": "function"
	},
	{
		"inputs": [],
		"name": "getLatest",
		"outputs": [
			{
				"internalType": "string",
				"name": "",
				"type": "string"
			},
			{
				"internalType": "string",
				"name": "",
				"type": "string"
			},
			{
				"internalType": "string",
				"name": "",
				"type": "string"
			}
		],
		"stateMutability": "view",
		"type": "function"
	},
	{
		"inputs": [],
		"name": "owner",
		"outputs": [
			{
				"internalType": "address",
				"name": "",
				"type": "address"
			}
		],
		"stateMutability": "view",
		"type": "function"
	},
	{
		"inputs": [],
		"name": "url",
		"outputs": [
			{
				"internalType": "string",
				"name": "",
				"type": "string"
			}
		],
		"stateMutability": "view",
		"type": "function"
	},
	{
		"inputs": [],
		"name": "version",
		"outputs": [
			{
				"internalType": "string",
				"name": "",
				"type": "string"
			}
		],
		"stateMutability": "view",
		"type": "function"
	}
]
"""

# Convertiamo la stringa ABI in JSON
try:
    contract_abi = json.loads(abi_string)
    # Creiamo l'oggetto contratto
    contract = web3.eth.contract(address=contract_address, abi=contract_abi)
    print("✅ Configurazione Blockchain OK")
except Exception as e:
    print(f"⚠️ Errore nella configurazione (hai incollato l'ABI giusta?): {e}")


@app.route('/check', methods=['GET'])
def check_update():
    if not web3.is_connected():
        return jsonify({"error": "Errore connessione Blockchain"}), 500

    try:
        print("Sto leggendo dalla Blockchain...")
        
        # Chiama la funzione 'getLatest' dallo Smart Contract
        # Restituisce: (versione, url, checksum)
        dati = contract.functions.getLatest().call()
        
        # Costruiamo la risposta per l'ESP32
        risposta = {
            "version": dati[0],
            "url": dati[1],
            "checksum": dati[2]
        }
        
        print(f"✅ Dati trovati: {risposta}")
        return jsonify(risposta)
        
    except Exception as e:
        print(f"❌ Errore durante la lettura: {e}")
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    # Avvia il server visibile sulla rete locale
    app.run(host='0.0.0.0', port=5000)