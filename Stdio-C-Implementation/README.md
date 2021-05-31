# Sulimovici Raoul 331CB
# Sisteme de Operare - Tema 2

-> Pentru structura SO_FILE am folosit un int/HANDLE pentru file_descriptor, curr_pos pentru pozitia actuala in buffer, size pentru cati bytes sunt ocupati din buffer, error pentru semnalarea erorilor si eof pentru semnalarea finalului fisierului.

-> so_fopen va deschide fisierele folosind modul oferit si argumentele functiilor open/CreateFile necesare si va initializa SO_FILE.

-> so_fclose va elibera structura SO_FILE, va face flush la buffer si va inchide file descriptorul.

-> pentru fileno voi returna file descriptorul din structura SO_FILE

-> so_fgetc va verifica erorile, dupa care, daca buffer-ul este gol, il va umple cu date printr-un read din fisier. Dupa va returna primul caracter necitit din acesta.

-> so_fputc va verifica erorile, dupa care, daca nu este loc liber in buffer, il va goli si va scrie datele in fisier. Dupa va salva caracterul dorit in primul loc liber din buffer.

-> so_fread si so_fwrite vor itera de nmemb * size ori si vor incerca sa citeasca/scrie caracterul curent din vector, folosind functiile so_fgetc/so_fputc. Se vor verifica eventualele valori returnate si cazuri de eroare si se va nota operatia facuta in last_operation.

-> so_fflush va face o scriere a intreg bufferului in fisier

-> so_fseek se foloseste de functia lseek pentru a pozitiona file descriptorul in fisier si se vor respecta condiitile mentionate in enunt despre ultima operatie facuta.

-> s_ftell va returna pozitia file descriptorului in fisier.

-> so_feof si so_ferror vor returna 1 daca s-a ajuns la sfarsitul fisierului sau daca au existat erori.

-> so_popen si so_pclose nu au fost implementate.

-> Am implementat tema partial si pe Windows si pe Linux