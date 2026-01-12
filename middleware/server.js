const express = require('express');
const { ethers } = require('ethers');
const fs = require('fs');
const path = require('path');

const app = express();
const port = 5000;

// --- CONFIGURAZIONE ---
const RPC_URL = "https://ethereum-sepolia.publicnode.com"; // O il tuo provider (Infura/Alchemy)

// Percorsi ai file (assumendo la stessa struttura cartelle di prima)
const abiPath = path.join(__dirname, '../blockchain/abi.json');
const addressPath = path.join(__dirname, '../blockchain/contract_address.txt');

// --- CARICAMENTO DATI ---
let contract;
try {
    // Leggi ABI e Indirizzo
    const contractABI = JSON.parse(fs.readFileSync(abiPath, 'utf8'));
    const contractAddress = fs.readFileSync(addressPath, 'utf8').trim();

    // Connessione a Blockchain
    const provider = new ethers.JsonRpcProvider(RPC_URL);
    contract = new ethers.Contract(contractAddress, contractABI, provider);

    console.log(`✅ Contratto caricato: ${contractAddress}`);
} catch (error) {
    console.error("❌ ERRORE Iniziale:", error.message);
}

// --- ROTTE API ---
app.get('/check_firmware', async (req, res) => {
    try {
        if (!contract) throw new Error("Contratto non inizializzato");

        // Chiama la funzione 'getLatest' dallo Smart Contract
        // NOTA: Ethers.js restituisce un array o un oggetto
        const data = await contract.getLatest();

        // Rispondiamo all'ESP32
        res.json({
            version: data[0],
            url: data[1],
            checksum: data[2]
        });

    } catch (error) {
        console.error("Errore durante la chiamata:", error);
        res.status(500).json({ error: error.message });
    }
});

// --- AVVIO SERVER ---
app.listen(port, '0.0.0.0', () => {
    console.log(`🚀 Middleware Node.js attivo su http://0.0.0.0:${port}`);
});