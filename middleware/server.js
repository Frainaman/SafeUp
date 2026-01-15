const express = require('express');
const { ethers } = require('ethers');
const fs = require('fs');
const path = require('path');

const app = express();
const port = 5000;

const RPC_URL = "https://ethereum-sepolia.publicnode.com";

const abiPath = path.join(__dirname, '../blockchain/abi.json'); // Controlla che il nome del file sia giusto!
const addressPath = path.join(__dirname, '../blockchain/contract_address.txt');

let contract;
try {
    const contractABI = JSON.parse(fs.readFileSync(abiPath, 'utf8'));
    const contractAddress = fs.readFileSync(addressPath, 'utf8').trim();


    const provider = new ethers.JsonRpcProvider(RPC_URL);
    contract = new ethers.Contract(contractAddress, contractABI, provider);

    console.log(`Contratto caricato: ${contractAddress}`);
} catch (error) {
    console.error("ERRORE Iniziale (Controlla i file ABI/Address):", error.message);
}

app.get('/check_firmware', async (req, res) => {
    try {
        if (!contract) throw new Error("Contratto non inizializzato");

        console.log("Richiesta ricevuta dall'ESP32...");

        const result = await contract.getLatest();

        console.log("Dati grezzi dalla Blockchain:", result);

        res.json({
            version: result[0],
            url: result[1],
            checksum: result[2]
        });

        console.log("Risposta inviata correttamente!");

    } catch (error) {
        console.error("Errore durante la chiamata:", error);
        res.status(500).json({ error: error.message });
    }
});

app.listen(port, '0.0.0.0', () => {
    console.log(` Middleware Node.js attivo su http://0.0.0.0:${port}`);
});