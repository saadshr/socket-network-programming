# Guide pédagogique du projet

## Fichiers

Le chat utilise `include/net_io.h`, `src/common/net_io.c`, `src/server/chat_server.c` et `src/client/chat_client.c`. `send_all` gère les envois partiels et `recv_line` accumule le flux TCP jusqu'à `\n`.

Le FTP utilise `src/ftp/ftp_common.h`, `ftp_common.c`, `ftp_server.c` et `ftp_client.c`. Le serveur contrôle les commandes et la sandbox Windows ; le client fournit le menu ; le module commun cadre `\r\n`, parse `PASV` et transfère les fichiers par blocs de 64 KiB. `ftp_root/.gitkeep` conserve le dossier autorisé et `tests/ftp_validation.ps1` automatise les scénarios.

## Fonctions réseau essentielles

1. `WSAStartup` initialise WinSock2.
2. `socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)` crée un socket TCP IPv4.
3. `bind` associe un serveur à une adresse et un port.
4. `listen` attend les connexions entrantes.
5. `accept` retourne le socket dédié au client accepté.
6. `connect` joint le serveur côté client.
7. `send` et `recv` échangent des octets et leurs retours doivent être contrôlés.
8. `closesocket` ferme un socket et `WSACleanup` termine WinSock2.

TCP ne connaît pas les messages applicatifs. Le chat utilise `\n`; FTP utilise `\r\n` sur le contrôle et ferme la connexion data pour signaler la fin du transfert.

## Une opération FTP

Le client reçoit `220`, envoie `USER` puis `PASS` et reçoit `331` puis `230`. Pour un fichier, il envoie `PASV`; le serveur répond `227` avec un port temporaire. Le client ouvre une seconde socket. `LIST`, `RETR` ou `STOR` restent sur le contrôle et reçoivent `150`; les octets passent sur la donnée. Après fermeture de la donnée, le serveur répond `226`.

`STOR` reçoit d'abord dans un fichier temporaire, puis utilise `MoveFileExW` sans écrasement. Les chemins absolus, `..` et `FILE_ATTRIBUTE_REPARSE_POINT` sont refusés pour rester dans `ftp_root`.

## Compilation et tests autonomes

Depuis PowerShell à la racine :

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts\build.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts\test.ps1
```

Le seul prérequis est GCC/MSYS2 accessible par `gcc`. Les scripts contrôlent les codes de sortie et arrêtent la livraison en cas d'erreur.
