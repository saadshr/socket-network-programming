# Présentation orale en cinq minutes

## Répartition entre deux étudiants

### Étudiant A — 0:00 à 2:30

Présenter l'objectif, l'architecture (`5000` pour le chat, `2121` pour FTP), `ftp_root`, puis expliquer `WSAStartup`, `socket`, `bind`, `listen`, `accept` et `connect`. Montrer `USER`, `PASS` et `PASV`.

### Étudiant B — 2:30 à 5:00

Expliquer les deux connexions FTP. Montrer `LIST`, `STOR`, `RETR`, les réponses `150`/`226`, puis `RETR ../README.md` avec `550`. Terminer avec SHA-256, curl et la régression chat.

## Questions/réponses

**Pourquoi le port 2121 ?** Il évite les privilèges administrateur du port 21 et reste local à la démonstration.

**Pourquoi deux connexions FTP ?** Le contrôle transporte commandes/réponses ; la donnée passive transporte listing ou fichiers.

**Que signifient 227, 150 et 226 ?** `227` annonce le port passif, `150` démarre le transfert, `226` confirme sa fin.

**Quel est le rôle de DNS ?** Aucun nom DNS n'est requis ici : le client utilise `127.0.0.1`. DNS traduit normalement un nom en adresse IP ; il ne remplace pas `connect`.

**Pourquoi `recv` ne correspond-il pas toujours à une commande ?** TCP fragmente ou regroupe les octets ; le projet accumule jusqu'à `\n` ou `\r\n`.

**Pourquoi refuser les reparse points ?** Un junction Windows peut sortir de `ftp_root`; le serveur refuse `FILE_ATTRIBUTE_REPARSE_POINT`.

**Pourquoi une variable d'environnement pour le mot de passe ?** Pour éviter de commiter un secret. Le FTP classique reste non chiffré et est limité à localhost.
