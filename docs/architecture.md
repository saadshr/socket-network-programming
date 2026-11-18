# Architecture initiale

Le projet est organisé autour de deux futurs exécutables C, un client et un serveur. Ils utiliseront le même code pour encoder et décoder les messages.

```text
Client C  <────── échanges sur socket ──────>  Serveur C
   │                                              │
   └──────────── code C partagé ─────────────────┘
```

- **Client (`src/client/`)** : constitue les requêtes, les envoie au serveur et interprète les réponses.
- **Serveur (`src/server/`)** : reçoit les messages, les valide et appelle le traitement correspondant au cas d’usage avant de répondre.
- **Code commun (`src/common/` et `include/`)** : regroupe le format des messages et les fonctions réseau dont les deux exécutables ont besoin. Les détails propres au système pourront rester dans le code réseau partagé si plusieurs plateformes sont visées.
- **Tests (`tests/`)** : vérifieront d’abord l’encodage et la lecture des messages, puis un échange réel en local.

## Décisions à prendre après le choix du thème

1. Définir les actions de l’utilisateur et les messages nécessaires au premier échange.
2. Choisir le rôle de chaque transport. TCP fournit un flux fiable et ordonné : il faudra définir comment délimiter les messages. UDP conserve les limites des datagrammes, mais ne garantit ni leur livraison ni leur ordre : il faudra décider comment traiter les pertes si elles comptent pour l’application.
3. Fixer la plateforme cible et l’outil de compilation avant d’ajouter du code réseau spécifique à Windows ou à un système POSIX.
