# Module FTP SocketHub-C

Le module FTP est implémenté en C natif avec WinSock2 :

- `ftp_server.c` écoute sur `127.0.0.1:2121` et limite les fichiers à `ftp_root/` ;
- `ftp_client.c` fournit un menu terminal ;
- `ftp_common.c/.h` regroupent les transferts binaires, le framing de contrôle et le parsing PASV.

Le mot de passe est fourni par `SOCKETHUB_FTP_PASSWORD` au lancement du serveur. Aucun secret n'est codé ni versionné. Voir [../../docs/ftp-guide.md](../../docs/ftp-guide.md) et [../../docs/ftp-protocol.md](../../docs/ftp-protocol.md).
