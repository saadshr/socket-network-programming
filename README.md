# Socket Network Programming

Projet d’application client–serveur en C avec des sockets TCP/UDP. Le thème de l’application reste à définir.

## Structure du dépôt

```text
socket-network-programming/
├── src/
│   ├── client/    # Connexion au serveur et interactions côté client
│   ├── server/    # Réception des requêtes et traitement côté serveur
│   └── common/    # Code réseau et format des messages partagés
├── include/       # En-têtes C partagés
├── tests/         # Tests des messages et des échanges client–serveur
└── docs/          # Architecture et futures décisions de conception
```

La [note d’architecture](docs/architecture.md) décrit le rôle de chaque partie et les choix qui restent ouverts.

## État du projet

Le dépôt contient la structure de départ et la documentation. Il n’y a pas encore de programme à compiler ni de dépendances à installer.

La prochaine étape est de choisir le thème et de décrire un premier échange concret : ce que le client envoie, ce que le serveur fait et la réponse attendue. Cet échange guidera le choix du transport et le premier test de bout en bout.
