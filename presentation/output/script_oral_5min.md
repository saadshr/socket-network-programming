# Script oral — SocketHub-C (5 minutes maximum)

## Étudiant 1 — diapositives 1 à 5 (environ 2 min 30)

### Diapositive 1 — 20 secondes

« Bonjour. Nous présentons SocketHub-C, un projet de programmation réseau native en C sous Windows avec WinSock2. Le projet comporte deux applications : un chat TCP et un mini-service FTP pédagogique. »

### Diapositive 2 — 30 secondes

« L’objectif est de mettre en pratique le modèle client/serveur. Le chat écoute sur 127.0.0.1, port 5000. Le FTP écoute sur le port 2121. Dans les deux cas, le client initie la communication et le serveur attend, accepte puis traite les requêtes. Le FTP ajoute une connexion de données séparée en mode passif. »

### Diapositive 3 — 35 secondes

« Côté serveur, le programme initialise WinSock2 avec WSAStartup, crée le socket, l’associe avec bind, écoute avec listen, puis accepte la connexion avec accept. Les échanges utilisent recv et send. Notre utilitaire recv_line reconstitue les lignes TCP et send_all gère les envois partiels. Le client suit un cycle plus court avec socket, connect, envoi et réception. Enfin, les deux côtés ferment avec closesocket puis WSACleanup. »

### Diapositive 4 — 35 secondes

« Cette séquence montre le chat sur 127.0.0.1:5000. Le client se connecte, envoie “Bonjour SocketHub-C”, puis “Test de communication TCP”. Le serveur répond à chaque message avec un accusé. La commande /quit déclenche le message de fermeture avant le nettoyage des sockets. »

### Diapositive 5 — 30 secondes

« À gauche, la compilation GCC des quatre exécutables réussit. À droite, la capture réelle montre les deux terminaux, la connexion, les deux échanges et /quit. Cette preuve termine la partie Chat TCP. »

### Transition — 10 secondes

« Je passe maintenant la parole à mon binôme pour la partie FTP et les transferts de fichiers. »

## Étudiant 2 — diapositives 6 à 8 (environ 2 min 30)

### Diapositive 6 — 50 secondes

« Le FTP utilise une connexion de contrôle sur le port 2121. Le client envoie USER puis PASS. Le serveur répond 331 lorsqu’il demande le mot de passe, puis 230 après authentification. La commande PASV ouvre ensuite un port de données distinct. Le contrôle transporte les commandes et les codes FTP, tandis que la connexion de données transporte le listing ou les octets du fichier. Le projet reste limité à localhost, car FTP classique n’est pas chiffré. »

### Diapositive 7 — 50 secondes

« STOR envoie un fichier local vers le serveur. La capture montre l’envoi de demo_cc.txt, avec les réponses 227, 150 puis 226. LIST demande ensuite le contenu du répertoire distant. La capture affiche .gitkeep et demo_cc.txt, avec la fin du transfert confirmée par 226. Les fichiers restent dans la sandbox ftp_root. »

### Diapositive 8 — 40 secondes

« RETR télécharge demo_cc.txt vers un fichier local. Enfin, PowerShell calcule les empreintes SHA-256 du fichier original et de la copie téléchargée. La comparaison renvoie True, ce qui confirme que le contenu reçu est identique. »

### Conclusion — 10 secondes

« SocketHub-C réunit donc un chat TCP fonctionnel, l’authentification FTP, le listing, l’upload, le download et une vérification d’intégrité. La mise en œuvre reste volontairement locale, mono-session et non chiffrée. »

## Repères de temps

| Partie | Temps cible |
|---|---:|
| Étudiant 1, diapositives 1 à 5 | 2 min 30 |
| Transition | 10 s |
| Étudiant 2, diapositives 6 à 8 | 2 min 20 |
| Marge | 0 à 10 s |
| Total | 5 min maximum |

