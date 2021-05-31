# Sulimovici Raoul-Renatto 331CB
# Tema 3 Sisteme de Operare

## Implementare
Am creat:
    -> o structura scheduler, care va retine numarul de operatii io si cuanta de timp din init
    -> o structura thread cu un thread_id, o prioritate primita in fork, o cuanta de timp primita de la scheduler si un stadiu
    -> o structura params, care va trimite parametrii catre thread, cu un handler primit din fork, o prioritate, aceeasi cu a thread-ului si un thread

Am initializat scheduler-ul, o variabila care verifica daca acesta este deja initializat, un contor pentru thread-uri si index-ul thread-ului curent.

Init va verifica daca numarul de operatii io sunt mai multe decat numarul maxim permis sau daca cuanta de timp este 0 si, in acest caz, va returna -1 (eroare).
De asemenea, va returna eroare si daca scheduler-ul este deja initializat.
Setez cuanta si numarul de io.

Fork va crea un nou thread si il va adauga in vectorul de thread-uri, actualizandu-i starea la NEW, prioritatea si cuanta.
Creez parametrii, ii folosesc la functia start_thread si fac join. Start_thread va apela handler-ul cu prioritatea data.

Wait si signal vor verifica variabila io, iar end va elibera scheduler.