# Protocole FTP implémenté

## Connexion de contrôle

Le serveur écoute sur `127.0.0.1:2121`. Toutes les commandes et réponses sont des lignes terminées par `\r\n`. Les réponses commencent par un code à trois chiffres.

| Commande | Réponse attendue | Effet |
|---|---|---|
| `USER student` | `331` | Demande le mot de passe. |
| `PASS ...` | `230` ou `530` | Authentifie ou refuse. |
| `SYST` | `215` | Annonce `UNIX Type: L8`. |
| `FEAT` | `211` multi-ligne | Annonce `PASV`, `EPSV` et `UTF8`. |
| `TYPE I` | `200` | Sélectionne le mode binaire. |
| `PWD` | `257` | Retourne `/`. |
| `PASV` | `227` | Retourne `h1,h2,h3,h4,p1,p2`. |
| `LIST` | `150`, puis `226` | Envoie une liste sur la connexion data. |
| `RETR nom` | `150`, puis `226` | Télécharge un fichier. |
| `STOR nom` | `150`, puis `226` | Upload vers un nouveau fichier. |
| `NOOP` | `200` | Vérifie que le contrôle répond. |
| `QUIT` | `221` | Ferme le contrôle, même avant authentification. |
| commande inconnue | `502` | Signale une commande non implémentée. |

Erreurs de transfert : `425` si aucune connexion passive ne peut être ouverte, `426` si le flux data est interrompu et `550` pour un chemin ou fichier refusé.

## Mode passif

Après `PASV`, le serveur crée un socket d'écoute temporaire sur `127.0.0.1` et un port éphémère. Pour `p1` et `p2`, le port est `p1 * 256 + p2`. Le client se connecte à ce port, puis envoie `LIST`, `RETR` ou `STOR` sur le contrôle.

`EPSV` est aussi accepté pour permettre à `curl.exe` de choisir le mode étendu. Les transferts restent limités à l'adresse locale.

## Chemins virtuels

Le client envoie des noms relatifs à `ftp_root`, par exemple `notes.txt`. Le serveur refuse `/etc/passwd`, `C:\Windows\win.ini`, `../README.md`, les composants `..` et les fichiers marqués `FILE_ATTRIBUTE_REPARSE_POINT`.

`STOR` ne remplace jamais une destination existante. Il crée un nom temporaire dans le parent validé, reçoit les octets, ferme le fichier, puis utilise `MoveFileExW` sans remplacement.
