# Guide FTP pour débutant

## Préparer le serveur

Le serveur FTP utilise uniquement `ftp_root/` comme espace de fichiers. Il ne faut pas y placer de données sensibles. Le mot de passe n'est pas dans le code : il est fourni au processus par PowerShell.

```powershell
$env:SOCKETHUB_FTP_USER = "student"
$env:SOCKETHUB_FTP_PASSWORD = "secret-local"
.\build\ftp_server.exe
```

Le serveur affiche `127.0.0.1:2121`. Le port 2121 évite les privilèges administrateur nécessaires au port FTP classique 21.

## Utiliser le client C

Dans un second terminal :

```powershell
.\build\ftp_client.exe
```

Le client demande les mêmes identifiants puis propose :

- `SYST` : type de système annoncé ;
- `FEAT` : extensions disponibles ;
- `TYPE I` : mode binaire ;
- `PWD` : répertoire virtuel courant, toujours `/` dans cette version ;
- `LIST` : liste du contenu de `ftp_root` ;
- `RETR` : téléchargement vers un chemin local choisi ;
- `STOR` : envoi d'un fichier local vers un nouveau nom distant ;
- `NOOP` : test de la connexion ;
- `QUIT` : fermeture propre.

## Ce qui se passe pour un fichier

Pour `LIST`, `RETR` ou `STOR`, le client envoie `PASV`. Le serveur répond avec un port temporaire, puis le client ouvre une seconde socket TCP. La commande de transfert est envoyée sur la connexion de contrôle. Les octets circulent sur la connexion de données et celle-ci est fermée avant la réponse finale `226`.

Les fichiers sont transférés en mode binaire par blocs de 64 KiB. Les appels `send` peuvent être partiels ; `send_all` boucle jusqu'à l'envoi complet. Un upload arrive dans un fichier temporaire et n'est déplacé vers sa destination qu'après réception complète.

## Sécurité et limites

Le service est limité à `127.0.0.1`, car FTP classique transmet les identifiants sans chiffrement. Les chemins absolus, `..`, reparse points et destinations déjà existantes sont refusés. FTPS/TLS, comptes multiples, suppression et mode actif ne sont pas proposés.
