// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract FirmwareRegistry {
    // Variabili di stato (dove salviamo i dati sulla Blockchain)
    address public owner;       // L'indirizzo di chi comanda (Tu/Collega)
    string public version;      // Es: "1.0.0"
    string public url;          // Es: "http://il-tuo-server.com/firmware.bin"
    string public checksum;     // Es: "a1b2c3d4..." (Hash SHA256 del file)

    // Costruttore: Eseguito una volta sola al Deploy
    constructor() {
        owner = msg.sender; // Chi fa il deploy diventa il proprietario
    }

    // Modificatore: Protegge le funzioni critiche
    modifier onlyOwner() {
        require(msg.sender == owner, "Errore: Solo il proprietario puo rilasciare aggiornamenti!");
        _;
    }

    // FUNZIONE DI SCRITTURA (Costa Gas - Solo Owner)
    // Chiama questa funzione per pubblicare un nuovo aggiornamento
    function releaseFirmware(string memory _version, string memory _url, string memory _checksum) public onlyOwner {
        version = _version;
        url = _url;
        checksum = _checksum;
    }

    // FUNZIONE DI LETTURA (Gratuita - Per il Middleware Node.js/Python)
    // Restituisce tutti i dati in un colpo solo
    function getLatest() public view returns (string memory, string memory, string memory) {
        return (version, url, checksum);
    }
}