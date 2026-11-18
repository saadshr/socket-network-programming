# FAQ pédagogique

## Qu'est-ce qu'un socket ?
Une interface logicielle permettant à deux processus d'échanger des données sur un réseau.

## Qu'est-ce que TCP ?
Un transport fiable, ordonné et orienté connexion. Il fournit un flux d'octets, pas des messages délimités.

## Que signifie `127.0.0.1` ?
C'est l'adresse de boucle locale : le client et le serveur communiquent sur le même ordinateur.

## Qu'est-ce qu'un port ?
Un identifiant du service réseau d'un processus sur une adresse IP. Le chat utilise `5000`.

## À quoi servent `bind` et `listen` ?
`bind` associe le socket serveur à `127.0.0.1:5000`. `listen` le transforme en socket d'écoute des connexions entrantes.

## À quoi servent `accept` et `connect` ?
`connect` est appelé par le client pour demander une connexion. `accept` est appelé par le serveur et retourne un socket dédié à ce client.

## Pourquoi utiliser `send_all` ?
Parce que `send` peut envoyer moins d'octets que demandé. `send_all` boucle jusqu'à l'envoi complet ou une erreur.

## Pourquoi `recv` ne correspond-il pas à un message ?
TCP est un flux : un message peut être fragmenté ou plusieurs messages regroupés. `recv_line` recherche `\n` et conserve les octets supplémentaires.

## Quel est le rôle de WinSock2 ?
WinSock2 est l'API réseau Windows utilisée ici. Chaque processus appelle `WSAStartup` avant les sockets et `WSACleanup` à la fin.

## Différence entre chat et FTP ?
Le chat actuel échange des lignes sur une seule connexion TCP. FTP utilise un protocole de commandes/réponses, une connexion de contrôle et une connexion de données séparée, notamment en mode passif.

## FTP fonctionne-t-il déjà ?
Non. `USER`, `PASS`, `PASV`, `LIST`, `RETR`, `STOR` et `QUIT` sont des fonctionnalités futures et non développées dans cette version.
