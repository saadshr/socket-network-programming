# Démonstration et présentation orale

## Préparer la démonstration

À la racine du dépôt :

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts\build.ps1
```

### Chat : deux terminaux

Terminal 1 :

```powershell
.\build\chat_server.exe
```

Terminal 2 :

```powershell
.\build\chat_client.exe
```

Entrer `premier message`, `deuxième message`, puis `/quit`. Résultat attendu : deux accusés `Serveur: reçu -> ...`, puis `Serveur: au revoir !`. Le serveur affiche les lignes reçues et se termine après la session.

### FTP : deux terminaux

Terminal 1 :

```powershell
$env:SOCKETHUB_FTP_USER = "student"
$env:SOCKETHUB_FTP_PASSWORD = "secret-local"
.\build\ftp_server.exe
```

Terminal 2 :

```powershell
.\build\ftp_client.exe
```

Entrer les mêmes identifiants, choisir `LIST`, puis `STOR` vers un nouveau nom et `RETR` vers un fichier local. Montrer `227`, `150` et `226`. Pour une vérification externe, dans un troisième terminal pendant que le serveur est lancé :

```powershell
curl.exe --ftp-pasv --user student:secret-local ftp://127.0.0.1:2121/
```

La valeur est un secret de démonstration local uniquement : ne pas la mettre dans Git. Pour illustrer la sandbox, essayer `RETR ../README.md` ou une destination déjà existante : le serveur répond `550`. Quitter avec `QUIT`, code `221`.

## Script oral de cinq minutes

### Étudiant A — 0:00 à 2:30

1. Présenter l'objectif : deux programmes réseau natifs C sous Windows, chat TCP et FTP local.
2. Montrer les ports `5000` et `2121`, puis expliquer `WSAStartup`, `socket`, `bind`, `listen`, `accept` et `connect`.
3. Faire le chat : deux messages, accusés, `/quit`.
4. Introduire FTP : `USER`, `PASS`, `SYST`, `FEAT`, `TYPE I`, `PWD`.

### Étudiant B — 2:30 à 5:00

1. Expliquer que `PASV` crée une deuxième connexion de données.
2. Montrer `LIST`, `STOR`, `RETR` et les codes `227`, `150`, `226`.
3. Montrer un chemin interdit avec `550` et rappeler la protection contre `..`, les chemins absolus et les reparse points.
4. Terminer par la preuve SHA-256, `curl.exe`, les tests automatisés et les limites : localhost, mono-session, FTP non chiffré.

## Questions fréquentes

**Pourquoi `recv` ne correspond-il pas à une commande ?** TCP est un flux ; le code accumule jusqu'à `\n` pour le chat et `\r\n` pour FTP.

**Pourquoi FTP utilise-t-il deux sockets ?** Le contrôle transporte les commandes/réponses, la donnée transporte les fichiers ou le listing.

**Que fait DNS ?** Il traduit un nom comme `serveur.example` en adresse IP. Le projet utilise directement `127.0.0.1`, donc DNS n'intervient pas.

**Pourquoi le mot de passe n'est-il pas dans le code ?** Il est fourni par une variable d'environnement et masqué dans les journaux.

**Pourquoi le port 2121 ?** Il évite les privilèges du port 21 et convient à une démonstration locale.

**Le FTP est-il sécurisé sur Internet ?** Non. FTP classique n'est pas chiffré ; ce projet le limite donc à localhost. FTPS/TLS et le mode actif ne sont pas implémentés.
