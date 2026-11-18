# SocketHub-C

SocketHub-C est un projet universitaire de programmation réseau native en C sous Windows. Il contient maintenant une messagerie TCP et un mini-service FTP pédagogique.

Le chat utilise `127.0.0.1:5000`. Le FTP utilise `127.0.0.1:2121`, une connexion de contrôle et une connexion de données passive. L'UDP et l'application de temps restent hors périmètre.

## Fonctionnalités terminées

- serveur TCP sur `127.0.0.1:5000` ;
- client TCP et échanges de plusieurs messages ;
- commande `/quit` et détection des déconnexions ;
- vérifications WinSock2 et nettoyage avec `closesocket`/`WSACleanup` ;
- envois partiels gérés par `send_all` ;
- messages cadrés par ligne avec `recv_line` ;
- messages TCP fragmentés ou regroupés correctement traités.

## Fonctionnalités FTP terminées

- serveur et client C natifs dans `src/ftp/` ;
- commandes `USER`, `PASS`, `SYST`, `FEAT`, `TYPE I`, `PWD`, `PASV`, `LIST`, `RETR`, `STOR`, `NOOP` et `QUIT` ;
- mode passif avec connexion de contrôle séparée de la connexion de données ;
- transferts texte et binaires avec buffers de 64 KiB et gestion des envois partiels ;
- authentification configurable par variables d'environnement ;
- sandbox limitée à `ftp_root/`, avec refus des chemins absolus, `..` et reparse points ;
- upload temporaire puis validation par déplacement sans écrasement d'un fichier existant ;
- compatibilité vérifiée avec `curl.exe`.

Le serveur accepte un client par exécution et se termine après la session. Il faut le relancer pour une nouvelle démonstration.

## Fonctionnalités FTP hors périmètre actuel

Le FTP n'est pas chiffré : il est volontairement limité à `127.0.0.1`. FTPS/TLS, comptes multiples et mode actif ne sont pas implémentés. Voir [src/ftp/README.md](src/ftp/README.md).

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
gcc -Wall -Wextra -Wpedantic -std=c11 -Isrc/ftp -Iinclude src/common/net_io.c src/ftp/ftp_common.c src/ftp/ftp_server.c -o build/ftp_server.exe -lws2_32
gcc -Wall -Wextra -Wpedantic -std=c11 -Isrc/ftp -Iinclude src/common/net_io.c src/ftp/ftp_common.c src/ftp/ftp_client.c -o build/ftp_client.exe -lws2_32
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

Pour le FTP, dans PowerShell, définir des identifiants de démonstration uniquement dans l'environnement local :

```powershell
$env:SOCKETHUB_FTP_USER = "student"
$env:SOCKETHUB_FTP_PASSWORD = "choisir-un-secret-local"
.\build\ftp_server.exe
```

Dans un autre terminal :

```powershell
.\build\ftp_client.exe
```

Le serveur crée ou utilise [ftp_root/](ftp_root/) comme unique dossier autorisé. Le menu du client guide `LIST`, `STOR` et `RETR`.

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
├── src/ftp/
│   ├── ftp_common.c
│   ├── ftp_common.h
│   ├── ftp_client.c
│   ├── ftp_server.c
│   └── README.md
├── ftp_root/.gitkeep
├── tests/README.md
├── tests/ftp_validation.ps1
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
- [Guide FTP](docs/ftp-guide.md)
- [Protocole FTP](docs/ftp-protocol.md)
- [Démonstration FTP en cinq minutes](docs/ftp-demo.md)
- [Tests FTP](docs/ftp-tests.md)

## Limites connues

- un seul client par exécution du serveur ;
- accusé de réception simple, pas encore de chat multi-client ;
- lignes limitées par un tampon de 4096 octets ;
- aucune authentification ni chiffrement ;
- écoute limitée à `127.0.0.1` ;
- pas de FTPS/TLS, mode actif, comptes multiples ni suppression de fichiers.

Limites FTP : le serveur est mono-session séquentiel, les transferts sont limités au mode passif, le service n'est pas chiffré et le mot de passe doit être fourni dans l'environnement du processus serveur.

Les résultats détaillés de la dernière validation réelle sont dans [tests/validation.md](tests/validation.md). Aucune capture PNG authentique n'a pu être produite dans la session graphique disponible ; aucune image n'est donc intégrée ici.

## Préparation des commits

Les changements ne sont pas commités. Après accord du binôme :

```bash
git add include/net_io.h src/common/net_io.c src/client/chat_client.c src/server/chat_server.c
git commit -m "feat(chat): implement TCP chat client and server"
git add README.md docs tests src/ftp
git commit -m "docs(chat): document architecture protocol and demo"
```

Ne lancer ces commandes qu'après relecture et accord. Les prochains commits FTP devront être séparés du chat si une nouvelle évolution est nécessaire.
