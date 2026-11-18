# Captures d'écran à réaliser manuellement

Ce dossier ne contient volontairement aucune image inventée. Les captures doivent être prises pendant une démonstration réelle.

Captures recommandées :

1. compilation GCC sans erreur ;
2. serveur affichant `127.0.0.1:5000` en écoute ;
3. client affichant au moins deux messages et leurs réponses ;
4. serveur affichant les messages correspondants ;
5. traitement de `/quit` dans les deux terminaux.

Convention proposée : `01-compilation.png`, `02-server-listening.png`, `03-client-chat.png`, `04-server-chat.png`, `05-quit.png`.

Ne créer ces fichiers qu'à partir de sorties réellement observées. Les captures FTP viendront après son implémentation.

## État actuel

La tentative de capture automatisée du 18 novembre 2026 n'a pas trouvé de fenêtres terminal Windows capturables dans la session graphique disponible. Les fichiers `tcp-server.png`, `tcp-client.png` et `tcp-exchange.png` n'ont donc pas été créés. Les journaux et résultats réels sont documentés dans [tests/validation.md](../../tests/validation.md).
