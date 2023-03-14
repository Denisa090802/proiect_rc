
# Introducere
DevSync este o aplicatie care permite sincronizarea mai multor dispozitive, mentinand permanent 
informatiile actualizate intre acestea. Avand la baza o arhitectura Peer to peer, programul va utiliza o 
multitudine de functii de propagare a informatiei. Pe langa informatii de sistem, nodurile vor stoca 
informatia aferenta unei baze de date distribuite care va contine informatii cu privire la o serie de 
evenimente de pe parcursul unui an(data, locatie etc.), avand gradul de redundanta egal cu numarul de 
noduri active.

#Tehnologii utilizate
Tinand cont de importanta transmiterii integrale a informatiei intre dispozitive, aplicatia va folosi 
protocolul TCP, pentru a ne asigura de facptul ca informatiile se transmit corect. Pentru a descoperi noile 
noduri active care apar in retea, dar si pentru eliminarea nodurile recent dezactivate, se vor folosi thread uri separate cu scopul de a nu afecta viteza/frecventa sincronizarii datelor. Stocarea datelor se va realiza 
cu ajutorul unei baze de date locale, de tip SQLite, aflata in fiecare nod al carui informatii vor fi 
sincronizate.

#Arhitectura aplicatiei
Folosing principiul P2P, fiecare nod din sistem joaca atat rol de client, cat si de server. Fiecare instanta isi 
deschide doua procese: unul prin care asteapta request ul pentru procesare si altul prin care realizeaza 
conexiunea cu celelalte noduri. 
Pentru orice peer, verific daca adresele din subnetul acestuia gazduiesc alte instante ale aceluiasi program.
Un request de citire va returna returnatiile din baza de date locala, in timp de un request de 
inserare/actualizare/stergere va efectua operatia respectiva pe baza de date locala si, ulterior, va fi trimis 
catre celelalte peer-uri, in cazul in care informatiile nu erau deja actualizate.

#Detalii de implementare
In firul de executie corespunzator conectarii, generam toate adresele IP din subnet-ul curent, apoi le 
retinem cu ajutorul unei liste (set) pe cele care ofera un raspuns valid request-ului de verificare. Fiecare
request va avea un timeout de 3 secunde, pentru a nu bloca executia programului, iar in momentul 
conectarii, nodului nou conectat va primi informatii despre toate celelalte cu care este compatibil. Daca se 
descopera ca unul dintre noduri nu mai este activ in retea, se va trimite un request corespunzator celorlalte 
noduri active si se va elimina din ele adresa IP a acestuia. Acest proces este rulat periodic.
In firul de executie responsansabil cu efectuarea cererilor, daca se va primi o solicitare de citire, atunci 
informatiile vor fi cautate si returnate din baza de date locala. In cazul unei solicitari de scriere, se va 
verifica daca operatia a fost efectuata in nodul, iar daca acest lucru nu s-a intamplat, ea se va opera si 
acelasi request va fi trimis catre celelalte noduri active din lista de peer uri. 
In acest fel putem garanta ca orice informatie a fost sincronizata, deoarece actualizarile de informatii aduc
dupa sine o cerere de actualizare in celelalte noduri.


#Concluzii 
Arhitectura creata permite introducerea mai multor peer uri in retea, astfel se suporta un trafic mai mare 
fara a afecta performanta de citire, dar si asigura persistenta datelor chiar si in cazul in care unul dintre 
noduri devine inactive, informatia fiind stocata in continuare in celelalte. 
O imbunatarire ce va creste si functionalitatea proiectului este dezvoltarea unei aplicatii externe (ce va 
juca doar rol de client) care se va putea conecta la oricare dintre peer uri si va putea executa comenzi de 
stergere/actualizare/interogare/inserare pe intregul sistem. Deasemenea se va putea verifica integritatea 
datelor, precum si informatii de sistem despre fiecare nod (active/inactive/spatiu de stocare utilizat). 
Aceasta aplicatie va juca rol de interfata pentru sistemul P2P. Totodata, prin intermediul acesteia putem 
dezvolta un protocol de integrare extern, pentru a oferi functionalitatea stocarii de date pe baza arhitecturii 
peer to peer unor alte sisteme. Putem oferi posibilitatea rularii de queryuri directe pe bazele de date ale 
nodurilor prin aceasta aplicatie, precum si distribuirea acestora pentru a asigura o utilizare uniforma a 
tuturor peer-urilor. Prin acest mijloc, putem mari numarul de clienti simultani si de query-uri executate pe 
sistemul nostru. 

#Bibliografie
https://www.techopedia.com/definition/454/peer-to-peer-architecture-p2p-architecture
https://profs.info.uaic.ro/~computernetworks/cursullaboratorul.php
https://stackoverflow.com/questions/4130147/get-local-ip-address-in-c-linux
https://app.diagrams.net/
https://profs.info.uaic.ro/~computernetworks/files/NetEx/S5/servTcpIt.c
https://profs.info.uaic.ro/~computernetworks/files/NetEx/S5/cliTcpIt.c
