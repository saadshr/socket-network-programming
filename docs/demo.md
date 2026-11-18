# Démonstration du chat TCP

## Compilation

Dans MSYS2 UCRT64, à la racine du dépôt :

```bash
mkdir -p build
gcc -Wall -Wextra -Wpedantic -std=c11 -Iinclude src/common/net_io.c src/server/chat_server.c -o build/chat_server.exe -lws2_32
gcc -Wall -Wextra -Wpedantic -std=c11 -Iinclude src/common/net_io.c src/client/chat_client.c -o build/chat_client.exe -lws2_32
```

## Deux terminaux PowerShell

Terminal 1 :

```powershell
Set-Location C:\chemin\vers\socket-network-programming
.\build\chat_server.exe
```

Sortie attendue :

```text
Serveur de chat en écoute sur 127.0.0.1:5000
Un client peut se connecter. Ctrl+C arrête le serveur.
```

Terminal 2 :

```powershell
Set-Location C:\chemin\vers\socket-network-programming
.\build\chat_client.exe
```

Saisir `premier message`, `deuxième message`, puis `/quit`. Le client doit afficher les deux accusés et `Serveur: au revoir !`; le serveur doit afficher les trois lignes reçues, dont `Client: /quit`.

## Vérifications

- adresse et port affichés correctement ;
- au moins deux messages reçus dans le bon ordre ;
- réponse correspondant à chaque message ;
- `/quit` termine sans erreur ;
- après compilation, les deux `.exe` existent dans `build/`.

## Dépannage

| Symptôme | Solution |
|---|---|
| `connect` échoue | Lancer le serveur d'abord et conserver `127.0.0.1:5000`. |
| `bind` échoue | Fermer une ancienne instance ou libérer le port 5000. |
| `gcc` introuvable | Utiliser MSYS2 UCRT64 et vérifier `gcc --version`. |
| `ws2_32` introuvable | Utiliser MinGW/UCRT64 et conserver `-lws2_32` en fin de commande. |
| Pas de réponse | Appuyer sur Entrée : le protocole attend `\n`. |
| Serveur terminé après `/quit` | Comportement prévu ; le relancer pour une autre session. |

Cette démonstration ne teste pas FTP, le mode passif, `LIST`, `RETR` ou `STOR`, qui ne sont pas encore implémentés.

Les résultats de validation exécutés le 18 novembre 2026 sont consignés dans [tests/validation.md](../tests/validation.md). Aucune capture PNG n'a été inventée : la session graphique ne rendait pas les fenêtres terminal capturables.
