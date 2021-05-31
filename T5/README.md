# Sulimovici Raoul-Renatto 331CB

Pentru implementare am folosit codul din echo_server, la care am adaugat functionalitati din http_reply_once si alte functii noi.
Se vor crea conexiuni si salva in structura connection.
Se vor primi request-uri pe care le manage-uim cu epoll si iocp.
Initializez structurile din http_parser si primesc path-ul dorit, dupa care trimit fisierul cu sendfile, respectiv TransmitFile.

Implementarea este minimala si contine 90% codul din sample, asa ca nu pot spune foarte mult despre cod.