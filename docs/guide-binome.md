# Guide du binôme

Ce document explique le projet de zéro, sans supposer de connaissances préalables en sockets.

## 1. Les fichiers à connaître

| Fichier | Rôle |
|---|---|
| `include/net_io.h` | Interface commune du cadrage TCP et de `send_all`. |
| `src/common/net_io.c` | Envois complets et lecture de lignes dans un flux TCP. |
| `src/server/chat_server.c` | Serveur chat sur `127.0.0.1:5000`. |
| `src/client/chat_client.c` | Client chat interactif et commande `/quit`. |
| `src/ftp/ftp_common.h/.c` | Constantes, lignes FTP, PASV et transferts par blocs. |
| `src/ftp/ftp_server.c` | Serveur FTP, commandes, contrôle du sandbox et fichiers Windows. |
| `src/ftp/ftp_client.c` | Client FTP terminal et menu. |
| `ftp_root/.gitkeep` | Racine autorisée du FTP ; les fichiers de test restent locaux. |
| `scripts/build.ps1` | Compile les quatre exécutables avec les options imposées. |
| `scripts/test.ps1` | Compile puis exécute la régression chat et la validation FTP. |
| `tests/ftp_validation.ps1` | Scénarios FTP, sécurité des chemins, SHA-256 et curl. |
| `tests/validation.md` | Résultats réels et reproductibles. |

## 2. Le cycle socket TCP

Un socket est une extrémité de communication. Le serveur appelle `WSAStartup`, `socket`, `bind`, `listen`, puis `accept`. Le client appelle `WSAStartup`, `socket`, puis `connect`. Les deux côtés échangent des octets avec `send` et `recv`, ferment avec `closesocket`, puis appellent `WSACleanup`.

`bind` associe le serveur à une adresse et un port. `listen` prépare la file des connexions. `accept` crée une socket dédiée à un client ; le socket d'écoute reste disponible jusqu'à la fin de la session.

TCP est fiable et ordonné, mais ne connaît pas les messages. Le chat termine chaque ligne par `\n`. `recv_line` accumule les fragments et conserve les octets supplémentaires lorsqu'un seul `recv` contient plusieurs lignes. `send_all` boucle parce qu'un appel à `send` peut être partiel.

## 3. Le chat

Dans un terminal, lancer `build/chat_server.exe`, puis dans un autre `build/chat_client.exe`. Le client envoie plusieurs lignes et attend un accusé après chacune. `/quit` est envoyé comme une ligne normale, reçoit `Serveur: au revoir !`, puis ferme proprement la connexion.

Le serveur est volontairement mono-session : après la déconnexion ou `/quit`, il termine et doit être relancé pour une nouvelle démonstration.

## 4. Le FTP

Le serveur FTP écoute `127.0.0.1:2121`. Il reçoit les commandes sur une connexion de contrôle. Pour `LIST`, `RETR` ou `STOR`, le client envoie `PASV`, reçoit `227`, puis ouvre une deuxième connexion TCP vers le port annoncé. Le contrôle transporte ensuite la commande et les codes `150` puis `226`; la connexion data transporte le listing ou les octets.

La séquence minimale est :

```text
220 -> USER -> 331 -> PASS -> 230 -> TYPE I -> 200
PASV -> 227 -> connexion data -> LIST/RETR/STOR -> 150 -> octets -> 226
QUIT -> 221
```

Commandes disponibles : `USER`, `PASS`, `SYST`, `FEAT`, `TYPE I`, `PWD`, `PASV`, `LIST`, `RETR`, `STOR`, `NOOP` et `QUIT`. `EPSV` est aussi accepté pour `curl.exe`.

Les chemins FTP sont relatifs à `ftp_root`. Les chemins absolus, `..`, les composants reparse point et les destinations déjà existantes sont refusés. `STOR` reçoit d'abord les octets dans un fichier temporaire, puis le déplace sans écraser une destination. Les transferts utilisent des blocs de 64 KiB et contrôlent les envois partiels.

Le mot de passe est injecté par `SOCKETHUB_FTP_PASSWORD`; il ne doit jamais être commité. Les journaux affichent `PASS [masqué]`, jamais sa valeur. Le service reste limité à localhost car FTP classique n'est pas chiffré.

## 5. Compiler et tester

Depuis PowerShell à la racine :

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts\build.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts\test.ps1
```

Le seul prérequis est GCC/MSYS2 dans le `PATH`; le script ajoute `-lws2_32` et arrête immédiatement la procédure en cas d'erreur. Les résultats documentés sont dans [validation.md](../tests/validation.md).

## 6. Réponses courtes à l'oral

- **Pourquoi TCP ?** Pour un flux fiable et ordonné.
- **Pourquoi deux connexions FTP ?** Une pour le contrôle, une pour les données.
- **Pourquoi `127.0.0.1` ?** La démonstration reste locale et n'expose pas FTP sur le réseau.
- **Quel est le rôle de DNS ?** DNS traduit normalement un nom en IP ; ici l'adresse IP locale est écrite directement.
- **Pourquoi `550` ?** Le fichier ou le chemin est refusé, inexistant ou hors sandbox.
- **Pourquoi pas le port 21 ?** Le port 2121 évite les privilèges administrateur.
