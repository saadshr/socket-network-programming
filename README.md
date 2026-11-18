# SocketHub-C

SocketHub-C est un projet universitaire de programmation réseau native en C sous Windows. Le but final est de construire une messagerie client/serveur TCP puis un mini-client et un mini-serveur FTP.

Cette version valide uniquement le chat TCP local. Le FTP, l'UDP et l'application de temps ne sont pas implémentés.

## Fonctionnalités terminées

- serveur TCP sur `127.0.0.1:5000` ;
- client TCP et échanges de plusieurs messages ;
- commande `/quit` et détection des déconnexions ;
- vérifications WinSock2 et nettoyage avec `closesocket`/`WSACleanup` ;
- envois partiels gérés par `send_all` ;
- messages cadrés par ligne avec `recv_line` ;
- messages TCP fragmentés ou regroupés correctement traités.

Le serveur accepte un client par exécution et se termine après la session. Il faut le relancer pour une nouvelle démonstration.

## Fonctionnalités FTP à venir

Les éléments suivants sont prévus mais **non implémentés** : `USER`, `PASS`, `PASV`, `LIST`, `RETR`, `STOR`, `QUIT`, connexion de contrôle, connexion de données passive, restriction à un répertoire autorisé et protection contre `..`. Voir [src/ftp/README.md](src/ftp/README.md).

## Technologies et prérequis

- C natif, sans framework ;
- WinSock2 et bibliothèque `ws2_32` ;
- Windows avec MSYS2 UCRT64 ou MinGW 64-bit ;
- GCC accessible depuis le terminal ;
- deux terminaux PowerShell.

Vérifier GCC avec `gcc --version`.

## Compilation exacte

Depuis la racine du dépôt dans MSYS2 :

```bash
mkdir -p build
gcc -Wall -Wextra -Wpedantic -std=c11 -Iinclude src/common/net_io.c src/server/chat_server.c -o build/chat_server.exe -lws2_32
gcc -Wall -Wextra -Wpedantic -std=c11 -Iinclude src/common/net_io.c src/client/chat_client.c -o build/chat_client.exe -lws2_32
```

## Exécution PowerShell

Terminal 1 :

```powershell
Set-Location C:\chemin\vers\socket-network-programming
.\build\chat_server.exe
```

Terminal 2 :

```powershell
Set-Location C:\chemin\vers\socket-network-programming
.\build\chat_client.exe
```

Saisir plusieurs messages puis `/quit`.

Échange réel observé :

```text
Client : premier message
Serveur: reçu -> premier message
Client : deuxième message
Serveur: reçu -> deuxième message
Client : /quit
Serveur: au revoir !
```

## Arborescence

```text
socket-network-programming/
├── include/net_io.h
├── src/client/chat_client.c
├── src/common/net_io.c
├── src/server/chat_server.c
├── src/ftp/README.md
├── tests/README.md
└── docs/
    ├── architecture.md
    ├── protocol.md
    ├── demo.md
    ├── faq.md
    └── screenshots/README.md
```

## Documentation

- [Architecture](docs/architecture.md)
- [Protocole TCP](docs/protocol.md)
- [Guide de démonstration](docs/demo.md)
- [FAQ](docs/faq.md)
- [Captures à réaliser](docs/screenshots/README.md)
- [Résultats de validation](tests/validation.md)

## Limites connues

- un seul client par exécution du serveur ;
- accusé de réception simple, pas encore de chat multi-client ;
- lignes limitées par un tampon de 4096 octets ;
- aucune authentification ni chiffrement ;
- écoute limitée à `127.0.0.1` ;
- aucun FTP disponible.

Les résultats détaillés de la dernière validation réelle sont dans [tests/validation.md](tests/validation.md). Aucune capture PNG authentique n'a pu être produite dans la session graphique disponible ; aucune image n'est donc intégrée ici.

## Préparation des commits

Les changements ne sont pas commités. Après accord du binôme :

```bash
git add include/net_io.h src/common/net_io.c src/client/chat_client.c src/server/chat_server.c
git commit -m "feat(chat): implement TCP chat client and server"
git add README.md docs tests src/ftp
git commit -m "docs(chat): document architecture protocol and demo"
```

Ne lancer ces commandes qu'après relecture et accord.
