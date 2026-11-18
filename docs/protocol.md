# Protocole du chat TCP

## Cadrage

TCP fournit un flux fiable et ordonné d'octets, mais ne conserve pas les frontières entre les appels à `send`. Un message peut donc être fragmenté dans plusieurs `recv`, ou plusieurs messages peuvent être regroupés dans un seul `recv`.

Le projet utilise un protocole texte simple : **chaque message applicatif est une ligne terminée par `\n`**.

```text
premier message\nsecond message\n/quit\n
```

`recv_line` recherche `\n`, retire ce délimiteur et un éventuel `\r` final, puis conserve les octets restants pour la ligne suivante.

## `line_reader_t` et `recv_line`

`line_reader_t` contient un tableau de 4096 octets et le nombre d'octets utilisés. `recv_line` cherche d'abord une ligne dans ce qui est déjà en mémoire. Si elle est incomplète, la fonction appelle `recv` et ajoute les octets reçus.

Exemples équivalents :

```text
Fragmenté : recv("bonj") puis recv("our\n")
Regroupé  : recv("un\ndeux\n")
```

Dans les deux cas, le programme récupère les lignes applicatives correctes. Les retours sont `1` pour une ligne, `0` pour une déconnexion propre, `-1` pour une erreur Winsock et `-2` pour une ligne trop longue.

## `send_all`

`send_all` boucle jusqu'à l'envoi des `length` octets demandés. Il ne suppose pas qu'un appel à `send` suffit. Une valeur `SOCKET_ERROR` ou `0` avant la fin signale une erreur.

Le client ajoute `\n` avant l'envoi. Le serveur répond par une ligne telle que :

```text
Serveur: reçu -> premier message\n
```

## `/quit` et déconnexions

À la réception exacte de `/quit`, le serveur envoie `Serveur: au revoir !\n`, puis ferme le socket client et le socket d'écoute. Le client affiche la réponse puis ferme sa connexion. `recv == 0` est traité comme une déconnexion propre ; `SOCKET_ERROR` déclenche l'affichage du code de `WSAGetLastError`.

Ce protocole concerne uniquement le chat. Le futur FTP aura ses réponses de contrôle terminées par `\r\n` et une connexion de données séparée en mode passif.
