# Tests FTP réels

Validation exécutée le 18 novembre 2026 sous Windows avec GCC MSYS2 UCRT64, `curl.exe 8.22.0` et les exécutables compilés localement. Les processus serveur ont été contrôlés puis arrêtés ; les fichiers de test ont été supprimés après validation.

La batterie est rejouable depuis la racine avec :

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File tests\ftp_validation.ps1
```

## Compilation

Les quatre commandes chat/FTP avec `-Wall -Wextra -Wpedantic -std=c11 -lws2_32` ont réussi sans avertissement bloquant. `ftp_server.exe` et `ftp_client.exe` ont été générés dans `build/`.

## Résultats

| Test | Résultat | Observation réelle |
|---|---:|---|
| Authentification et contrôle | PASS | Codes `331, 230, 215, 211, 200, 257, 200`. |
| `PASV` + `LIST` | PASS | `150` puis `226`, fichier de seed visible. |
| Upload texte `STOR` | PASS | `150/226`. |
| Download texte `RETR` | PASS | `150/226`, 39 octets identiques. |
| Upload binaire | PASS | `150/226`, données reçues dans le sandbox. |
| Download binaire + SHA-256 | PASS | `150/226`, hash téléchargé égal au fichier serveur. |
| Fichier inexistant | PASS | `550`. |
| Traversée `../` | PASS | `550`. |
| Chemin absolu | PASS | `550`. |
| Reparse point/junction | PASS | `550`, le junction vers `build` a été refusé par le serveur. |
| Écrasement existant | PASS | `550`. |
| Identifiants invalides | PASS | `331, 530, 221`. |
| Commande inconnue | PASS | `502`. |
| `QUIT` | PASS | `221`. |
| Client FTP C | PASS | Menu exécuté ; upload/download et SHA-256 local réussis, code 0. |
| `curl.exe` externe | PASS | Listing FTP réussi, code 0, mode passif utilisé. |
| Régression chat TCP | PASS | Deux messages et `/quit`, client code 0, serveur terminé proprement. |

Les secrets utilisés étaient des valeurs de démonstration injectées uniquement dans l'environnement du processus (`student` / valeur locale de test). Aucun mot de passe n'est enregistré dans le dépôt.
