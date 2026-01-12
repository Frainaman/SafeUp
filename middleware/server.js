const express = require('express');
const { ethers } = require('ethers');
const fs = require('fs');
const path = require('path');

const app = express();
const port = 5000;

// --- CONFIGURAZIONE ---
const RPC_URL = "https://ethereum-sepolia.publicnode.com";

// Percorsi ai file
const abiPath = path.join(__dirname, '../blockchain/abi.json'); // Controlla che il nome del file sia giusto!
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
    console.error("❌ ERRORE Iniziale (Controlla i file ABI/Address):", error.message);
}

// --- ROTTE API ---
app.get('/check_firmware', async (req, res) => {
    try {
        if (!contract) throw new Error("Contratto non inizializzato");

        console.log("🔍 Richiesta ricevuta dall'ESP32...");

        // 1. CORREZIONE NOME FUNZIONE:
        // Usa getFirmware() (nuovo contratto) invece di getLatest()
        const result = await contract.getLatest();

        console.log("📦 Dati grezzi dalla Blockchain:", result);

        // 2. CORREZIONE VARIABILE:
        // Usiamo 'result' perché è lì che abbiamo salvato i dati sopra
        res.json({
            version: result[0],
            url: result[1],
            checksum: result[2]
        });

        console.log("✅ Risposta inviata correttamente!");

    } catch (error) {
        console.error("❌ Errore durante la chiamata:", error);
        res.status(500).json({ error: error.message });
    }
});

// --- AVVIO SERVER ---
app.listen(port, '0.0.0.0', () => {
    console.log(`🚀 Middleware Node.js attivo su http://0.0.0.0:${port}`);
});