# Validation TCP du 18 novembre 2026

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

Aucune capture PNG n'a été créée. Les fenêtres terminal lancées pour la tentative graphique n'ont pas été exposées dans la session Windows contrôlable ; aucune capture authentique n'était donc disponible. Voir [docs/screenshots/README.md](../docs/screenshots/README.md) pour la liste des captures à prendre manuellement.

## Reproduction

La démonstration interactive est décrite dans [docs/demo.md](../docs/demo.md). Le scénario de cadrage nécessite un client TCP de test capable d'envoyer des fragments ; il ne constitue pas une nouvelle fonctionnalité du programme de chat.
