# Validation TCP et FTP du 18 novembre 2026

Cette note conserve les résultats d'une exécution réelle sur Windows avec GCC MSYS2 UCRT64. Les exécutables ont été recompilés avant les scénarios et les processus serveur créés par le harnais ont été arrêtés/nettoyés à la fin de chaque scénario.

## Compilation exécutée

```text
gcc -Wall -Wextra -Wpedantic -std=c11 -Iinclude src/common/net_io.c src/server/chat_server.c -o build/chat_server.exe -lws2_32
gcc -Wall -Wextra -Wpedantic -std=c11 -Iinclude src/common/net_io.c src/client/chat_client.c -o build/chat_client.exe -lws2_32
```

Résultat : compilation réussie, sans erreur GCC. Les exécutables restent dans `build/`, exclu par `.gitignore`.

## Scénarios exécutés

| Scénario | Résultat | Preuve observée |
|---|---|---|
| Connexion TCP, deux messages, accusés et `/quit` | PASS | Réponses `Serveur: reçu -> alpha`, `Serveur: reçu -> beta`, puis `Serveur: au revoir !`. |
| Cadrage fragmenté et regroupé | PASS | Envoi de `frag` puis `mented\nthird\n` ; deux réponses distinctes et correctes. |
| Message vide | PASS | Réponse `Serveur: reçu -> ` reçue. |
| Message long | PASS | 4094 caractères envoyés ; réponse de longueur 4111, conforme au préfixe et au contenu. |
| Déconnexion sans `/quit` | PASS | Le journal serveur contient `Le client s'est déconnecté.`. |
| Serveur indisponible | PASS | Client terminé avec le code 1 et erreur Winsock 10061 (`connect a échoué`). |

Les scénarios utilisent uniquement des messages de test neutres et ne contiennent aucune information personnelle.

## Captures d'écran

Aucune capture PNG n'est suivie dans le dépôt. Les preuves textuelles ci-dessus sont suffisantes pour reproduire et vérifier les scénarios sans inventer d'image.

## Reproduction

## Validation FTP

La même exécution a compilé les quatre programmes avec `-Wall -Wextra -Wpedantic -std=c11 -lws2_32`, puis a lancé les scénarios FTP sous Windows avec `curl.exe 8.22.0`. Les processus et les fichiers temporaires ont été nettoyés.

| Scénario | Résultat | Preuve observée |
|---|---|---|
| Authentification et commandes de contrôle | PASS | Codes `331, 230, 215, 211, 200, 257, 200`. |
| `PASV` + `LIST` | PASS | `150` puis `226`, contenu du sandbox visible. |
| `STOR`/`RETR` texte | PASS | Transfert complet et contenu identique. |
| `STOR`/`RETR` binaire | PASS | Hash SHA-256 téléchargé identique à l'original. |
| Fichier inexistant, chemin absolu, `../` | PASS | Réponses `550`. |
| Reparse point/junction | PASS | Réponse `550`, sortie du sandbox refusée. |
| Écrasement existant | PASS | Réponse `550`. |
| Identifiants invalides | PASS | Codes `331, 530, 221`. |
| Commande inconnue | PASS | Code `502`. |
| Client FTP C | PASS | Menu, upload/download, code de sortie 0. |
| `curl.exe` externe | PASS | Listing en mode passif, code 0. |

Les valeurs d'identifiants sont injectées uniquement dans l'environnement du processus de test. Aucun mot de passe réel n'est enregistré dans Git. La batterie complète se rejoue avec :

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts\test.ps1
```

La démonstration humaine et le script oral sont décrits dans [demo-orale.md](../docs/demo-orale.md). L'explication de l'architecture et du code est dans [architecture.md](../docs/architecture.md) et [guide-binome.md](../docs/guide-binome.md).
