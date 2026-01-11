const express = require('express');
const { ethers } = require('ethers');
const fs = require('fs');
const path = require('path');

const app = express();
const port = 5000;

// --- 1. MODIFICA: ABILITA LA CARTELLA PUBLIC ---
// Questo permette di scaricare il file firmware.bin che hai appena copiato
app.use(express.static(path.join(__dirname, 'public')));

// --- CONFIGURAZIONE ---
const RPC_URL = "https://ethereum-sepolia-rpc.publicnode.com";

// Percorsi ai file
const abiPath = path.join(__dirname, '../blockchain/abi.json');
const addressPath = path.join(__dirname, '../blockchain/contract_address.txt');

// --- CARICAMENTO DATI ---
let contract;

try {
    if (!fs.existsSync(abiPath) || !fs.existsSync(addressPath)) {
        throw new Error("File ABI o Indirizzo non trovati nei percorsi specificati.");
    }

    const contractABI = JSON.parse(fs.readFileSync(abiPath, 'utf8'));
    const contractAddress = fs.readFileSync(addressPath, 'utf8').trim();

    const provider = new ethers.JsonRpcProvider(RPC_URL);
    contract = new ethers.Contract(contractAddress, contractABI, provider);

    console.log(`✅ Contratto caricato: ${contractAddress}`);

} catch (error) {
    console.error(`❌ ERRORE Iniziale: ${error.message}`);
}

// --- ROTTE API ---
app.get('/check_firmware', async (req, res) => {
    try {
        if (!contract) {
            return res.status(503).json({ error: "Servizio Blockchain non disponibile" });
        }

        console.log("📡 Richiesta ricevuta dall'ESP32...");

        // Chiama la funzione 'getLatest' dallo Smart Contract
        const data = await contract.getLatest();
        
        // --- 2. MODIFICA: FORZIAMO L'URL CORRETTO ---
        // Anche se la Blockchain ha un URL vecchio, noi inviamo quello funzionante di Ngrok.
        // NOTA: Sostituisci 'fractus-...' con il tuo indirizzo Ngrok ATTUALE se è cambiato!
        
        const responseData = {
            version: data[0].toString(), // Prende la versione dalla Blockchain
            
            // 👇 QUI INSERIAMO IL TUO NGROK PER IL DOWNLOAD 👇
            url: "https://fractus-jeremiah-nonrupturable.ngrok-free.dev/firmware.bin",
            // Invece di usare data[1] (che potrebbe essere sbagliato), usiamo il link diretto al tuo PC.

            checksum: data[2] // Prende il checksum dalla Blockchain
        };

        console.log("📤 Invio dati:", responseData);
        res.json(responseData);

    } catch (error) {
        console.error("❌ Errore durante la chiamata RPC:", error.message);
        res.status(500).json({ error: "Errore nel recupero dati dalla Blockchain" });
    }
});

// --- AVVIO SERVER ---
app.listen(port, '0.0.0.0', () => {
    console.log(`🚀 Middleware Node.js attivo su http://0.0.0.0:${port}`);
    console.log(`📂 Servendo file statici dalla cartella 'public'`);
});