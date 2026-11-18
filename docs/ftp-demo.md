# Démonstration FTP en cinq minutes

## 0:00–1:00 — compiler

Dans MSYS2 UCRT64 à la racine :

```bash
mkdir -p build
gcc -Wall -Wextra -Wpedantic -std=c11 -Isrc/ftp -Iinclude src/common/net_io.c src/ftp/ftp_common.c src/ftp/ftp_server.c -o build/ftp_server.exe -lws2_32
gcc -Wall -Wextra -Wpedantic -std=c11 -Isrc/ftp -Iinclude src/common/net_io.c src/ftp/ftp_common.c src/ftp/ftp_client.c -o build/ftp_client.exe -lws2_32
```

## 1:00–2:00 — lancer le serveur

PowerShell 1 :

```powershell
$env:SOCKETHUB_FTP_USER = "student"
$env:SOCKETHUB_FTP_PASSWORD = "secret-local"
.\build\ftp_server.exe
```

Expliquer `127.0.0.1:2121` et le rôle de [ftp_root/](../ftp_root/).

## 2:00–4:00 — montrer le client C

PowerShell 2 :

```powershell
.\build\ftp_client.exe
```

Entrer `student`, `secret-local`, puis montrer `SYST`, `FEAT`, `PWD`, `LIST`, `STOR` d'un fichier texte et `RETR` de ce même fichier. Insister sur les réponses `227`, `150` et `226` : elles prouvent les deux connexions TCP.

## 4:00–5:00 — sécurité et preuve

Montrer que `RETR ../README.md` répond `550`, qu'un `STOR` vers un fichier existant répond `550`, puis quitter avec `QUIT` et le code `221`. Pour une preuve externe rapide :

```powershell
curl.exe --ftp-pasv --user student:secret-local ftp://127.0.0.1:2121/
```

Les résultats réellement exécutés sont détaillés dans [ftp-tests.md](ftp-tests.md). Le service FTP reste local et non chiffré ; le mot de passe de démonstration ne doit pas être commité.
