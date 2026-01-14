// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract FirmwareRegistry {
    address public owner;       
    string public version;     
    string public url;          
    string public checksum;     
    constructor() {
        owner = msg.sender; 
    }

    modifier onlyOwner() {
        require(msg.sender == owner, "Errore: Solo il proprietario puo rilasciare aggiornamenti!");
        _;
    }

    function releaseFirmware(string memory _version, string memory _url, string memory _checksum) public onlyOwner {
        version = _version;
        url = _url;
        checksum = _checksum;
    }

    function getLatest() public view returns (string memory, string memory, string memory) {
        return (version, url, checksum);
    }
}